import * as fs from 'fs'
import * as path from 'path'
import sharp from 'sharp'

// --- Configuration ---
// TO BE PROVIDED BY THE USER:
const SVG_INPUT_DIR = './assets/icons/' // Placeholder: Please update with the correct path to your SVG assets.
// It was found that 'heizbox/apps/esp32/include' is the likely include directory.
const C_OUTPUT_DIR = './apps/esp32/include/icons/'
const DEFAULT_ICON_SIZE = 48 // Default size if not specified in SVG filename

// --- Helper Functions ---

/**
 * Converts a raw 1-bit monochrome buffer to a C-style byte array string.
 * Each byte in the output represents 8 pixels.
 *
 * @param buffer The 1-bit raw bitmap buffer from sharp.
 * @param width The width of the original image in pixels.
 * @param height The height of the original image in pixels.
 * @returns A string representing the C byte array.
 */
function convertBitmapToCByteArray(buffer: Buffer, width: number, height: number): string {
	const bytes: string[] = []
	let byte = 0
	let bitCount = 0

	for (let i = 0; i < buffer.length; i++) {
		const pixelValue = buffer[i] // Get the 8-bit grayscale value

		// Apply threshold: if pixel is dark (e.g., < 128), set bit to 1 (black), else 0 (white)
		if (pixelValue >= 128) {
			// invertierte Logik
			byte |= 1 << (7 - bitCount)
		}

		bitCount++
		if (bitCount === 8) {
			bytes.push(`0x${byte.toString(16).padStart(2, '0')}`)
			byte = 0
			bitCount = 0
		}
	}

	// Add any remaining byte if the total number of bits is not a multiple of 8
	if (bitCount > 0) {
		bytes.push(`0x${byte.toString(16).padStart(2, '0')}`)
	}

	// Format for C array: 16 bytes per line
	let cArrayString = ''
	for (let i = 0; i < bytes.length; i++) {
		cArrayString += bytes[i]
		if (i < bytes.length - 1) {
			cArrayString += ', '
		}
		if ((i + 1) % 16 === 0 && i < bytes.length - 1) {
			cArrayString += '\n    '
		}
	}
	return cArrayString
}

/**
 * Extracts icon name and size from a filename.
 * E.g., "session-24.svg" -> { name: "session", size: 24 }
 *       "fire.svg"       -> { name: "fire", size: 48 } (using default)
 */
function parseFilename(filename: string): { name: string; size: number } {
	const baseName = path.basename(filename, '.svg')
	const parts = baseName.split('-')
	let size = DEFAULT_ICON_SIZE
	let name = baseName

	const lastPart = parts[parts.length - 1]
	if (lastPart && /^\d+$/.test(lastPart)) {
		size = parseInt(lastPart, 10)
		name = parts.slice(0, parts.length - 1).join('-')
	}

	return { name, size }
}

async function generateIcons() {
	console.log(`Starting icon generation from ${SVG_INPUT_DIR} to ${C_OUTPUT_DIR}`)

	// Ensure output directory exists
	fs.mkdirSync(C_OUTPUT_DIR, { recursive: true })

	const svgFiles = fs.readdirSync(SVG_INPUT_DIR).filter((file) => file.endsWith('.svg'))

	if (svgFiles.length === 0) {
		console.warn(`No SVG files found in ${SVG_INPUT_DIR}.`)
		return
	}

	for (const svgFile of svgFiles) {
		const svgFilePath = path.join(SVG_INPUT_DIR, svgFile)
		const { name: iconName, size: iconSize } = parseFilename(svgFile)

		console.log(`Processing ${svgFile} (Icon: ${iconName}, Size: ${iconSize}x${iconSize})`)

		try {
			// Read SVG content
			const svgContent = fs.readFileSync(svgFilePath, 'utf-8')

			// Convert SVG to raw 1-bit monochrome bitmap using sharp
			const { data, info } = await sharp(Buffer.from(svgContent))
				.resize(iconSize, iconSize)
				.flatten({ background: { r: 0, g: 0, b: 0 } }) // Ensure black background for transparent SVGs
				.threshold(128) // Convert to 1-bit monochrome. Pixels >= 128 become white (1), < 128 become black (0).
				.toColourspace('b-w') // Ensure it's monochrome
				.raw({ depth: 'uchar' } as any) // Output raw 8-bit data
				.toBuffer({ resolveWithObject: true })

			console.log(`Sharp output info for ${svgFile}:`, info)
			console.log(`Sharp output data for ${svgFile} (first 100 bytes):`, data.slice(0, 100))

			if (info.width !== iconSize || info.height !== iconSize || info.channels !== 1) {
				console.warn(
					`Sharp output for ${svgFile} has unexpected dimensions or channels: ${info.width}x${info.height}, channels: ${info.channels}. Expected ${iconSize}x${iconSize}, 1 channel.`,
				)
			}

			// Convert the 1-bit buffer to a C-style byte array string
			const cByteArray = convertBitmapToCByteArray(data, info.width, info.height)

			// Generate the C header file content
			const headerContent = `
#ifndef IMAGE_${iconName.toUpperCase()}_${iconSize}_H
#define IMAGE_${iconName.toUpperCase()}_${iconSize}_H

#include <pgmspace.h> // For PROGMEM

// Icon: ${iconName}, Size: ${iconSize}x${iconSize} pixels
// Raw 1-bit monochrome data (packed, 1 byte = 8 pixels)
// Black pixels are represented as 1, white as 0.
static const unsigned char PROGMEM image_${iconName}[] = {
    ${cByteArray}
};

#endif // IMAGE_${iconName.toUpperCase()}_${iconSize}_H
`

			const outputFileName = `${iconName}-${iconSize}.h`
			const outputFilePath = path.join(C_OUTPUT_DIR, outputFileName)

			fs.writeFileSync(outputFilePath, headerContent.trim())
			console.log(`Generated ${outputFilePath}`)
		} catch (error) {
			console.error(`Error processing ${svgFile}:`, error)
		}
	}

	console.log('Icon generation complete.')
}

generateIcons().catch(console.error)

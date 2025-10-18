const { createGlobPatternsForDependencies } = require('@nx/react/tailwind')
const { join } = require('path')

/** @type {import('tailwindcss').Config} */
module.exports = {
	content: [
		join(__dirname, '{src,pages,components,app}/**/*!(*.stories|*.spec).{ts,tsx,html}'),
		...createGlobPatternsForDependencies(__dirname),
	],
	darkMode: 'class',
	theme: {
		extend: {
			colors: {},
			fontFamily: {
				josefin: ['"Josefin Sans"', 'sans-serif'],
			},
		},
	},
	plugins: [],
}

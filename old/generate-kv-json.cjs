const fs = require('fs');
const path = require('path');

const distPath = path.join(__dirname, 'dist');
const outputPath = path.join(__dirname, 'kv-assets.json');

const kvData = [];
const manifest = {};

function readDirRecursive(dir) {
  const files = fs.readdirSync(dir);
  files.forEach(file => {
    const filePath = path.join(dir, file);
    const stat = fs.statSync(filePath);
    if (stat.isDirectory()) {
      readDirRecursive(filePath);
    } else {
      const relativePath = path.relative(distPath, filePath).replace(/\\/g, '/');
      const content = fs.readFileSync(filePath, 'utf8');
      kvData.push({ key: relativePath, value: content });

      // For manifest, we assume the original path is the key and the hashed path is the value
      // This might need adjustment based on how Vite generates hashed filenames
      const originalPath = relativePath.replace(/-\w{8}\./, '.'); // Remove hash from filename
      manifest[originalPath] = relativePath;
    }
  });
}

readDirRecursive(distPath);

// Add the manifest to KV data
kvData.push({ key: '__STATIC_CONTENT_MANIFEST', value: JSON.stringify(manifest) });

fs.writeFileSync(outputPath, JSON.stringify(kvData, null, 2));

console.log(`Generated ${outputPath} with ${kvData.length} assets.`);
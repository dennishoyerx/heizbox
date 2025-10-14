import { slate, slateDark } from "@radix-ui/colors"
const { createGlobPatternsForDependencies } = require('@nx/react/tailwind');
const { join } = require('path');


/** @type {import('tailwindcss').Config} */
module.exports = {
  content: [
    join(
      __dirname,
      '{src,pages,components,app}/**/*!(*.stories|*.spec).{ts,tsx,html}'
    ),
    ...createGlobPatternsForDependencies(__dirname),
  ],
  darkMode: 'class',
  theme: {
      extend: {
          colors: {
              ...slate,
              ...slateDark,
              gray: slate,
              grayDark: slateDark,
              background: colors.slate[900],
          },
      },
  },
  plugins: [],
};
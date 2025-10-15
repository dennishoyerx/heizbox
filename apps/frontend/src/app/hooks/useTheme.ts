import { useState, useEffect, useCallback } from 'react';

export type Theme = 'light' | 'dark';

export const useTheme = () => {
  const [theme, setTheme] = useState<Theme>(() => {
    // Lese das Theme aus dem localStorage oder setze den Standardwert
    return (localStorage.getItem('theme') as Theme) || 'dark';
  });

  useEffect(() => {
    const root = window.document.documentElement;
    // Entferne die alte Klasse und füge die neue hinzu
    root.classList.remove('light', 'dark');
    root.classList.add(theme);
    // Speichere die Auswahl
    localStorage.setItem('theme', theme);
  }, [theme]);

  const toggleTheme = useCallback(() => {
    setTheme((prevTheme) => (prevTheme === 'light' ? 'dark' : 'light'));
  }, []);

  return { theme, toggleTheme };
};

import React, { useState } from 'react';
import { Link } from 'react-router-dom';
import { List, X } from '@phosphor-icons/react'; // Burger menu icons

interface HeaderProps {
  deviceName: string;
  deviceStatus: string;
  heatingStatus: string;
}

const Header: React.FC<HeaderProps> = ({ deviceName, deviceStatus, heatingStatus }) => {
  const [isMenuOpen, setIsMenuOpen] = useState(false);

  return (
    <header className="bg-white shadow-sm p-4">
      <div className="max-w-2xl mx-auto flex justify-between items-center">
        <h1 className="text-2xl font-bold text-slate-900">{deviceName}</h1>

        {/* Burger menu button for small screens */}
        <button className="md:hidden text-slate-600" onClick={() => setIsMenuOpen(!isMenuOpen)}>
          {isMenuOpen ? <X size={24} /> : <List size={24} />}
        </button>

        {/* Desktop navigation */}
        <nav className="hidden md:block">
          <Link to="/" className="mr-4 text-blue-600 hover:underline">Sessions</Link>
          <Link to="/usage" className="text-blue-600 hover:underline">Usage Statistics</Link>
        </nav>
      </div>

      {/* Mobile navigation */}
      {isMenuOpen && (
        <nav className="md:hidden bg-white py-2 px-4 border-t border-slate-200 mt-2">
          <Link to="/" className="block py-2 text-blue-600 hover:underline" onClick={() => setIsMenuOpen(false)}>Sessions</Link>
          <Link to="/usage" className="block py-2 text-blue-600 hover:underline" onClick={() => setIsMenuOpen(false)}>Usage Statistics</Link>
        </nav>
      )}

      {/* Device Status (always visible) */}
      <div className="max-w-2xl mx-auto mt-2 text-sm text-slate-500">
        Gerät Status: <span className="font-semibold ml-2">{deviceStatus}</span>
      </div>
      <div className="max-w-2xl mx-auto mt-1 text-sm text-slate-500">
        Heizstatus: <span className="font-semibold ml-2">{heatingStatus}</span>
      </div>
    </header>
  );
};

export default Header;

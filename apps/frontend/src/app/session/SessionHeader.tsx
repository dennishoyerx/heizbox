import React from 'react';

const SessionHeader = () => {
  return (
    <div className="mt-8 flex justify-center">
      <div id="heat-circle" className="relative flex h-48 w-48 items-center justify-center rounded-full bg-gradient-to-br from-orange-500 via-amber-600 to-red-700 shadow-[0_0_40px_rgba(255,100,0,0.5)]">
        <div id="heat-glow" className="absolute inset-0 rounded-full bg-[radial-gradient(circle_at_center,rgba(255,200,100,0.6),transparent)] blur-xl"></div>
        <svg xmlns="http://www.w3.org/2000/svg" className="h-12 w-12 text-white" fill="none" viewBox="0 0 24 24" stroke="currentColor">
          <path strokeLinecap="round" strokeLinejoin="round" strokeWidth="2" d="M14.85 14.85A3.5 3.5 0 0112 17.5a3.5 3.5 0 01-2.85-2.65M12 3v4m0 4v1m6.364 6.364A9 9 0 1112 3a9 9 0 016.364 15.364z" />
        </svg>
      </div>
    </div>
  );
};

export default SessionHeader;

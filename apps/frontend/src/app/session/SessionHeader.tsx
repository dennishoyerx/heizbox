import React, { useState, useEffect } from 'react';
import { fetchSession } from '../../api';
import type { SessionApiResponse } from '@heizbox/types';
import { Text, Flex } from '@radix-ui/themes';

interface SessionHeaderProps {
  isHeating: boolean;
}

const SessionHeader: React.FC<SessionHeaderProps> = ({ isHeating }) => {
  const [session, setSession] = useState<SessionApiResponse | null>(null);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    const loadSession = async () => {
      try {
        const data = await fetchSession();
        setSession(data);
      } catch (e) {
        setError(e instanceof Error ? e.message : 'An unknown error occurred');
      }
    };

    loadSession();
    const interval = setInterval(loadSession, 5000); // every 5 seconds
    return () => clearInterval(interval);
  }, []);

  const heatCircleClass = isHeating
    ? "relative flex h-48 w-48 items-center justify-center rounded-full bg-gradient-to-br from-red-600 via-orange-500 to-amber-400 shadow-[0_0_60px_10px_rgba(255,100,0,0.7)] animate-pulse"
    : "relative flex h-48 w-48 items-center justify-center rounded-full bg-gradient-to-br from-slate-700 via-slate-800 to-slate-900 shadow-[0_0_40px_rgba(0,0,0,0.5)]";

  const heatGlowClass = isHeating
    ? "absolute inset-0 rounded-full bg-[radial-gradient(circle_at_center,rgba(255,200,100,0.8),transparent)] blur-2xl animate-ping"
    : "absolute inset-0 rounded-full bg-[radial-gradient(circle_at_center,rgba(100,100,100,0.2),transparent)] blur-xl";
    console.log('Rendering SessionHeader with session:', session, 'isHeating:', isHeating);
  return (
    <Flex direction="column" align="center" gap="4" className="mt-8">
      <div id="heat-circle" className={heatCircleClass}>
        {session && isHeating && (
        <><div id="heat-glow" className={heatGlowClass}></div>
        <svg xmlns="http://www.w3.org/2000/svg" className="h-12 w-12 text-white" fill="none" viewBox="0 0 24 24" stroke="currentColor">
          <path strokeLinecap="round" strokeLinejoin="round" strokeWidth="2" d="M14.85 14.85A3.5 3.5 0 0112 17.5a3.5 3.5 0 01-2.85-2.65M12 3v4m0 4v1m6.364 6.364A9 9 0 1112 3a9 9 0 016.364 15.364z" />
                      </svg></>
        )}
        {session && !isHeating && (
          <Flex direction="column" align="center" className="absolute text-white">
            <Text size="8" weight="bold" color="white">{session.clicks}</Text>
          </Flex>
        )}
      </div>
      {error && <Text color="red">Error loading session: {error}</Text>}
    </Flex>
  );
};

export default SessionHeader;

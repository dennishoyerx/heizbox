import { useState, useEffect } from 'react';
import type { ApiResponse } from '../types';
import { formatDateForDisplay, formatTimeForDisplay, calculateConsumption } from './utils';
import { fetchSessions } from './api';

// --- REDESIGNED COMPONENTS ---

const ChevronIcon = ({ isOpen }: { isOpen: boolean }) => (
  <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={2} stroke="currentColor"
    className={`w-5 h-5 text-slate-500 transition-transform duration-300 ${isOpen ? 'rotate-180' : ''}`}>
    <path strokeLinecap="round" strokeLinejoin="round" d="m19.5 8.25-7.5 7.5-7.5-7.5" />
  </svg>
);

const SessionCard = ({ session, index, totalSessions }: { session: SessionGroup, index: number, totalSessions: number }) => {
  const [isOpen, setIsOpen] = useState(false);
  const count = session.length;

  const sessionConsumption = calculateConsumption(count);
  const consumptionValue = parseFloat(sessionConsumption);

  let consumptionClasses = 'bg-red-100 text-red-800';
  if (consumptionValue <= 0.2) {
    consumptionClasses = 'bg-green-100 text-green-800';
  } else if (consumptionValue <= 0.3) {
    consumptionClasses = 'bg-yellow-100 text-yellow-800';
  }

  // Calculate time range
  const startTime = new Date(session[0].created_at);
  const endTime = new Date(session[session.length - 1].created_at);
  const timeRangeString = `${formatTimeForDisplay(startTime)} - ${formatTimeForDisplay(endTime)}`;

  return (
    <div className="bg-white rounded-xl border border-slate-200 mb-3 transition-all duration-200 hover:border-blue-500 hover:shadow-sm">
      <button
        className="flex justify-between items-center w-full p-4 text-left"
        onClick={() => setIsOpen(!isOpen)}
      >
        <div className="flex items-baseline space-x-3">
          <h2 className="font-bold text-lg text-slate-800">Session {totalSessions - index}</h2>
          <p className="text-sm text-slate-500 font-mono">{timeRangeString}</p>
        </div>
        <div className="flex items-center space-x-2">
            <span className="text-xs font-semibold bg-slate-100 text-slate-600 px-2 py-1 rounded-md">{count} Klicks</span>
            <span className={`text-xs font-bold px-2 py-1 rounded-md ${consumptionClasses}`}>{sessionConsumption}g</span>
            <ChevronIcon isOpen={isOpen} />
        </div>
      </button>

      {isOpen && (
        <div className="px-4 pb-4">
          <div className="mt-2 pt-4 space-y-2 border-t border-slate-100">
            {session.map(row => (
              <div key={row.id} className="flex justify-between text-sm font-mono text-slate-600">
                <span>{formatDateForDisplay(new Date(row.created_at))}</span>
                <span className="font-semibold">{row.duration}s</span>
              </div>
            ))}
          </div>
        </div>
      )}
    </div>
  );
};

// --- REDESIGNED MAIN APP ---

function App() {
  const [data, setData] = useState<ApiResponse | null>(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    const loadData = async () => {
      try {
        setLoading(true);
        const result = await fetchSessions();
        setData(result);
      } catch (e: any) {
        setError(e.message);
      } finally {
        setLoading(false);
      }
    };
    loadData();
  }, []);

  return (
    <div className="bg-slate-50 font-sans min-h-screen">
      <main className="max-w-2xl mx-auto py-10 px-4">
        <header className="pb-6 border-b border-slate-200 mb-6">
          <h1 className="text-3xl font-bold text-slate-900">Heizbox Sessions</h1>
          {data && <div className="font-mono text-slate-600 mt-1">Verbrauch: {data.totalConsumption}g</div>}
        </header>

        {loading && <p className="text-slate-500">Lade Daten...</p>}
        {error && <p className="text-red-600 font-semibold">Fehler beim Laden: {error}</p>}

        {data && (
          <>
            {data.sessions && data.sessions.length > 0 ? (
              <div>
                {data.sessions.map((session, index) => (
                  <SessionCard
                    key={session[0]?.id || index}
                    session={session}
                    index={index}
                    totalSessions={data.sessions.length}
                  />
                ))}
              </div>
            ) : (
              !loading && <p className="text-slate-500 text-center py-8">Keine Sessions im ausgewählten Zeitraum gefunden.</p>
            )}
          </>
        )}
      </main>
    </div>
  );
}

export default App;

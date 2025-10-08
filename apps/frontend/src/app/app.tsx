import { useState, useEffect } from 'react';
import type { ApiResponse } from '../../libs/types/src/interfaces'; // Adjusted path for ApiResponse
import { fetchSessions } from '../api'; // Adjusted path
import { SessionCard } from '@heizbox/ui'; // Import SessionCard from the ui library

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
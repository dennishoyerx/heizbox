import { useState, useEffect } from 'react';
import type { ApiResponse } from '@heizbox/types';
import { fetchSessions } from '../api';
import { SessionCard } from './components/SessionCard';

// --- REDESIGNED MAIN APP ---

function App() {
  const [data, setData] = useState<ApiResponse | null>(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [deviceIsOn, setDeviceIsOn] = useState<boolean>(false); // New state for device status

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

  useEffect(() => {
    // For now, hardcode a deviceId. In a real app, this would come from user context or a config.
    const deviceId = 'my-esp32-device';
    const backendBaseUrl = import.meta.env.VITE_API_URL || 'http://localhost:8787'; // Assuming backend runs on 8787 locally
    const wsUrl = `${backendBaseUrl.replace('http', 'ws')}/ws/status?deviceId=${deviceId}&type=frontend`;

    const ws = new WebSocket(wsUrl);

    ws.onopen = () => {
      console.log('WebSocket connected to device status (frontend)');
    };

    ws.onmessage = (event) => {
      const message = JSON.parse(event.data);
      console.log('Received WebSocket message from device:', message);
      if (message && typeof message.isOn === 'boolean') {
        setDeviceIsOn(message.isOn);
      }
      // TODO: Handle other types of messages from the device
    };

    ws.onclose = () => {
      console.log('WebSocket disconnected from device status');
      setDeviceIsOn(false); // Device is off if WebSocket disconnects
    };

    ws.onerror = (err) => {
      console.error('WebSocket error:', err);
      setDeviceIsOn(false);
    };

    return () => {
      ws.close();
    };
  }, []); // Run once on component mount

  return (
    <div className="bg-slate-50 font-sans min-h-screen">
      <main className="max-w-2xl mx-auto py-10 px-4">
        <header className="pb-6 border-b border-slate-200 mb-6">
          <h1 className="text-3xl font-bold text-slate-900">Heizbox Sessions</h1>
          {data && <div className="font-mono text-slate-600 mt-1">Verbrauch: {data.totalConsumption}g</div>}
          <div className="mt-2 text-sm text-slate-500">
            Gerät Status: <span className={`font-semibold ${deviceIsOn ? 'text-green-500' : 'text-red-500'}`}>
              {deviceIsOn ? 'Online' : 'Offline'}
            </span>
          </div>
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
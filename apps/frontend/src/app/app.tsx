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
  const [deviceIsHeating, setDeviceIsHeating] = useState<boolean>(false); // New state for heating status



  useEffect(() => {
    const fetchInitialStatus = async () => {
      try {
        const deviceId = 'my-esp32-device';
        const backendBaseUrl = import.meta.env.VITE_PUBLIC_API_URL || 'http://127.0.0.1:8787';
        const response = await fetch(`${backendBaseUrl}/api/device-status/${deviceId}/status`);
        if (response.ok) {
          const status = await response.json();
          setDeviceIsOn(status.isOn);
          setDeviceIsHeating(status.isHeating);
        } else {
          console.error('Failed to fetch initial device status', response.statusText);
        }
      } catch (error) {
        console.error('Error fetching initial device status:', error);
      }
    };

    fetchInitialStatus();

    // For now, hardcode a deviceId. In a real app, this would come from user context or a config.
    const deviceId = 'my-esp32-device';
    const backendBaseUrl = import.meta.env.VITE_PUBLIC_API_URL || 'http://127.0.0.1:8787';
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
      if (message && typeof message.isHeating === 'boolean') {
        setDeviceIsHeating(message.isHeating);
      }
      if (message && message.type === 'sessionCreated') {
        console.log('New session created, re-fetching sessions...');
        loadData(); // Re-fetch sessions
      }
      // TODO: Handle other types of messages from the device
    };

    ws.onclose = () => {
      console.log('WebSocket disconnected from device status');
      setDeviceIsOn(false); // Device is off if WebSocket disconnects
      setDeviceIsHeating(false); // Device is not heating if WebSocket disconnects
    };

    ws.onerror = (err) => {
      console.error('WebSocket error:', err);
      setDeviceIsOn(false);
      setDeviceIsHeating(false);
    };

    return () => {
      // ws.close(); // Temporarily commented out for debugging
    };
  }, [loadData]); // Run once on component mount

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
          <div className="mt-1 text-sm text-slate-500">
            Heizstatus: <span className={`font-semibold ${deviceIsHeating ? 'text-orange-500' : 'text-gray-500'}`}>
              {deviceIsHeating ? 'Heizt' : 'Inaktiv'}
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
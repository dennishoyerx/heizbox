import { useState, useEffect } from 'react';
import { Routes, Route } from 'react-router-dom';
import type { ApiResponse } from '@heizbox/types';
import { fetchHeatCycles } from '../api';
import { HeatCycleCard } from './components';
import Header from './components/Header';
import { UsagePage } from './usage';
import { Flex, Text } from '@radix-ui/themes';

// --- REDESIGNED MAIN APP ---

function App() {
  const [data, setData] = useState<ApiResponse | null>(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [deviceIsOn, setDeviceIsOn] = useState<boolean>(false); // New state for device status
  const [deviceIsHeating, setDeviceIsHeating] = useState<boolean>(false); // New state for heating status
  
  const loadData = async () => {
      try {
        setLoading(true);
        const result = await fetchHeatCycles();
        setData(result);
      } catch (e: unknown) {
        if (e instanceof Error) {
          setError(e.message);
        }
      } finally {
        setLoading(false);
      }
  };

  useEffect(() => {
    loadData();
  }, []);

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
      if (message && message.type === 'heatCycleCreated') {
        console.log('New heat cycle created, re-fetching heat cycles...');
        loadData(); // Re-fetch heat cycles
      }
      // TODO: Handle other types of messages from the device
    };

    ws.onclose = () => {
      console.log('WebSocket disconnected from device status');
      setDeviceIsOn(false);
      setDeviceIsHeating(false); 
    };

    ws.onerror = (err) => {
      console.error('WebSocket error:', err);
      setDeviceIsOn(false);
      setDeviceIsHeating(false);
    };

    return () => {
      // ws.close(); // Temporarily commented out for debugging
    };
  }, []);

  return (
    <Flex direction="column" gap="4">
      <Header
        deviceName="Heizbox"
        deviceStatus={deviceIsOn ? 'Online' : 'Offline'}
        heatingStatus={deviceIsHeating ? 'Heizt' : 'Inaktiv'}
      />

      <main>
        {data && <Text>Verbrauch: {data.totalConsumption}g</Text>}

        <Routes>
          <Route path="/" element={
            <Flex direction="column" gap="3">
              {loading && <Text>Lade Daten...</Text>}
              {error && <Text color="red">Fehler beim Laden: {error}</Text>}

              {data && (
                <>
                  {data.heatCycles && data.heatCycles.length > 0 ? (
                    <Flex direction="column" gap="3">
                      {data.heatCycles.map((heatCycle, index) => (
                        <HeatCycleCard
                          key={heatCycle.id || index}
                          heatCycle={heatCycle}
                          index={index}
                          totalHeatCycles={data.heatCycles.length}
                        />
                      ))}
                    </Flex>
                  ) : (
                    !loading && <Text>Keine Heat Cycles im ausgewählten Zeitraum gefunden.</Text>
                  )}
                </>
              )}
            </Flex>
          } />
          <Route path="/usage" element={<UsagePage />} />
        </Routes>
      </main>
    </Flex>
  );
}

export default App;
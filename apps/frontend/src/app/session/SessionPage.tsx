import { useState, useEffect, useCallback } from "react";
import type { ApiResponse } from "@heizbox/types";
import { fetchHeatCycles } from "../../api";
import { SessionCard } from "./SessionCard";
import SessionHeader from "./SessionHeader";
import { Flex, Text } from "@radix-ui/themes";
import { useWebSocketEvent } from "../WebSocketContext";

function SessionPage({ isHeating }: { isHeating: boolean }) {
  const [data, setData] = useState<ApiResponse | null>(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  const loadData = async () => {
    try {
      setLoading(true);
      const result = await fetchHeatCycles();
      setData(result);
      setError(null);
    } catch (e: unknown) {
      if (e instanceof Error) {
        setError(e.message);
      }
    } finally {
      setLoading(false);
    }
  };

  // Initial Load
  useEffect(() => {
    loadData();
  }, []);

  // Auf heatCycleCompleted Event reagieren - Daten neu laden
  useWebSocketEvent(
    "heatCycleCompleted",
    useCallback((event) => {
      console.log("Heat cycle completed, reloading data...", event);
      loadData();
    }, []),
  );

  // Auf heatCycleCreated Event reagieren - auch neu laden
  useWebSocketEvent(
    "heatCycleCreated",
    useCallback((event) => {
      console.log("New heat cycle created, reloading data...", event);
      loadData();
    }, []),
  );

  return (
    <Flex direction="column" gap="3" maxWidth="600px" className="mx-auto">
      <SessionHeader isHeating={isHeating} />
      {data && <Text>Verbrauch: {data.totalConsumption}g</Text>}
      {loading && <Text>Lade Daten...</Text>}
      {error && <Text color="red">Fehler beim Laden: {error}</Text>}
      {data && (
        <>
          {data.heatCycles && data.heatCycles.length > 0 ? (
            <Flex direction="column" gap="3">
              {data.heatCycles.map((heatCycle, index) => (
                <SessionCard
                  key={heatCycle[0]?.id || index}
                  heatCycle={heatCycle}
                  index={index}
                  totalHeatCycles={data.heatCycles.length}
                />
              ))}
            </Flex>
          ) : (
            !loading && (
              <Text>Keine Heat Cycles im ausgewählten Zeitraum gefunden.</Text>
            )
          )}
        </>
      )}
    </Flex>
  );
}

export default SessionPage;

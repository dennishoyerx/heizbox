import {
  createContext,
  useContext,
  useEffect,
  useState,
  useCallback,
  ReactNode,
} from "react";

// Event-Typen definieren
export type WebSocketEvent =
  | { type: "heatCycleCreated"; data?: any }
  | { type: "heatCycleCompleted"; data?: any }
  | { type: "deviceStatusChanged"; isOn: boolean; isHeating: boolean }
  | { type: "error"; error: string };

type EventListener = (event: WebSocketEvent) => void;

interface WebSocketContextValue {
  isConnected: boolean;
  deviceIsOn: boolean;
  deviceIsHeating: boolean;
  addEventListener: (listener: EventListener) => () => void;
  sendMessage: (message: any) => void;
}

const WebSocketContext = createContext<WebSocketContextValue | null>(null);

export const useWebSocket = () => {
  const context = useContext(WebSocketContext);
  if (!context) {
    throw new Error("useWebSocket must be used within WebSocketProvider");
  }
  return context;
};

// Custom Hook für spezifische Events
export const useWebSocketEvent = (
  eventType: WebSocketEvent["type"],
  handler: (event: WebSocketEvent) => void,
) => {
  const { addEventListener } = useWebSocket();

  useEffect(() => {
    const unsubscribe = addEventListener((event) => {
      if (event.type === eventType) {
        handler(event);
      }
    });
    return unsubscribe;
  }, [addEventListener, eventType, handler]);
};

interface WebSocketProviderProps {
  children: ReactNode;
  deviceId?: string;
}

export const WebSocketProvider = ({
  children,
  deviceId = "my-esp32-device",
}: WebSocketProviderProps) => {
  const [isConnected, setIsConnected] = useState(false);
  const [deviceIsOn, setDeviceIsOn] = useState(false);
  const [deviceIsHeating, setDeviceIsHeating] = useState(false);
  const [listeners, setListeners] = useState<Set<EventListener>>(new Set());
  const [ws, setWs] = useState<WebSocket | null>(null);
  const reconnectTimeoutRef = useState<NodeJS.Timeout | null>(null)[0];

  // Event Broadcasting
  const broadcastEvent = useCallback(
    (event: WebSocketEvent) => {
      listeners.forEach((listener) => listener(event));
    },
    [listeners],
  );

  // Listener hinzufügen/entfernen
  const addEventListener = useCallback((listener: EventListener) => {
    setListeners((prev) => new Set(prev).add(listener));
    return () => {
      setListeners((prev) => {
        const next = new Set(prev);
        next.delete(listener);
        return next;
      });
    };
  }, []);

  // Message senden
  const sendMessage = useCallback(
    (message: any) => {
      if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify(message));
      }
    },
    [ws],
  );

  useEffect(() => {
    // Initial Status fetchen
    const fetchInitialStatus = async () => {
      try {
        const backendBaseUrl =
          import.meta.env.VITE_PUBLIC_API_URL || "http://127.0.0.1:8787";
        const response = await fetch(
          `${backendBaseUrl}/api/device-status/${deviceId}/status`,
        );
        if (response.ok) {
          const status = await response.json();
          setDeviceIsOn(status.isOn);
          setDeviceIsHeating(status.isHeating);
        }
      } catch (error) {
        console.error("Error fetching initial device status:", error);
      }
    };

    fetchInitialStatus();

    // WebSocket Connection
    const backendBaseUrl =
      import.meta.env.VITE_PUBLIC_API_URL || "http://127.0.0.1:8787";
    const wsUrl = `${backendBaseUrl.replace("http", "ws")}/ws/status?deviceId=${deviceId}&type=frontend`;
    const websocket = new WebSocket(wsUrl);

    websocket.onopen = () => {
      console.log("WebSocket connected");
      setIsConnected(true);
    };

    websocket.onmessage = (event) => {
      const message = JSON.parse(event.data);

      // Device Status Updates
      if (typeof message.isOn === "boolean") {
        setDeviceIsOn(message.isOn);
      }
      if (typeof message.isHeating === "boolean") {
        setDeviceIsHeating(message.isHeating);
      }

      // Event Broadcasting für spezifische Events
      if (message.type === "heatCycleCreated") {
        broadcastEvent({ type: "heatCycleCreated", data: message });
      } else if (message.type === "heatCycleCompleted") {
        broadcastEvent({ type: "heatCycleCompleted", data: message });
      }

      // Generisches Status-Event
      if (
        typeof message.isOn === "boolean" ||
        typeof message.isHeating === "boolean"
      ) {
        broadcastEvent({
          type: "deviceStatusChanged",
          isOn: message.isOn ?? deviceIsOn,
          isHeating: message.isHeating ?? deviceIsHeating,
        });
      }
    };

    websocket.onclose = () => {
      console.log("WebSocket disconnected");
      setIsConnected(false);
      setDeviceIsOn(false);
      setDeviceIsHeating(false);
    };

    websocket.onerror = (err) => {
      console.error("WebSocket error:", err);
      setIsConnected(false);
      broadcastEvent({ type: "error", error: "Connection error" });
    };

    setWs(websocket);

    return () => {
      websocket.close();
    };
  }, [deviceId, broadcastEvent]);

  const value: WebSocketContextValue = {
    isConnected,
    deviceIsOn,
    deviceIsHeating,
    addEventListener,
    sendMessage,
  };

  return (
    <WebSocketContext.Provider value={value}>
      {children}
    </WebSocketContext.Provider>
  );
};

import {
  createContext,
  useContext,
  useEffect,
  useState,
  useCallback,
  ReactNode,
} from "react";
import type {
  ServerWebSocketMessage,
  ClientWebSocketMessage,
  DeviceStatusPayload,
} from "@heizbox/types";

// Der Event-Typ ist nun direkt die Nachricht vom Server.
export type WebSocketEvent = ServerWebSocketMessage;

type EventListener = (event: WebSocketEvent) => void;

interface WebSocketContextValue {
  isConnected: boolean;
  deviceIsOn: boolean;
  deviceIsHeating: boolean;
  addEventListener: (listener: EventListener) => () => void;
  sendMessage: (message: ClientWebSocketMessage) => void;
}

const WebSocketContext = createContext<WebSocketContextValue | null>(null);

export const useWebSocket = () => {
  const context = useContext(WebSocketContext);
  if (!context) {
    throw new Error("useWebSocket must be used within WebSocketProvider");
  }
  return context;
};

// Custom Hook für spezifische Events, jetzt voll typsicher
export const useWebSocketEvent = <T extends WebSocketEvent["type"]>(
  eventType: T,
  handler: (event: Extract<WebSocketEvent, { type: T }>) => void,
) => {
  const { addEventListener } = useWebSocket();

  useEffect(() => {
    const unsubscribe = addEventListener((event) => {
      if (event.type === eventType) {
        handler(event as Extract<WebSocketEvent, { type: T }>);
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

  const broadcastEvent = useCallback(
    (event: WebSocketEvent) => {
      listeners.forEach((listener) => listener(event));
    },
    [listeners],
  );

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

  const sendMessage = useCallback(
    (message: ClientWebSocketMessage) => {
      if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify(message));
      }
    },
    [ws],
  );

  useEffect(() => {
    const fetchInitialStatus = async () => {
      try {
        const backendBaseUrl =
          import.meta.env.VITE_PUBLIC_API_URL || "http://127.0.0.1:8787";
        const response = await fetch(
          `${backendBaseUrl}/api/device-status/${deviceId}/status`,
        );
        if (response.ok) {
          const status = (await response.json()) as DeviceStatusPayload;
          setDeviceIsOn(status.isOn);
          setDeviceIsHeating(status.isHeating);
        }
      } catch (error) {
        console.error("Error fetching initial device status:", error);
      }
    };

    fetchInitialStatus();

    const backendBaseUrl =
      import.meta.env.VITE_PUBLIC_API_URL || "http://127.0.0.1:8787";
    const wsUrl = `${backendBaseUrl.replace("http", "ws")}/ws?deviceId=${deviceId}&type=frontend`;
    const websocket = new WebSocket(wsUrl);

    websocket.onopen = () => {
      console.log("WebSocket connected");
      setIsConnected(true);
    };

    websocket.onmessage = (event) => {
      const message = JSON.parse(event.data) as ServerWebSocketMessage;

      // Broadcast des spezifischen Events
      broadcastEvent(message);

      // Zusätzlich den lokalen Gerätestatus aktualisieren, falls es ein Status-Event ist
      if (message.type === "deviceStatusChanged") {
        setDeviceIsOn(message.payload.isOn);
        setDeviceIsHeating(message.payload.isHeating);
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
      broadcastEvent({
        type: "error",
        payload: { message: "Connection error" },
      });
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

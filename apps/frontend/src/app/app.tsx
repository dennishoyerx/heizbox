import { useState, useEffect } from "react";
import { Routes, Route } from "react-router-dom";
import Header from "./components/Header";
import { UsagePage } from "./usage";
import { SessionPage } from "./session";
import { Box, Flex, Theme, ThemePanel } from "@radix-ui/themes";

function App() {
  const [deviceIsOn, setDeviceIsOn] = useState<boolean>(false); // New state for device status
  const [deviceIsHeating, setDeviceIsHeating] = useState<boolean>(false); // New state for heating status
  const [sessionKey, setSessionKey] = useState(0);
  const [theme, setTheme] = useState(localStorage.getItem("theme") || "dark");

  useEffect(() => {
    const root = window.document.documentElement;
    if (theme === "dark") {
      root.classList.add("dark");
    } else {
      root.classList.remove("dark");
    }
    localStorage.setItem("theme", theme);
  }, [theme]);

  const toggleTheme = () => {
    setTheme(theme === "light" ? "dark" : "light");
  };

  useEffect(() => {
    const fetchInitialStatus = async () => {
      try {
        const deviceId = "my-esp32-device";
        const backendBaseUrl =
          import.meta.env.VITE_PUBLIC_API_URL || "http://127.0.0.1:8787";
        const response = await fetch(
          `${backendBaseUrl}/api/device-status/${deviceId}/status`,
        );
        if (response.ok) {
          const status = await response.json();
          setDeviceIsOn(status.isOn);
          setDeviceIsHeating(status.isHeating);
        } else {
          console.error(
            "Failed to fetch initial device status",
            response.statusText,
          );
        }
      } catch (error) {
        console.error("Error fetching initial device status:", error);
      }
    };

    fetchInitialStatus();

    const deviceId = "my-esp32-device";
    const backendBaseUrl =
      import.meta.env.VITE_PUBLIC_API_URL || "http://127.0.0.1:8787";
    const wsUrl = `${backendBaseUrl.replace("http", "ws")}/ws/status?deviceId=${deviceId}&type=frontend`;

    const ws = new WebSocket(wsUrl);

    ws.onopen = () => {
      console.log("WebSocket connected to device status (frontend)");
    };

    ws.onmessage = (event) => {
      const message = JSON.parse(event.data);
      if (message && typeof message.isOn === "boolean") {
        setDeviceIsOn(message.isOn);
      }
      if (message && typeof message.isHeating === "boolean") {
        setDeviceIsHeating(message.isHeating);
      }
      if (message && message.type === "heatCycleCreated") {
        setSessionKey((prevKey) => prevKey + 1);
      }
    };

    ws.onclose = () => {
      setDeviceIsOn(false);
      setDeviceIsHeating(false);
    };

    ws.onerror = (err) => {
      setDeviceIsOn(false);
      setDeviceIsHeating(false);
    };

    return () => {
      // ws.close();
    };
  }, []);

  return (
    <Theme
      appearance={theme as "light" | "dark"}
      grayColor="slate"
      accentColor="orange"
      radius="medium"
    >
      <Box className="min-h-screen">
        <Flex direction="column" gap="4">
          <Header
            deviceName="Heizbox"
            deviceStatus={deviceIsOn ? "Online" : "Offline"}
            heatingStatus={deviceIsHeating ? "Heizt" : "Inaktiv"}
            theme={theme}
            toggleTheme={toggleTheme}
          />

          <main className="p-4">
            <Routes>
              <Route
                path="/"
                element={
                  <SessionPage key={sessionKey} isHeating={deviceIsHeating} />
                }
              />
              <Route path="/usage" element={<UsagePage theme={theme} />} />
            </Routes>
          </main>
        </Flex>
      </Box>
    </Theme>
  );
}

export default App;

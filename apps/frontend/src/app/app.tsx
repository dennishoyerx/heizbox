import { useState, useEffect } from "react";
import { Routes, Route } from "react-router-dom";
import Header from "./components/Header";
import { UsagePage } from "./usage";
import { SessionPage } from "./session";
import { Box, Flex, Theme } from "@radix-ui/themes";
import { WebSocketProvider, useWebSocket } from "./WebSocketContext";
import { ConnectionStatus } from "./components/ConnectionStatus";

function AppContent() {
  const { deviceIsOn, deviceIsHeating } = useWebSocket();
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
                element={<SessionPage isHeating={deviceIsHeating} />}
              />
              <Route path="/usage" element={<UsagePage theme={theme} />} />
            </Routes>
          </main>
        </Flex>
      </Box>
      <ConnectionStatus />
    </Theme>
  );
}

function App() {
  return (
    <WebSocketProvider deviceId="my-esp32-device">
      <AppContent />
    </WebSocketProvider>
  );
}

export default App;

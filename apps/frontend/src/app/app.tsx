import { Routes, Route } from "react-router-dom";
import Header from "./components/Header";
import { UsagePage } from "./usage";
import { SessionPage } from "./session";
import { Box, Flex, Theme } from "@radix-ui/themes";
import { WebSocketProvider, useWebSocket } from "./WebSocketContext";
import { ConnectionStatus } from "./components/ConnectionStatus";
import { useTheme } from "./hooks/useTheme";

function AppContent() {
  const { deviceIsOn, deviceIsHeating } = useWebSocket();
  const { theme, toggleTheme } = useTheme();

  return (
    <Theme
      appearance={theme}
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
          <main>
            <Routes>
              <Route path="/" element={<SessionPage />} />
              <Route path="/usage" element={<UsagePage />} />
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

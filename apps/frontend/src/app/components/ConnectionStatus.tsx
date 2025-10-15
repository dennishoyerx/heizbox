import { useWebSocket } from "../WebSocketContext";
import { Box, Flex, Text, Badge } from "@radix-ui/themes";
import { WifiHighIcon, WifiSlashIcon } from "@phosphor-icons/react";

export const ConnectionStatus = () => {
  const { isConnected } = useWebSocket();

  if (isConnected) {
    return null; // Nichts anzeigen wenn verbunden
  }

  return (
    <Box
      style={{
        position: "fixed",
        bottom: "1rem",
        right: "1rem",
        zIndex: 1000,
      }}
    >
      <Badge color="red" size="2">
        <Flex gap="2" align="center">
          <WifiSlashIcon size={16} />
          <Text>Verbindung unterbrochen</Text>
        </Flex>
      </Badge>
    </Box>
  );
};

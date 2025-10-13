import React from 'react';
import { Link } from 'react-router-dom';
import { Flex, Heading, DropdownMenu, Button, Text } from '@radix-ui/themes';
import { List, Sun, Moon, Circle, CircleNotch , Fire } from '@phosphor-icons/react';
import { Box } from '@radix-ui/themes/src/index.js';

interface HeaderProps {
  deviceName: string;
  deviceStatus: string;
  heatingStatus: string;
  theme: string;
  toggleTheme: () => void;
}

const Header: React.FC<HeaderProps> = ({ deviceName, deviceStatus, heatingStatus, theme, toggleTheme }) => {
  return (
    <Box className="shadow-sm">
      <Flex direction="column" gap="2" p="4" className="shadow-sm mx-auto" maxWidth="600px">
        <Flex justify="between" align="center">
          <Flex gap="2" align="center">
            <Heading>{deviceName}</Heading>
          </Flex>
                  <Flex gap="2" align="center">
                      <Flex>
                          {heatingStatus === 'Heizt' && <Fire size={24} color="orange" />}                      </Flex>

                          {deviceStatus === 'Online' && <Circle weight="bold" size={20} color='green' />}
                          {deviceStatus === 'Offline' && <CircleNotch weight="bold" size={20} color='red' />}
          <DropdownMenu.Root>
            <DropdownMenu.Trigger>
              <Button variant="ghost">
                <List size={24} />
              </Button>
            </DropdownMenu.Trigger>
            <DropdownMenu.Content>
              <DropdownMenu.Item asChild>
                <Link to="/">Heat Cycles</Link>
              </DropdownMenu.Item>
              <DropdownMenu.Item asChild>
                <Link to="/usage">Usage Statistics</Link>
              </DropdownMenu.Item>
              <DropdownMenu.Separator />
              <DropdownMenu.Item onClick={toggleTheme}>
                <Flex gap="2" align="center">
                  {theme === 'light' ? <Moon size={16} /> : <Sun size={16} />}
                  {theme === 'light' ? 'Dark Mode' : 'Light Mode'}
                </Flex>
              </DropdownMenu.Item>
            </DropdownMenu.Content>
          </DropdownMenu.Root>
          </Flex>
        </Flex>
      </Flex>
    </Box>
  );
};

export default Header;

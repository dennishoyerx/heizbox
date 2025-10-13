import React from 'react';
import { Link } from 'react-router-dom';
import { Flex, Heading, DropdownMenu, Button, Text } from '@radix-ui/themes';
import { List, Sun, Moon } from '@phosphor-icons/react';

interface HeaderProps {
  deviceName: string;
  deviceStatus: string;
  heatingStatus: string;
  theme: string;
  toggleTheme: () => void;
}

const Header: React.FC<HeaderProps> = ({ deviceName, deviceStatus, heatingStatus, theme, toggleTheme }) => {
  return (
    <div className="shadow-sm bg-white dark:bg-gray-800">
      <Flex direction="column" gap="2" p="4">
        <Flex justify="between" align="center">
          <Heading>{deviceName}</Heading>
          <DropdownMenu.Root>
            <DropdownMenu.Trigger>
              <Button variant="soft">
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
        <Text size="2" color="gray">Gerät Status: <Text weight="bold">{deviceStatus}</Text></Text>
        <Text size="2" color="gray">Heizstatus: <Text weight="bold">{heatingStatus}</Text></Text>
      </Flex>
    </div>
  );
};

export default Header;

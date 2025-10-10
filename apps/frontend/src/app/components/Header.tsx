import React from 'react';
import { Link } from 'react-router-dom';
import { Flex, Heading, DropdownMenu, Button, Text } from '@radix-ui/themes';
import { List } from '@phosphor-icons/react';

interface HeaderProps {
  deviceName: string;
  deviceStatus: string;
  heatingStatus: string;
}

const Header: React.FC<HeaderProps> = ({ deviceName, deviceStatus, heatingStatus }) => {
  return (
    <Flex direction="column" gap="2" p="4" className="shadow-sm">
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
              <Link to="/">Sessions</Link>
            </DropdownMenu.Item>
            <DropdownMenu.Item asChild>
              <Link to="/usage">Usage Statistics</Link>
            </DropdownMenu.Item>
          </DropdownMenu.Content>
        </DropdownMenu.Root>
      </Flex>
      <Text size="2" color="gray">Gerät Status: <Text weight="bold">{deviceStatus}</Text></Text>
      <Text size="2" color="gray">Heizstatus: <Text weight="bold">{heatingStatus}</Text></Text>
    </Flex>
  );
};

export default Header;

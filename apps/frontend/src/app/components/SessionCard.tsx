import * as Collapsible from '@radix-ui/react-collapsible';
import type { SessionGroup } from '@heizbox/types';
import { formatDateForDisplay, formatTimeForDisplay, calculateConsumption } from '@heizbox/utils';
import { CaretDown } from '@phosphor-icons/react';
import { Card, Flex, Text, Heading, Badge } from '@radix-ui/themes';

export const SessionCard = ({ session, index, totalSessions }: { session: SessionGroup, index: number, totalSessions: number }) => {
  const count = session.length;

  const sessionConsumption = calculateConsumption(count);
  const consumptionValue = parseFloat(sessionConsumption);

  let consumptionColor: "red" | "yellow" | "green" = 'red';
  if (consumptionValue <= 0.2) {
    consumptionColor = 'green';
  } else if (consumptionValue <= 0.3) {
    consumptionColor = 'yellow';
  }

  // Calculate time range
  const startTime = new Date(session[0].created_at);
  const endTime = new Date(session[session.length - 1].created_at);
  const timeRangeString = `${formatTimeForDisplay(startTime)} - ${formatTimeForDisplay(endTime)}`;

  return (
    <Card>
      <Collapsible.Root>
        <Collapsible.Trigger style={{ width: '100%' }}>
          <Flex justify="between" align="center">
            <Flex align="baseline" gap="3">
              <Heading size="3">Session {totalSessions - index}</Heading>
              <Text size="2" color="gray">{timeRangeString}</Text>
            </Flex>
            <Flex align="center" gap="3">
              <Badge>{count} Klicks</Badge>
              <Badge color={consumptionColor}>{sessionConsumption}g</Badge>
              <CaretDown />
            </Flex>
          </Flex>
        </Collapsible.Trigger>

        <Collapsible.Content>
          <Flex direction="column" gap="2" mt="4">
            {session.map(row => (
              <Flex key={row.id} justify="between">
                <Text size="2" color="gray">{formatDateForDisplay(new Date(row.created_at))}</Text>
                <Text size="2" weight="bold">{row.duration}s</Text>
              </Flex>
            ))}
          </Flex>
        </Collapsible.Content>
      </Collapsible.Root>
    </Card>
  );
};

import { useState } from 'react';
import type { HeatCycleGroup } from '@heizbox/types';
import { formatTimestampForTimeDisplay, calculateConsumption } from '@heizbox/utils';
import { Card, Flex, Text, Heading, Badge } from '@radix-ui/themes';
import { HeatCycleDetailModal } from './HeatCycleDetailModal';

export const HeatCycleCard = ({ heatCycle, index, totalHeatCycles }: { heatCycle: HeatCycleGroup, index: number, totalHeatCycles: number }) => {
  const [isModalOpen, setIsModalOpen] = useState(false);
  const count = heatCycle.length;

  const heatCycleConsumption = calculateConsumption(count);
  const consumptionValue = parseFloat(heatCycleConsumption);

  let consumptionColor: "red" | "yellow" | "green" = 'red';
  if (consumptionValue <= 0.2) {
    consumptionColor = 'green';
  } else if (consumptionValue <= 0.3) {
    consumptionColor = 'yellow';
  }

  // Calculate time range
  const startTimestamp = heatCycle[0].created_at;
  const endTimestamp = heatCycle[heatCycle.length - 1].created_at;
  const timeRangeString = `${formatTimestampForTimeDisplay(startTimestamp)} - ${formatTimestampForTimeDisplay(endTimestamp)}`;

  return (
    <>
      <Card>
        <Flex justify="between" align="center">
          <Flex align="baseline" gap="3" wrap="wrap" gapY="0">
            <Heading size="3">Heat Cycle {totalHeatCycles - index}</Heading>
            <Text size="2" color="gray">{timeRangeString}</Text>
          </Flex>
          <Flex align="center" gap="3">
            <button onClick={() => setIsModalOpen(true)} className="cursor-pointer">
              <Badge>{count} Klicks</Badge>
            </button>
            <Badge color={consumptionColor}>{heatCycleConsumption}g</Badge>
          </Flex>
        </Flex>
      </Card>
      <HeatCycleDetailModal 
        isOpen={isModalOpen} 
        onClose={() => setIsModalOpen(false)} 
        heatCycle={heatCycle} 
      />
    </>
  );
};

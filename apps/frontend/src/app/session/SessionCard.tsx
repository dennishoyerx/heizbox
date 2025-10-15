import { useState } from "react";
import type { HeatCycleGroup, HeatCycleRow } from "@heizbox/types";
import {
  formatTimestampForTimeDisplay,
  calculateConsumption,
} from "@heizbox/utils";
import { Card, Flex, Text, Heading, Badge } from "@radix-ui/themes";
import { SessionDetailModal } from "./SessionDetailModal";

export const SessionCard = ({
  heatCycle,
  index,
  totalHeatCycles,
}: {
  heatCycle: HeatCycleGroup;
  index: number;
  totalHeatCycles: number;
}) => {
  const [isModalOpen, setIsModalOpen] = useState(false);
  const count = heatCycle.length;
  const caps = heatCycle.filter((hc: HeatCycleRow) => hc.cycle === 1).length;

  const heatCycleConsumption = calculateConsumption(caps);
  const consumptionValue = parseFloat(heatCycleConsumption);

  let consumptionColor: "red" | "yellow" | "green" = "red";
  if (consumptionValue <= 0.2) {
    consumptionColor = "green";
  } else if (consumptionValue <= 0.3) {
    consumptionColor = "yellow";
  }

  // Calculate time range
  const startTimestamp = heatCycle[0].created_at;
  const endTimestamp = heatCycle[heatCycle.length - 1].created_at;
  const timeRangeString = `${formatTimestampForTimeDisplay(startTimestamp)} - ${formatTimestampForTimeDisplay(endTimestamp)}`;

  return (
    <>
      <Card onClick={() => setIsModalOpen(true)} className="cursor-pointer">
        <Flex justify="between" align="center">
          <Flex align="baseline" gap="3" wrap="wrap" gapY="0">
            <Heading size="3">Session {totalHeatCycles - index}</Heading>
            <Text size="2" color="gray">
              {timeRangeString}
            </Text>
          </Flex>
          <Flex align="center" gap="3">
            <Badge color="iris">
              <Flex align="center" direction="column">
                <div>{caps} Caps</div>
                <div>{count} Klicks</div>
              </Flex>
            </Badge>
            <Badge color={consumptionColor}>{heatCycleConsumption}g</Badge>
          </Flex>
        </Flex>
      </Card>
      <SessionDetailModal
        isOpen={isModalOpen}
        onClose={() => setIsModalOpen(false)}
        heatCycle={heatCycle}
      />
    </>
  );
};

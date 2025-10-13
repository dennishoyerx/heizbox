
import * as Dialog from '@radix-ui/react-dialog';
import { motion, AnimatePresence } from 'framer-motion';
import type { HeatCycleGroup } from '@heizbox/types';
import { formatTimestampForTimeDisplay } from '@heizbox/utils';
import { Text, Flex, Heading, Badge } from '@radix-ui/themes';
import { X } from '@phosphor-icons/react';

interface HeatCycleDetailModalProps {
  isOpen: boolean;
  onClose: () => void;
  heatCycle: HeatCycleGroup | null;
}

export const HeatCycleDetailModal = ({ isOpen, onClose, heatCycle }: HeatCycleDetailModalProps) => {
  if (!heatCycle) {
    return null;
  }

  const averageInterval = heatCycle.length > 1 
    ? heatCycle.slice(1).reduce((acc, row, index) => {
        const prevRow = heatCycle[index]; // index is of the sliced array, so it's row-1 from original
        return acc + (row.created_at - prevRow.created_at);
      }, 0) / (heatCycle.length - 1)
    : 0;

  return (
    <AnimatePresence>
      {isOpen && (
        <Dialog.Root open={isOpen} onOpenChange={onClose}>
          <Dialog.Portal forceMount>
            <Dialog.Overlay asChild>
              <motion.div
                className="fixed inset-0 bg-black/50"
                initial={{ opacity: 0 }}
                animate={{ opacity: 1 }}
                exit={{ opacity: 0 }}
              />
            </Dialog.Overlay>
            <Dialog.Content asChild>
              <motion.div
                className="fixed left-1/2 top-1/2 w-[90vw] max-w-md -translate-x-1/2 -translate-y-1/2 rounded-lg bg-gray-800 p-6 text-white shadow-lg"
                initial={{ opacity: 0, scale: 0.95 }}
                animate={{ opacity: 1, scale: 1 }}
                exit={{ opacity: 0, scale: 0.95 }}
                transition={{ duration: 0.2 }}
              >
                <Flex justify="between" align="center">
                  <Heading>Heat Cycle Details</Heading>
                  <Dialog.Close asChild>
                    <button className="rounded-full p-1 hover:bg-gray-700">
                      <X size={20} />
                    </button>
                  </Dialog.Close>
                </Flex>

                <Flex direction="column" gap="4" mt="4">
                  <Flex justify="between">
                    <Text color="gray">Total Clicks:</Text>
                    <Badge size="2">{heatCycle.length}</Badge>
                  </Flex>
                  <Flex justify="between">
                    <Text color="gray">Avg. Interval:</Text>
                    <Text>{averageInterval.toFixed(2)}s</Text>
                  </Flex>

                  <div>
                    <Text as="div" color="gray" mb="2">Timestamps:</Text>
                    <div className="max-h-60 overflow-y-auto rounded-md bg-gray-900 p-2">
                      {heatCycle.map((row) => (
                        <Flex key={row.id} justify="between" className="p-1">
                          <Text size="2">{formatTimestampForTimeDisplay(row.created_at)}</Text>
                          <Badge color="gray">{row.duration}s</Badge>
                        </Flex>
                      ))}
                    </div>
                  </div>
                </Flex>
              </motion.div>
            </Dialog.Content>
          </Dialog.Portal>
        </Dialog.Root>
      )}
    </AnimatePresence>
  );
};


import * as Dialog from '@radix-ui/react-dialog';
import { motion, AnimatePresence } from 'framer-motion';
import type { HeatCycleGroup } from '@heizbox/types';
import { formatTimestampForTimeDisplay } from '@heizbox/utils';
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
      const prevRow = heatCycle[index];
      return acc + (row.created_at - prevRow.created_at);
    }, 0) / (heatCycle.length - 1)
    : 0;

  return (
    <Dialog.Root open={isOpen} onOpenChange={onClose}>
      <AnimatePresence>
        {isOpen && (
          <Dialog.Portal forceMount>
            <Dialog.Overlay asChild>
              <motion.div
                className="fixed inset-0 bg-black/70"
                initial={{ opacity: 0 }}
                animate={{ opacity: 1 }}
                exit={{ opacity: 0 }}
              />
            </Dialog.Overlay>
            <Dialog.Content asChild>
              <motion.div
                className="fixed left-1/2 top-1/2 w-[90vw] max-w-md -translate-x-1/2 -translate-y-1/2"
                initial={{ opacity: 0, scale: 0.95 }}
                animate={{ opacity: 1, scale: 1 }}
                exit={{ opacity: 0, scale: 0.95 }}
                transition={{ duration: 0.2 }}
              >
                <div className="w-full max-w-md bg-slate-900 rounded-2xl shadow-xl border border-slate-700/50 p-5 space-y-4 text-white">
                  <header className="flex justify-between items-center border-b border-slate-800 pb-3">
                    <Dialog.Title className="text-lg font-semibold">Heat Cycle Details</Dialog.Title>
                    <Dialog.Close asChild>
                      <button className="text-slate-400 hover:text-white transition-colors rounded-full p-1">
                        <X size={20} />
                      </button>
                    </Dialog.Close>
                  </header>

                  <div className="grid grid-cols-2 gap-3 text-sm text-slate-300">
                    <div className="bg-slate-800/70 p-4 rounded-lg flex flex-col items-center justify-center space-y-1">
                      <span className="text-3xl font-bold text-cyan-400">{heatCycle.length}</span>
                      <span className="text-xs text-slate-400">Total Clicks</span>
                    </div>
                    <div className="bg-slate-800/70 p-4 rounded-lg flex flex-col items-center justify-center space-y-1">
                      <span className="text-3xl font-bold text-cyan-400">{averageInterval.toFixed(2)}s</span>
                      <span className="text-xs text-slate-400">Avg. Interval</span>
                    </div>
                  </div>

                  <section className="space-y-2 pt-2">
                    <h3 className="text-sm font-medium text-slate-400 uppercase tracking-wider px-1">Timestamps</h3>
                    <div className="bg-slate-800/50 rounded-lg max-h-60 overflow-y-auto divide-y divide-slate-800 border border-slate-800">
                      {heatCycle.map(row => (
                        <div key={row.id} className="flex justify-between px-3 py-1 text-sm hover:bg-slate-800 transition-colors">
                          <span>{formatTimestampForTimeDisplay(row.created_at)}</span>
                          <span className="font-mono text-slate-400">{row.duration}s</span>
                        </div>
                      ))}
                    </div>
                  </section>
                </div>
              </motion.div>
            </Dialog.Content>
          </Dialog.Portal>
        )}
      </AnimatePresence>
    </Dialog.Root>
  );
};

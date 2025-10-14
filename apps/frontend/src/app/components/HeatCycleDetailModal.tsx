import * as Dialog from '@radix-ui/react-dialog';
import { motion, AnimatePresence } from 'framer-motion';
import type { HeatCycleGroup } from '@heizbox/types';
import { formatTimestampForTimeDisplay } from '@heizbox/utils';
import { X } from '@phosphor-icons/react';
import { Badge } from '@radix-ui/themes';

interface HeatCycleDetailModalProps {
  isOpen: boolean;
  onClose: () => void;
  heatCycle: HeatCycleGroup | null;
}

export const HeatCycleDetailModal = ({ isOpen, onClose, heatCycle }: HeatCycleDetailModalProps) => {
  if (!heatCycle) {
    return null;
  }

  const averageInterval = heatCycle.length > 0
    ? heatCycle.reduce((acc, row) => acc + row.duration, 0) / heatCycle.length
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
                    <Dialog.Title className="text-lg font-semibold">Session Details</Dialog.Title>
                    <Dialog.Close asChild>
                      <button className="text-slate-400 hover:text-white transition-colors rounded-full p-1">
                        <X size={20} />
                      </button>
                    </Dialog.Close>
                  </header>

                  <div className="grid grid-cols-3 gap-3 text-sm text-slate-300">
                    <div className="bg-slate-800/70 p-4 rounded-lg flex flex-col items-center justify-center space-y-1">
                      <span className="text-3xl font-bold text-cyan-400">{heatCycle.filter(x => x.cycle === 1).length}</span>
                      <span className="text-xs text-slate-400">Caps</span>
                    </div>
                    <div className="bg-slate-800/70 p-4 rounded-lg flex flex-col items-center justify-center space-y-1">
                      <span className="text-3xl font-bold text-cyan-400">{heatCycle.length}</span>
                      <span className="text-xs text-slate-400">Clicks</span>
                    </div>
                    <div className="bg-slate-800/70 p-4 rounded-lg flex flex-col items-center justify-center space-y-1">
                      <span className="text-3xl font-bold text-cyan-400">{averageInterval.toFixed(2)}s</span>
                      <span className="text-xs text-slate-400">Avg. Interval</span>
                    </div>
                  </div>

                  <section className="space-y-2 pt-2">
                    <h3 className="text-sm font-medium text-slate-400 uppercase tracking-wider px-1">Caps</h3>
                    <div className="bg-slate-800/50 rounded-lg max-h-60 overflow-y-auto divide-y divide-slate-800 border border-slate-800">
                      {groupConsecutiveCycles(heatCycle).map((group, idx) => (
                        <div key={group.map(r => r.id).join('-')} className="flex justify-between px-3 py-1 text-sm hover:bg-slate-800 transition-colors">
                          <span>
                            {group.length > 1
                              ? `${formatTimestampForTimeDisplay(group[0].created_at)} - ${formatTimestampForTimeDisplay(group[group.length - 1].created_at)}`
                              : formatTimestampForTimeDisplay(group[0].created_at)
                            }
                          </span>
                          <div className="flex gap-1">
                            <span className="font-mono text-slate-400 flex gap-1">
                              {group.map((r) => <span>{r.duration}s</span>)}
                            </span>
                            {/*group.length > 1 ? (
                              <Badge size="2" color="indigo" variant="solid" className="px-2">
                                Cycles {group[0].cycle}-{group[group.length - 1].cycle}
                              </Badge>
                            ) : (
                              <Badge size="2" color="indigo" variant="outline" className="px-2">
                                {group[0].cycle}
                              </Badge>
                            )*/}
                          </div>
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

function groupConsecutiveCycles(cycles: HeatCycleGroup) {
  const groups: Array<HeatCycleGroup> = [];
  let currentGroup: HeatCycleGroup = [];

  for (let i = 0; i < cycles.length; i++) {
    const curr = cycles[i];
    const prev = cycles[i - 1];

    if (
      currentGroup.length === 0 ||
      (prev && curr.cycle === prev.cycle + 1 && curr.cycle <= 4)
    ) {
      currentGroup.push(curr);
    } else {
      groups.push(currentGroup);
      currentGroup = [curr];
    }

    // Gruppe beenden, wenn cycle 4 erreicht ist
    if (curr.cycle === 4) {
      groups.push(currentGroup);
      currentGroup = [];
    }
  }
  if (currentGroup.length) groups.push(currentGroup);
  return groups;
}

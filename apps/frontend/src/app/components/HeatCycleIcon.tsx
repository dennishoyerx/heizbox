import { Flame, ArrowClockwise } from "@phosphor-icons/react";

export function HeatCycleIcon() {
  return (
    <div className="relative w-6 h-6">
      <ArrowClockwise
        weight="duotone"
        className="absolute inset-0 text-slate-500/60 animate-spin-slow"
      />
      <Flame
        weight="fill"
        className="absolute inset-1 text-orange-500 drop-shadow-sm"
      />
    </div>
  );
}

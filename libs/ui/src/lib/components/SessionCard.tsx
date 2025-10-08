import { useState } from 'react';
import type { SessionGroup } from '@heizbox/types'; // Assuming @heizbox/types is available
import { formatDateForDisplay, formatTimeForDisplay, calculateConsumption } from '@heizbox/utils'; // Assuming @heizbox/utils is available
import { ChevronIcon } from './ChevronIcon'; // Import ChevronIcon from the same library

export const SessionCard = ({ session, index, totalSessions }: { session: SessionGroup, index: number, totalSessions: number }) => {
  const [isOpen, setIsOpen] = useState(false);
  const count = session.length;

  const sessionConsumption = calculateConsumption(count);
  const consumptionValue = parseFloat(sessionConsumption);

  let consumptionClasses = 'bg-red-100 text-red-800';
  if (consumptionValue <= 0.2) {
    consumptionClasses = 'bg-green-100 text-green-800';
  } else if (consumptionValue <= 0.3) {
    consumptionClasses = 'bg-yellow-100 text-yellow-800';
  }

  // Calculate time range
  const startTime = new Date(session[0].created_at);
  const endTime = new Date(session[session.length - 1].created_at);
  const timeRangeString = `${formatTimeForDisplay(startTime)} - ${formatTimeForDisplay(endTime)}`;

  return (
    <div className="bg-white rounded-xl border border-slate-200 mb-3 transition-all duration-200 hover:border-blue-500 hover:shadow-sm">
      <button
        className="flex justify-between items-center w-full p-4 text-left"
        onClick={() => setIsOpen(!isOpen)}
      >
        <div className="flex items-baseline space-x-3">
          <h2 className="font-bold text-lg text-slate-800">Session {totalSessions - index}</h2>
          <p className="text-sm text-slate-500 font-mono">{timeRangeString}</p>
        </div>
        <div className="flex items-center space-x-2">
            <span className="text-xs font-semibold bg-slate-100 text-slate-600 px-2 py-1 rounded-md">{count} Klicks</span>
            <span className={`text-xs font-bold px-2 py-1 rounded-md ${consumptionClasses}`}>{sessionConsumption}g</span>
            <ChevronIcon isOpen={isOpen} />
        </div>
      </button>

      {isOpen && (
        <div className="px-4 pb-4">
          <div className="mt-2 pt-4 space-y-2 border-t border-slate-100">
            {session.map(row => (
              <div key={row.id} className="flex justify-between text-sm font-mono text-slate-600">
                <span>{formatDateForDisplay(new Date(row.created_at))}</span>
                <span className="font-semibold">{row.duration}s</span>
              </div>
            ))}
          </div>
        </div>
      )}
    </div>
  );
};

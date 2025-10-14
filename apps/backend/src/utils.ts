import type { HeatCycleRow } from '@heizbox/types';

const GROUP_SESSION_INTERVAL_MINUTES = 60;

export const formatDateForDB = (date: Date): number => {
  return Math.floor(date.getTime() / 1000);
};

export const getBerlinTimeRange = () => {
  const now = new Date();
  const nowBerlin = new Date(now.toLocaleString("en-US", { timeZone: "Europe/Berlin" }));

  const startBerlin = new Date(nowBerlin);
  startBerlin.setHours(9, 0, 0, 0);

  // If it's before 9 AM, we are interested in the range from yesterday 9 AM to today 9 AM.
  if (nowBerlin.getHours() < 9) {
    startBerlin.setDate(startBerlin.getDate() - 1);
  }

  const endBerlin = new Date(startBerlin);
  endBerlin.setDate(startBerlin.getDate() + 1);

  return {
    start: formatDateForDB(startBerlin),
    end: formatDateForDB(endBerlin),
  };
};

export const groupSessions = (rows: HeatCycleRow[]): HeatCycleRow[][] => {
  if (!rows || rows.length === 0) {
    return [];
  }

  const intervalMs = GROUP_SESSION_INTERVAL_MINUTES * 60 * 1000;
  const heatCycles: HeatCycleRow[][] = [];
  let currentGroup: HeatCycleRow[] = [rows[0]];

  for (let i = 1; i < rows.length; i++) {
    const currentRow = rows[i];
    const previousRow = rows[i - 1];
    const currentDate = new Date(Number(currentRow.created_at) * 1000);
    const previousDate = new Date(Number(previousRow.created_at) * 1000);

    if (currentDate.getTime() - previousDate.getTime() >= intervalMs) {
      heatCycles.push(currentGroup);
      currentGroup = [currentRow];
    } else {
      currentGroup.push(currentRow);
    }
  }

  heatCycles.push(currentGroup);
  return heatCycles.reverse();
};

export const calculateConsumption = (count: number): string => (0.05 * Math.ceil(count)).toFixed(2);

export const getMimeType = (path: string): string => {
  const extension = path.split('.').pop()?.toLowerCase();
  switch (extension) {
    case 'html': return 'text/html';
    case 'js': return 'application/javascript';
    case 'css': return 'text/css';
    case 'svg': return 'image/svg+xml';
    case 'png': return 'image/png';
    case 'jpg':
    case 'jpeg': return 'image/jpeg';
    case 'json': return 'application/json';
    default: return 'application/octet-stream';
  }
};

export const generateUuid = (): string => {
  return crypto.randomUUID();
};

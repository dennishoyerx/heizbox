import type { SessionRow } from '../types';

export const formatDateForDB = (date: Date): string => {
  return date.toISOString();
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

export const groupSessions = (rows: SessionRow[]): SessionRow[][] => {
  if (!rows || rows.length === 0) {
    return [];
  }

  const sessions: SessionRow[][] = [];
  let currentGroup: SessionRow[] = [rows[0]];

  for (let i = 1; i < rows.length; i++) {
    const currentRow = rows[i];
    const previousRow = rows[i - 1];
    const currentDate = new Date(currentRow.created_at);
    const previousDate = new Date(previousRow.created_at);

    if (currentDate.getTime() - previousDate.getTime() >= 3600000) { // 1 hour gap
      sessions.push(currentGroup);
      currentGroup = [currentRow];
    } else {
      currentGroup.push(currentRow);
    }
  }

  sessions.push(currentGroup);
  return sessions.reverse();
};

export const calculateConsumption = (count: number): string => (0.05 * Math.ceil(count / 2)).toFixed(2);

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

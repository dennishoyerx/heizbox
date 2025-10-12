export function formatDuration(seconds: number): string {
  const minutes = Math.floor(seconds / 60);
  const remainingSeconds = seconds % 60;
  return `${minutes}m ${remainingSeconds}s`;
}
export const formatTimestampForDateDisplay = (timestamp: number) => {
    const date = new Date(timestamp > 1e12 ? timestamp : timestamp * 1000);
    const pad = (n: number) => String(n).padStart(2, '0');
    return `${pad(date.getDate())}.${pad(date.getMonth() + 1)} ${pad(date.getHours())}:${pad(date.getMinutes())}`;
};

export const formatTimestampForTimeDisplay = (timestamp: number) => {
    const date = new Date(timestamp > 1e12 ? timestamp : timestamp * 1000);
    const pad = (n: number) => String(n).padStart(2, '0');
    return `${pad(date.getHours())}:${pad(date.getMinutes())}`;
};

export const formatDateForDisplay = (date: Date) => {
    const pad = (n: number) => String(n).padStart(2, '0');
    return `${pad(date.getDate())}.${pad(date.getMonth() + 1)} ${pad(date.getHours())}:${pad(date.getMinutes())}`;
};

export const formatTimeForDisplay = (date: Date) => {
  const pad = (n: number) => String(n).padStart(2, '0');
  return `${pad(date.getHours())}:${pad(date.getMinutes())}`;
};

export const calculateConsumption = (count: number) => (0.05 * Math.ceil(count / 2)).toFixed(2);
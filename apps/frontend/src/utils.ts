export const formatDateForDisplay = (date: Date) => {
  const berlinDate = new Date(date.toLocaleString("sv-SE", { timeZone: "Europe/Berlin" }));
  const pad = (n: number) => String(n).padStart(2, '0');
  return `${pad(berlinDate.getDate())}.${pad(berlinDate.getMonth() + 1)} ${pad(berlinDate.getHours())}:${pad(berlinDate.getMinutes())}`;
};

export const formatTimeForDisplay = (date: Date) => {
  const berlinDate = new Date(date.toLocaleString("sv-SE", { timeZone: "Europe/Berlin" }));
  const pad = (n: number) => String(n).padStart(2, '0');
  return `${pad(berlinDate.getHours())}:${pad(berlinDate.getMinutes())}`;
};

export const calculateConsumption = (count: number) => (0.05 * Math.ceil(count)).toFixed(2);

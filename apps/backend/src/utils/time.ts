export const formatDateForDB = (date: Date): number => {
  return Math.floor(date.getTime() / 1000);
};

export const getBerlinTimeRange = () => {
  const now = new Date();
  const nowBerlin = new Date(
    now.toLocaleString('en-US', { timeZone: 'Europe/Berlin' })
  );

  const startBerlin = new Date(nowBerlin);
  startBerlin.setHours(9, 0, 0, 0);

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

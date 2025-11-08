export const formatDateForDB = (date: Date): number => {
  return Math.floor(date.getTime() / 1000);
};

export const getBerlinTimeRange = (dayOffset = 0) => {
  const now = new Date();
  const nowBerlin = new Date(
    now.toLocaleString('en-US', { timeZone: 'Europe/Berlin' })
  );

  // Determine the start of the "current" logical day (which is 9am)
  const currentDayStart = new Date(nowBerlin);
  currentDayStart.setHours(9, 0, 0, 0);

  if (nowBerlin.getHours() < 9) {
    currentDayStart.setDate(currentDayStart.getDate() - 1);
  }

  // Now, apply the offset to this calculated start date
  const startBerlin = new Date(currentDayStart);
  startBerlin.setDate(startBerlin.getDate() + dayOffset);

  const endBerlin = new Date(startBerlin);
  endBerlin.setDate(endBerlin.getDate() + 1);

  return {
    start: formatDateForDB(startBerlin),
    end: formatDateForDB(endBerlin),
  };
};

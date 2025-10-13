import React, { useState, useEffect } from 'react';
import { fetchStatistics } from '../../api';
import { BarChart, Bar, XAxis, YAxis, Tooltip, Legend, ResponsiveContainer, CartesianGrid } from 'recharts';

interface HeatCycleData {
  created_at: number;
  duration: number;
}

interface StatisticsData {
  range: string;
  totalHeatCycles: number;
  totalDuration: number;
  heatCycles: HeatCycleData[];
}

const UsagePage: React.FC<{ theme: string }> = ({ theme }) => {
  const [dailyStats, setDailyStats] = useState<StatisticsData | null>(null);
  const [weeklyStats, setWeeklyStats] = useState<StatisticsData | null>(null);
  const [monthlyStats, setMonthlyStats] = useState<StatisticsData | null>(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    const loadStatistics = async () => {
      try {
        setLoading(true);
        const daily = await fetchStatistics('day');
        setDailyStats(daily);
        const weekly = await fetchStatistics('week');
        setWeeklyStats(weekly);
        const monthly = await fetchStatistics('month');
        setMonthlyStats(monthly);
      } catch (e: any) {
        setError(e.message);
      } finally {
        setLoading(false);
      }
    };

    loadStatistics();
  }, []);

  const processMonthlyHeatmapData = (heatCycles: HeatCycleData[]) => {
    const heatmap: { [key: number]: number } = {}; // dayOfMonth: count
    const maxDay = new Date(new Date().getFullYear(), new Date().getMonth() + 1, 0).getDate();
    heatCycles.forEach(heatCycle => {
      const date = new Date(heatCycle.created_at  *1000);
      const dayOfMonth = date.getDate();
      heatmap[dayOfMonth] = (heatmap[dayOfMonth] || 0) + 1;
    });

    const formattedHeatmapData: { day: number; count: number }[] = [];
    for (let i = 1; i <= maxDay; i++) {
      formattedHeatmapData.push({ day: i, count: heatmap[i] || 0 });
    }
    return formattedHeatmapData;
  };

  const monthlyHeatmapData = monthlyStats ? processMonthlyHeatmapData(monthlyStats.heatCycles) : [];
  const maxHeatCyclesPerDay = Math.max(...monthlyHeatmapData.map(data => data.count));

  const processHourlyData = (heatCycles: HeatCycleData[]) => {
    const hourlyCounts: { [key: number]: number } = {};
    for (let i = 0; i < 24; i++) {
      hourlyCounts[i] = 0;
    }

    heatCycles.forEach(heatCycle => {
      const date = new Date(heatCycle.created_at *1000);
      const hour = date.getHours();
      hourlyCounts[hour]++;
    });

    return Object.keys(hourlyCounts).map(hour => ({
      hour: parseInt(hour, 10),
      heatCycles: hourlyCounts[parseInt(hour, 10)],
    })).sort((a, b) => a.hour - b.hour);
  };

  const hourlyHeatCyclesData = monthlyStats ? processHourlyData(monthlyStats.heatCycles) : [];

  const axisStrokeColor = theme === 'dark' ? '#E5E7EB' : '#374151';
  const tooltipBgColor = theme === 'dark' ? '#1F2937' : '#FFFFFF';

  if (loading) {
    return <p className="text-slate-500">Lade Statistiken...</p>;
  }

  if (error) {
    return <p className="text-red-600 font-semibold">Fehler beim Laden der Statistiken: {error}</p>;
  }

  return (
    <div className="text-gray-900 dark:text-gray-100">
      <h1 className="text-2xl font-bold mb-4">Usage Statistics</h1>

      <div className="grid grid-cols-1 md:grid-cols-3 gap-4 mb-8">
        <div className="bg-white dark:bg-gray-800 p-4 rounded shadow">
          <h2 className="text-xl font-semibold mb-2">Daily Overview</h2>
          {dailyStats && (
            <p>Total Heat Cycles: {dailyStats.totalHeatCycles}, Total Duration: {dailyStats.totalDuration}s</p>
          )}
          <ResponsiveContainer width="100%" height={200}>
            <BarChart data={dailyStats ? [{ name: 'Today', heatCycles: dailyStats.totalHeatCycles, duration: dailyStats.totalDuration }] : []}>
              <CartesianGrid strokeDasharray="3 3" stroke={axisStrokeColor} />
              <XAxis dataKey="name" stroke={axisStrokeColor} />
              <YAxis stroke={axisStrokeColor} />
              <Tooltip contentStyle={{ backgroundColor: tooltipBgColor, color: axisStrokeColor }} />
              <Legend wrapperStyle={{ color: axisStrokeColor }} />
              <Bar dataKey="heatCycles" fill="#8884d8" name="Heat Cycles" />
              <Bar dataKey="duration" fill="#82ca9d" name="Duration (s)" />
            </BarChart>
          </ResponsiveContainer>
        </div>

        <div className="bg-white dark:bg-gray-800 p-4 rounded shadow">
          <h2 className="text-xl font-semibold mb-2">Weekly Overview</h2>
          {weeklyStats && (
            <p>Total Heat Cycles: {weeklyStats.totalHeatCycles}, Total Duration: {weeklyStats.totalDuration}s</p>
          )}
          <ResponsiveContainer width="100%" height={200}>
            <BarChart data={weeklyStats ? [{ name: 'This Week', heatCycles: weeklyStats.totalHeatCycles, duration: weeklyStats.totalDuration }] : []}>
              <CartesianGrid strokeDasharray="3 3" stroke={axisStrokeColor} />
              <XAxis dataKey="name" stroke={axisStrokeColor} />
              <YAxis stroke={axisStrokeColor} />
              <Tooltip contentStyle={{ backgroundColor: tooltipBgColor, color: axisStrokeColor }} />
              <Legend wrapperStyle={{ color: axisStrokeColor }} />
              <Bar dataKey="heatCycles" fill="#8884d8" name="Heat Cycles" />
              <Bar dataKey="duration" fill="#82ca9d" name="Duration (s)" />
            </BarChart>
          </ResponsiveContainer>
        </div>

        <div className="bg-white dark:bg-gray-800 p-4 rounded shadow">
          <h2 className="text-xl font-semibold mb-2">Monthly Overview</h2>
          {monthlyStats && (
            <p>Total Heat Cycles: {monthlyStats.totalHeatCycles}, Total Duration: {monthlyStats.totalDuration}s</p>
          )}
          <ResponsiveContainer width="100%" height={200}>
            <BarChart data={monthlyStats ? [{ name: 'This Month', heatCycles: monthlyStats.totalHeatCycles, duration: monthlyStats.totalDuration }] : []}>
              <CartesianGrid strokeDasharray="3 3" stroke={axisStrokeColor} />
              <XAxis dataKey="name" stroke={axisStrokeColor} />
              <YAxis stroke={axisStrokeColor} />
              <Tooltip contentStyle={{ backgroundColor: tooltipBgColor, color: axisStrokeColor }} />
              <Legend wrapperStyle={{ color: axisStrokeColor }} />
              <Bar dataKey="heatCycles" fill="#8884d8" name="Heat Cycles" />
              <Bar dataKey="duration" fill="#82ca9d" name="Duration (s)" />
            </BarChart>
          </ResponsiveContainer>
        </div>
      </div>

      <div className="bg-white dark:bg-gray-800 p-4 rounded shadow mb-8">
        <h2 className="text-xl font-semibold mb-2">Monthly Usage Heatmap</h2>
        <div className="grid grid-cols-7 gap-1 p-2">
          {monthlyHeatmapData.map((dataPoint) => {
            const intensity = maxHeatCyclesPerDay > 0 ? (dataPoint.count / maxHeatCyclesPerDay) * 100 : 0;
            const color = theme === 'dark' 
              ? `hsl(210, 70%, ${15 + (intensity * 0.4)}%)` // Scale from dark blue (15%) to lighter blue (55%)
              : `hsl(210, 70%, ${100 - (intensity * 0.6)}%)`; // Scale from white (100%) to blue (40%)
            return (
              <div
                key={dataPoint.day}
                className="w-full aspect-square flex items-center justify-center text-xs rounded"
                style={{ backgroundColor: color, color: intensity > 50 ? 'white' : 'black' }}
                title={`Day ${dataPoint.day}: ${dataPoint.count} heat cycles`}
              >
                {dataPoint.day} ({dataPoint.count})
              </div>
            );
          })}
        </div>
      </div>

      <div className="bg-white dark:bg-gray-800 p-4 rounded shadow mb-8">
        <h2 className="text-xl font-semibold mb-2">Heat Cycles per Hour (Monthly)</h2>
        <ResponsiveContainer width="100%" height={250}>
          <BarChart data={hourlyHeatCyclesData}>
            <CartesianGrid strokeDasharray="3 3" stroke={axisStrokeColor} />
            <XAxis dataKey="hour" label={{ value: 'Hour of Day', position: 'insideBottom', offset: -5, fill: axisStrokeColor }} stroke={axisStrokeColor} />
            <YAxis label={{ value: 'Heat Cycles', angle: -90, position: 'insideLeft', fill: axisStrokeColor }} stroke={axisStrokeColor} />
            <Tooltip contentStyle={{ backgroundColor: tooltipBgColor, color: axisStrokeColor }} />
            <Legend wrapperStyle={{ color: axisStrokeColor }} />
            <Bar dataKey="heatCycles" fill="#8884d8" name="Heat Cycles" />
          </BarChart>
        </ResponsiveContainer>
      </div>
    </div>
  );
};

export default UsagePage;

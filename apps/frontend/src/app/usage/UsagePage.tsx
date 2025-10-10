import React, { useState, useEffect } from 'react';
import { fetchStatistics } from '../../api';
import { BarChart, Bar, XAxis, YAxis, Tooltip, Legend, ResponsiveContainer, CartesianGrid } from 'recharts';

interface SessionData {
  created_at: string;
  duration: number;
}

interface StatisticsData {
  range: string;
  totalSessions: number;
  totalDuration: number;
  sessions: SessionData[];
}

const UsagePage: React.FC = () => {
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

  const processMonthlyHeatmapData = (sessions: SessionData[]) => {
    const heatmap: { [key: number]: number } = {}; // dayOfMonth: count
    const maxDay = new Date(new Date().getFullYear(), new Date().getMonth() + 1, 0).getDate();

    sessions.forEach(session => {
      const date = new Date(session.created_at);
      const dayOfMonth = date.getDate();
      heatmap[dayOfMonth] = (heatmap[dayOfMonth] || 0) + 1;
    });

    const formattedHeatmapData: { day: number; count: number }[] = [];
    for (let i = 1; i <= maxDay; i++) {
      formattedHeatmapData.push({ day: i, count: heatmap[i] || 0 });
    }
    return formattedHeatmapData;
  };

  const monthlyHeatmapData = monthlyStats ? processMonthlyHeatmapData(monthlyStats.sessions) : [];
  const maxSessionsPerDay = Math.max(...monthlyHeatmapData.map(data => data.count));

  const processHourlyData = (sessions: SessionData[]) => {
    const hourlyCounts: { [key: number]: number } = {};
    for (let i = 0; i < 24; i++) {
      hourlyCounts[i] = 0;
    }

    sessions.forEach(session => {
      const date = new Date(session.created_at);
      const hour = date.getHours();
      hourlyCounts[hour]++;
    });

    return Object.keys(hourlyCounts).map(hour => ({
      hour: parseInt(hour, 10),
      sessions: hourlyCounts[parseInt(hour, 10)],
    })).sort((a, b) => a.hour - b.hour);
  };

  const hourlySessionsData = monthlyStats ? processHourlyData(monthlyStats.sessions) : [];

  if (loading) {
    return <p className="text-slate-500">Lade Statistiken...</p>;
  }

  if (error) {
    return <p className="text-red-600 font-semibold">Fehler beim Laden der Statistiken: {error}</p>;
  }

  return (
    <div className="p-4">
      <h1 className="text-2xl font-bold mb-4">Usage Statistics</h1>

      <div className="grid grid-cols-1 md:grid-cols-3 gap-4 mb-8">
        <div className="bg-white p-4 rounded shadow">
          <h2 className="text-xl font-semibold mb-2">Daily Overview</h2>
          {dailyStats && (
            <p>Total Sessions: {dailyStats.totalSessions}, Total Duration: {dailyStats.totalDuration}s</p>
          )}
          <ResponsiveContainer width="100%" height={200}>
            <BarChart data={dailyStats ? [{ name: 'Today', sessions: dailyStats.totalSessions, duration: dailyStats.totalDuration }] : []}>
              <CartesianGrid strokeDasharray="3 3" />
              <XAxis dataKey="name" />
              <YAxis />
              <Tooltip />
              <Legend />
              <Bar dataKey="sessions" fill="#8884d8" name="Sessions" />
              <Bar dataKey="duration" fill="#82ca9d" name="Duration (s)" />
            </BarChart>
          </ResponsiveContainer>
        </div>

        <div className="bg-white p-4 rounded shadow">
          <h2 className="text-xl font-semibold mb-2">Weekly Overview</h2>
          {weeklyStats && (
            <p>Total Sessions: {weeklyStats.totalSessions}, Total Duration: {weeklyStats.totalDuration}s</p>
          )}
          <ResponsiveContainer width="100%" height={200}>
            <BarChart data={weeklyStats ? [{ name: 'This Week', sessions: weeklyStats.totalSessions, duration: weeklyStats.totalDuration }] : []}>
              <CartesianGrid strokeDasharray="3 3" />
              <XAxis dataKey="name" />
              <YAxis />
              <Tooltip />
              <Legend />
              <Bar dataKey="sessions" fill="#8884d8" name="Sessions" />
              <Bar dataKey="duration" fill="#82ca9d" name="Duration (s)" />
            </BarChart>
          </ResponsiveContainer>
        </div>

        <div className="bg-white p-4 rounded shadow">
          <h2 className="text-xl font-semibold mb-2">Monthly Overview</h2>
          {monthlyStats && (
            <p>Total Sessions: {monthlyStats.totalSessions}, Total Duration: {monthlyStats.totalDuration}s</p>
          )}
          <ResponsiveContainer width="100%" height={200}>
            <BarChart data={monthlyStats ? [{ name: 'This Month', sessions: monthlyStats.totalSessions, duration: monthlyStats.totalDuration }] : []}>
              <CartesianGrid strokeDasharray="3 3" />
              <XAxis dataKey="name" />
              <YAxis />
              <Tooltip />
              <Legend />
              <Bar dataKey="sessions" fill="#8884d8" name="Sessions" />
              <Bar dataKey="duration" fill="#82ca9d" name="Duration (s)" />
            </BarChart>
          </ResponsiveContainer>
        </div>
      </div>

      <div className="bg-white p-4 rounded shadow mb-8">
        <h2 className="text-xl font-semibold mb-2">Monthly Usage Heatmap</h2>
        <div className="grid grid-cols-7 gap-1 p-2">
          {monthlyHeatmapData.map((dataPoint) => {
            const intensity = maxSessionsPerDay > 0 ? (dataPoint.count / maxSessionsPerDay) * 100 : 0;
            const color = `hsl(210, 70%, ${100 - (intensity * 0.6)}%)`; // Scale lightness from 100% (white) to 40% (dark blue)
            return (
              <div
                key={dataPoint.day}
                className="w-full aspect-square flex items-center justify-center text-xs rounded"
                style={{ backgroundColor: color }}
                title={`Day ${dataPoint.day}: ${dataPoint.count} sessions`}
              >
                {dataPoint.day} ({dataPoint.count})
              </div>
            );
          })}
        </div>
      </div>

      <div className="bg-white p-4 rounded shadow mb-8">
        <h2 className="text-xl font-semibold mb-2">Sessions per Hour (Monthly)</h2>
        <ResponsiveContainer width="100%" height={250}>
          <BarChart data={hourlySessionsData}>
            <CartesianGrid strokeDasharray="3 3" />
            <XAxis dataKey="hour" label={{ value: 'Hour of Day', position: 'insideBottom', offset: -5 }} />
            <YAxis label={{ value: 'Sessions', angle: -90, position: 'insideLeft' }} />
            <Tooltip />
            <Legend />
            <Bar dataKey="sessions" fill="#8884d8" name="Sessions" />
          </BarChart>
        </ResponsiveContainer>
      </div>
    </div>
  );
};

export default UsagePage;

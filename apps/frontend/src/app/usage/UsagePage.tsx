import React from "react";
import {
  BarChart,
  Bar,
  XAxis,
  YAxis,
  Tooltip,
  Legend,
  ResponsiveContainer,
  CartesianGrid,
  LineChart,
  Line,
  PieChart,
  Pie,
  Cell,
  Area,
  AreaChart,
} from "recharts";
import { useTheme } from "../hooks/useTheme";
import { useStatistics } from "./useStatistics";
import type { StatisticsApiResponse } from "@heizbox/types";

type StatisticsHeatCycle = StatisticsApiResponse["heatCycles"];

const CONSUMPTION_PER_CYCLE = 0.05; // Gramm pro Cycle

const processMonthlyHeatmapData = (heatCycles: StatisticsHeatCycle) => {
  const heatmap: { [key: number]: number } = {};
  const maxDay = new Date(
    new Date().getFullYear(),
    new Date().getMonth() + 1,
    0,
  ).getDate();
  heatCycles.forEach((heatCycle) => {
    const date = new Date(heatCycle.created_at * 1000);
    const dayOfMonth = date.getDate();
    heatmap[dayOfMonth] = (heatmap[dayOfMonth] || 0) + 1;
  });

  const formattedHeatmapData: { day: number; count: number }[] = [];
  for (let i = 1; i <= maxDay; i++) {
    formattedHeatmapData.push({ day: i, count: heatmap[i] || 0 });
  }
  return formattedHeatmapData;
};

const processHourlyData = (heatCycles: StatisticsHeatCycle) => {
  const hourlyCounts: { [key: number]: number } = {};
  for (let i = 0; i < 24; i++) {
    hourlyCounts[i] = 0;
  }

  heatCycles.forEach((heatCycle) => {
    const date = new Date(heatCycle.created_at * 1000);
    const hour = date.getHours();
    hourlyCounts[hour]++;
  });

  return Object.keys(hourlyCounts)
    .map((hour) => ({
      hour: parseInt(hour, 10),
      heatCycles: hourlyCounts[parseInt(hour, 10)],
    }))
    .sort((a, b) => a.hour - b.hour);
};

const processWeekdayData = (heatCycles: StatisticsHeatCycle) => {
  const weekdayNames = ["So", "Mo", "Di", "Mi", "Do", "Fr", "Sa"];
  const weekdayCounts: { [key: number]: number } = {};
  for (let i = 0; i < 7; i++) {
    weekdayCounts[i] = 0;
  }

  heatCycles.forEach((heatCycle) => {
    const date = new Date(heatCycle.created_at * 1000);
    const weekday = date.getDay();
    weekdayCounts[weekday]++;
  });

  return Object.keys(weekdayCounts).map((day) => ({
    weekday: weekdayNames[parseInt(day, 10)],
    count: weekdayCounts[parseInt(day, 10)],
  }));
};

const processDurationDistribution = (heatCycles: StatisticsHeatCycle) => {
  const ranges = [
    { label: "0-10s", min: 0, max: 10, count: 0 },
    { label: "10-30s", min: 10, max: 30, count: 0 },
    { label: "30s-1m", min: 30, max: 60, count: 0 },
    { label: "1-2m", min: 60, max: 120, count: 0 },
    { label: "2m+", min: 120, max: Infinity, count: 0 },
  ];

  heatCycles.forEach((cycle) => {
    const duration = cycle.duration || 0;
    const range = ranges.find((r) => duration >= r.min && duration < r.max);
    if (range) range.count++;
  });

  return ranges.map((r) => ({ name: r.label, value: r.count }));
};

const processDailyConsumption = (heatCycles: StatisticsHeatCycle) => {
  const dailyData: { [key: string]: { cycles: number; consumption: number } } =
    {};

  heatCycles.forEach((cap) => {
    const date = new Date(cap.created_at * 1000);
    const dateKey = `${date.getDate()}.${date.getMonth() + 1}`;

    if (!dailyData[dateKey]) {
      dailyData[dateKey] = { cycles: 0, consumption: 0 };
    }

    dailyData[dateKey].cycles += 1;
    if (cap.cycle === 1) {
      dailyData[dateKey].consumption += CONSUMPTION_PER_CYCLE;
    }
  });

  return Object.keys(dailyData)
    .map((date) => ({
      date,
      cycles: dailyData[date].cycles,
      consumption: dailyData[date].consumption,
    }))
    .sort((a, b) => {
      const [dayA, monthA] = a.date.split(".").map(Number);
      const [dayB, monthB] = b.date.split(".").map(Number);
      return monthA === monthB ? dayA - dayB : monthA - monthB;
    })
    .slice(-30);
};

const processCycleDistribution = (heatCycles: StatisticsHeatCycle) => {
  const cycleCounts: { [key: number]: number } = {};

  heatCycles.forEach((heatCycle) => {
    const cycles = heatCycle.cycle || 0;
    cycleCounts[cycles] = (cycleCounts[cycles] || 0) + 1;
  });

  return Object.keys(cycleCounts)
    .map((cycles) => ({
      cycles: parseInt(cycles, 10),
      count: cycleCounts[parseInt(cycles, 10)],
    }))
    .sort((a, b) => a.cycles - b.cycles);
};

const processEfficiencyData = (heatCycles: StatisticsHeatCycle) => {
  return heatCycles
    .map((c, idx) => ({
      index: idx + 1,
      duration: c.duration || 0,
      cyclesPerSecond: c.duration > 0 ? c.cycle / c.duration : 0,
      efficiency: c.duration > 0 ? (c.cycle * 60) / c.duration : 0, // Cycles pro Minute
    }))
    .slice(-30);
};

const UsagePage: React.FC = () => {
  const { theme } = useTheme();
  const { dailyStats, weeklyStats, monthlyStats, isLoading, isError, error } =
    useStatistics();

  const monthlyHeatmapData = monthlyStats
    ? processMonthlyHeatmapData(monthlyStats.heatCycles)
    : [];
  const maxHeatCyclesPerDay = Math.max(
    ...monthlyHeatmapData.map((data) => data.count),
    0,
  );

  const hourlyHeatCyclesData = monthlyStats
    ? processHourlyData(monthlyStats.heatCycles)
    : [];

  const weekdayData = monthlyStats
    ? processWeekdayData(monthlyStats.heatCycles)
    : [];

  const durationDistribution = monthlyStats
    ? processDurationDistribution(monthlyStats.heatCycles)
    : [];

  const dailyConsumptionData = monthlyStats
    ? processDailyConsumption(monthlyStats.heatCycles)
    : [];

  const cycleDistribution = monthlyStats
    ? processCycleDistribution(monthlyStats.heatCycles)
    : [];

  const efficiencyData = monthlyStats
    ? processEfficiencyData(monthlyStats.heatCycles)
    : [];

  const axisStrokeColor = "var(--gray-11)";
  const tooltipBgColor = "var(--color-panel-solid)";
  const gridColor = "var(--gray-6)";
  const COLORS = [
    "var(--blue-9)",
    "var(--green-9)",
    "var(--amber-9)",
    "var(--orange-9)",
    "var(--purple-9)",
  ];

  // Berechne erweiterte Statistiken
  const calculateExtendedStats = () => {
    if (!monthlyStats) return null;

    const cycles = monthlyStats.heatCycles;
    const totalSessions = cycles.length;
    const totalCycles = cycles.reduce((sum, c) => sum + (c.cycle || 0), 0);
    const totalDuration = cycles.reduce((sum, c) => sum + (c.duration || 0), 0);
    const totalConsumption = totalCycles * CONSUMPTION_PER_CYCLE;

    const avgDuration = totalSessions > 0 ? totalDuration / totalSessions : 0;
    const avgCyclesPerSession =
      totalSessions > 0 ? totalCycles / totalSessions : 0;
    const avgConsumptionPerSession =
      totalSessions > 0 ? totalConsumption / totalSessions : 0;
    const avgConsumptionPerDay =
      dailyConsumptionData.length > 0
        ? dailyConsumptionData.reduce((sum, d) => sum + d.consumption, 0) /
          dailyConsumptionData.length
        : 0;

    const maxDuration = Math.max(...cycles.map((c) => c.duration || 0), 0);
    const minDuration =
      totalSessions > 0
        ? Math.min(...cycles.map((c) => c.duration || Infinity))
        : 0;

    return {
      totalSessions,
      totalCycles,
      totalDuration,
      totalConsumption,
      avgDuration,
      avgCyclesPerSession,
      avgConsumptionPerSession,
      avgConsumptionPerDay,
      maxDuration,
      minDuration,
    };
  };

  const stats = calculateExtendedStats();

  if (isLoading) {
    return (
      <div className="flex items-center justify-center min-h-screen">
        <p className="text-slate-500 text-lg">Lade Statistiken...</p>
      </div>
    );
  }

  if (isError) {
    return (
      <div className="flex items-center justify-center min-h-screen">
        <p className="text-red-600 font-semibold">
          Fehler beim Laden der Statistiken: {error?.message}
        </p>
      </div>
    );
  }

  return (
    <div className="text-gray-900 dark:text-gray-100 p-6 max-w-7xl mx-auto">
      <h1 className="text-3xl font-bold mb-6">
        Umfassende Nutzungsstatistiken
      </h1>

      {/* Übersichtskarten */}
      <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-4 mb-8">
        <div className="bg-gradient-to-br from-blue-500 to-blue-600 p-6 rounded-lg shadow-lg text-white">
          <h3 className="text-sm font-medium opacity-90">Gesamt Sessions</h3>
          <p className="text-3xl font-bold mt-2">{stats?.totalSessions || 0}</p>
          <p className="text-xs mt-1 opacity-75">
            {stats?.totalCycles || 0} Cycles
          </p>
        </div>

        <div className="bg-gradient-to-br from-green-500 to-green-600 p-6 rounded-lg shadow-lg text-white">
          <h3 className="text-sm font-medium opacity-90">Durchschn. Dauer</h3>
          <p className="text-3xl font-bold mt-2">
            {stats ? stats.avgDuration.toFixed(1) : 0}s
          </p>
          <p className="text-xs mt-1 opacity-75">
            Ø {stats ? stats.avgCyclesPerSession.toFixed(1) : 0} Cycles/Session
          </p>
        </div>

        <div className="bg-gradient-to-br from-orange-500 to-orange-600 p-6 rounded-lg shadow-lg text-white">
          <h3 className="text-sm font-medium opacity-90">Gesamt Verbrauch</h3>
          <p className="text-3xl font-bold mt-2">
            {stats ? stats.totalConsumption.toFixed(2) : 0}g
          </p>
          <p className="text-xs mt-1 opacity-75">
            Ø {stats ? stats.avgConsumptionPerDay.toFixed(2) : 0}g/Tag
          </p>
        </div>

        <div className="bg-gradient-to-br from-purple-500 to-purple-600 p-6 rounded-lg shadow-lg text-white">
          <h3 className="text-sm font-medium opacity-90">
            Ø Verbrauch/Session
          </h3>
          <p className="text-3xl font-bold mt-2">
            {stats ? stats.avgConsumptionPerSession.toFixed(3) : 0}g
          </p>
          <p className="text-xs mt-1 opacity-75">
            Bei {CONSUMPTION_PER_CYCLE}g/Cycle
          </p>
        </div>
      </div>

      {/* Verbrauchsanalyse */}
      <div className="grid grid-cols-1 lg:grid-cols-2 gap-6 mb-6">
        {/* Täglicher Verbrauch */}
        <div className="bg-slate-800 p-6 rounded-lg shadow">
          <h2 className="text-xl font-semibold mb-4 text-slate-200">
            Täglicher Verbrauch (Letzte 30 Tage)
          </h2>
          <ResponsiveContainer width="100%" height={300}>
            <AreaChart data={dailyConsumptionData}>
              <CartesianGrid strokeDasharray="3 3" stroke={axisStrokeColor} />
              <XAxis dataKey="date" stroke={axisStrokeColor} />
              <YAxis stroke={axisStrokeColor} />
              <Tooltip
                contentStyle={{
                  backgroundColor: tooltipBgColor,
                  color: axisStrokeColor,
                }}
                formatter={(value: number, name: string) => [
                  `${value.toFixed(2)}g`,
                ]}
              />
              <Legend />
              <Area
                type="monotone"
                dataKey="consumption"
                stroke="#82ca9d"
                fill="#82ca9d"
                name="Verbrauch (g)"
              />
            </AreaChart>
          </ResponsiveContainer>
        </div>

        {/* Cycle-Verteilung */}
        <div className="bg-slate-800 p-6 rounded-lg shadow">
          <h2 className="text-xl font-semibold mb-4 text-slate-200">
            Click-Verteilung pro Session
          </h2>
          <ResponsiveContainer width="100%" height={300}>
            <BarChart data={cycleDistribution}>
              <CartesianGrid strokeDasharray="3 3" stroke={axisStrokeColor} />
              <XAxis
                dataKey="cycles"
                stroke={axisStrokeColor}
                label={{
                  value: "Heat Cycle",
                  position: "insideBottom",
                  offset: -5,
                }}
              />
              <YAxis
                stroke={axisStrokeColor}
                label={{
                  value: "Anzahl Clicks",
                  angle: -90,
                  position: "insideLeft",
                }}
              />
              <Tooltip
                contentStyle={{
                  backgroundColor: tooltipBgColor,
                  color: axisStrokeColor,
                }}
              />
              <Bar dataKey="count" fill="#8884d8" name="Sessions" />
            </BarChart>
          </ResponsiveContainer>
        </div>
      </div>

      {/* Zeitbasierte Analysen */}
      <div className="grid grid-cols-1 lg:grid-cols-2 gap-6 mb-6">
        {/* Wochentags-Verteilung */}
        <div className="bg-slate-800 p-6 rounded-lg shadow">
          <h2 className="text-xl font-semibold mb-4 text-slate-200">
            Nutzung nach Wochentag
          </h2>
          <ResponsiveContainer width="100%" height={300}>
            <BarChart data={weekdayData}>
              <CartesianGrid strokeDasharray="3 3" stroke={axisStrokeColor} />
              <XAxis dataKey="weekday" stroke={axisStrokeColor} />
              <YAxis stroke={axisStrokeColor} />
              <Tooltip
                contentStyle={{
                  backgroundColor: tooltipBgColor,
                  color: axisStrokeColor,
                }}
              />
              <Bar dataKey="count" fill="#8884d8" name="Sessions" />
            </BarChart>
          </ResponsiveContainer>
        </div>

        {/* Stündliche Verteilung */}
        <div className="bg-slate-800 p-6 rounded-lg shadow">
          <h2 className="text-xl font-semibold mb-4 text-slate-200">
            Nutzung nach Tageszeit
          </h2>
          <ResponsiveContainer width="100%" height={300}>
            <AreaChart data={hourlyHeatCyclesData}>
              <CartesianGrid strokeDasharray="3 3" stroke={axisStrokeColor} />
              <XAxis dataKey="hour" stroke={axisStrokeColor} />
              <YAxis stroke={axisStrokeColor} />
              <Tooltip
                contentStyle={{
                  backgroundColor: tooltipBgColor,
                  color: axisStrokeColor,
                }}
              />
              <Area
                type="monotone"
                dataKey="heatCycles"
                stroke="#8884d8"
                fill="#8884d8"
                name="Sessions"
              />
            </AreaChart>
          </ResponsiveContainer>
        </div>
      </div>

      {/* Monatliche Heatmap */}
      <div className="bg-slate-800 p-6 rounded-lg shadow mb-6">
        <h2 className="text-xl font-semibold mb-4 text-slate-200">
          Monatliche Nutzungs-Heatmap
        </h2>
        <div className="grid grid-cols-7 gap-2 p-2">
          {monthlyHeatmapData.map((dataPoint) => {
            const intensity =
              maxHeatCyclesPerDay > 0
                ? (dataPoint.count / maxHeatCyclesPerDay) * 100
                : 0;
            const color =
              theme === "dark"
                ? `hsl(210, 70%, ${15 + intensity * 0.4}%)`
                : `hsl(210, 70%, ${100 - intensity * 0.6}%)`;
            return (
              <div
                key={dataPoint.day}
                className="w-full aspect-square flex flex-col items-center justify-center text-xs rounded hover:scale-110 transition-transform cursor-pointer"
                style={{
                  backgroundColor: color,
                  color: intensity > 50 && theme === "dark" ? "white" : "black",
                }}
                title={`Tag ${dataPoint.day}: ${dataPoint.count} Sessions`}
              >
                <div className="font-bold">{dataPoint.day}</div>
                <div className="text-[10px]">{dataPoint.count}</div>
              </div>
            );
          })}
        </div>
      </div>

      {/* Dauer & Effizienz */}
      <div className="grid grid-cols-1 lg:grid-cols-2 gap-6 mb-6">
        {/* Dauer-Verteilung */}
        <div className="bg-slate-800 p-6 rounded-lg shadow">
          <h2 className="text-xl font-semibold mb-4 text-slate-200">
            Dauer-Verteilung
          </h2>
          <ResponsiveContainer width="100%" height={300}>
            <PieChart>
              <Pie
                data={durationDistribution}
                cx="50%"
                cy="50%"
                labelLine={false}
                label={(entry) => `${entry.name}: ${entry.value}`}
                outerRadius={100}
                fill="#8884d8"
                dataKey="value"
              >
                {durationDistribution.map((entry, index) => (
                  <Cell
                    key={`cell-${index}`}
                    fill={COLORS[index % COLORS.length]}
                  />
                ))}
              </Pie>
              <Tooltip />
            </PieChart>
          </ResponsiveContainer>
        </div>

        {/* Effizienz */}
        <div className="bg-slate-800 p-6 rounded-lg shadow">
          <h2 className="text-xl font-semibold mb-4 text-slate-200">
            Effizienz (Cycles/Min - Letzte 30)
          </h2>
          <ResponsiveContainer width="100%" height={300}>
            <LineChart data={efficiencyData}>
              <CartesianGrid strokeDasharray="3 3" stroke={axisStrokeColor} />
              <XAxis dataKey="index" stroke={axisStrokeColor} />
              <YAxis stroke={axisStrokeColor} />
              <Tooltip
                contentStyle={{
                  backgroundColor: tooltipBgColor,
                  color: axisStrokeColor,
                }}
                formatter={(value: number) => value.toFixed(2)}
              />
              <Legend />
              <Line
                type="monotone"
                dataKey="efficiency"
                stroke="#82ca9d"
                name="Cycles/Min"
                strokeWidth={2}
              />
            </LineChart>
          </ResponsiveContainer>
        </div>
      </div>
    </div>
  );
};

export default UsagePage;

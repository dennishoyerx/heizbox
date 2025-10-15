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
} from "recharts";
import { useTheme } from "../hooks/useTheme";
import { useStatistics } from "./useStatistics";
import type { StatisticsApiResponse } from "@heizbox/types";

// Der Typ für die HeatCycle-Daten in den Statistiken, direkt aus dem API-Typ abgeleitet.
type StatisticsHeatCycle = StatisticsApiResponse["heatCycles"];

const processMonthlyHeatmapData = (heatCycles: StatisticsHeatCycle) => {
  const heatmap: { [key: number]: number } = {}; // dayOfMonth: count
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

  const axisStrokeColor = theme === "dark" ? "#E5E7EB" : "#374151";
  const tooltipBgColor = theme === "dark" ? "#1F2937" : "#FFFFFF";

  if (isLoading) {
    return <p className="text-slate-500">Lade Statistiken...</p>;
  }

  if (isError) {
    return (
      <p className="text-red-600 font-semibold">
        Fehler beim Laden der Statistiken: {error?.message}
      </p>
    );
  }

  return (
    <div className="text-gray-900 dark:text-gray-100">
      <h1 className="text-2xl font-bold mb-4">Nutzungsstatistiken</h1>

      <div className="grid grid-cols-1 md:grid-cols-3 gap-4 mb-8">
        {/* Overview Cards */}
      </div>

      <div className="bg-white dark:bg-gray-800 p-4 rounded shadow mb-8">
        <h2 className="text-xl font-semibold mb-2">
          Monatliche Nutzungs-Heatmap
        </h2>
        <div className="grid grid-cols-7 gap-1 p-2">
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
                className="w-full aspect-square flex items-center justify-center text-xs rounded"
                style={{
                  backgroundColor: color,
                  color: intensity > 50 && theme === "dark" ? "white" : "black",
                }}
                title={`Tag ${dataPoint.day}: ${dataPoint.count} Zyklen`}
              >
                {dataPoint.day} ({dataPoint.count})
              </div>
            );
          })}
        </div>
      </div>

      <div className="bg-white dark:bg-gray-800 p-4 rounded shadow mb-8">
        <h2 className="text-xl font-semibold mb-2">
          Heizzyklen pro Stunde (Monat)
        </h2>
        <ResponsiveContainer width="100%" height={250}>
          <BarChart data={hourlyHeatCyclesData}>
            <CartesianGrid strokeDasharray="3 3" stroke={axisStrokeColor} />
            <XAxis
              dataKey="hour"
              label={{
                value: "Stunde des Tages",
                position: "insideBottom",
                offset: -5,
                fill: axisStrokeColor,
              }}
              stroke={axisStrokeColor}
            />
            <YAxis
              label={{
                value: "Heizzyklen",
                angle: -90,
                position: "insideLeft",
                fill: axisStrokeColor,
              }}
              stroke={axisStrokeColor}
            />
            <Tooltip
              contentStyle={{
                backgroundColor: tooltipBgColor,
                color: axisStrokeColor,
              }}
            />
            <Legend wrapperStyle={{ color: axisStrokeColor }} />
            <Bar dataKey="heatCycles" fill="#8884d8" name="Heizzyklen" />
          </BarChart>
        </ResponsiveContainer>
      </div>
    </div>
  );
};

export default UsagePage;

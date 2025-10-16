import { Hono } from "hono";
import { HeatCycleService } from '../services/heatCycleService';
import type { StatisticsApiResponse, HeatCycleRow } from "@heizbox/types";

const statistics = new Hono<{ Bindings: Env }>();

statistics.get("/", async (c) => {
  const range = c.req.query("range") || "month"; // Default to month

  let startDate: Date;
  const now = new Date();

  switch (range) {
    case "day":
      startDate = new Date(now.getFullYear(), now.getMonth(), now.getDate());
      break;
    case "week":
      startDate = new Date(
        now.getFullYear(),
        now.getMonth(),
        now.getDate() - now.getDay(),
      );
      break;
    case "month":
      startDate = new Date(now.getFullYear(), now.getMonth(), 1);
      break;
    default:
      return c.json(
        { error: "Invalid range specified. Use day, week, or month." },
        400,
      );
  }

  const start = Math.floor(startDate.getTime() / 1000);
  const end = Math.floor(now.getTime() / 1000);

  console.log(`Statistics route called with range: ${range}`);
  console.log(
    `Generated startDate: ${startDate.toISOString()} (${startDate.getTime()})`,
  );

  try {
    const service = new HeatCycleService(c.env.db);
    const results = await service.getHeatCyclesInRange(start, end);
    console.log("Statistics query results:", results);

    // Basic aggregation for now, more sophisticated aggregation can be added later
    const totalHeatCycles = results.length;
    const totalDuration = results.reduce(
      (sum: number, heatCycle: HeatCycleRow) => {
        const durationValue = Number(heatCycle.duration);
        return sum + (isNaN(durationValue) ? 0 : durationValue);
      },
      0,
    );

    return c.json<StatisticsApiResponse>({
      range,
      totalHeatCycles,
      totalDuration,
      heatCycles: results,
    });
  } catch (error) {
    console.error("Error fetching statistics:", error);
    return c.json({ error: "Failed to fetch statistics" }, 500);
  }
});

export default statistics;
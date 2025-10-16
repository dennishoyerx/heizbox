#!/usr/bin/env node
import readline from "node:readline";
import { stdin as input, stdout as output } from "node:process";
import chalk from "chalk";

const rl = readline.createInterface({ input, output });

function colorLog(line) {
  if (/heatCycleCompleted/.test(line)) return chalk.green(line);
  if (/statusUpdate/.test(line)) return chalk.yellow(line);
  if (/heartbeat/.test(line)) return chalk.blue(line);
  if (/Alarm/.test(line)) return chalk.red.bold(line);
  return line;
}

rl.on("line", (line) => {
  // Optional: Datum/Zeit extrahieren und hervorheben
  const dateMatch = line.match(/@ (\d{2}\.\d{2}\.\d{4}, \d{2}:\d{2}:\d{2})/);

  if (dateMatch) {
    line = line.replace(dateMatch[0], chalk.magenta(dateMatch[1]));
  }

  console.log(colorLog(line));
});

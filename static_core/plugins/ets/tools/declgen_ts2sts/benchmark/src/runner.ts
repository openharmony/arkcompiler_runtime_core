/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import * as fs from 'fs';
import * as path from 'path';
import { runCase } from './Benchmark';
import { getRegistry } from './registry';
import { printResult, printIncrementalSummary, writeJsonReport } from './Reporter';
import { BenchOptions, BenchResult } from './types';

interface CliArgs {
  filter?: RegExp;
  jsonPath?: string;
  options: BenchOptions;
}

function parseArgs(argv: readonly string[]): CliArgs {
  const out: CliArgs = { options: { warmup: 2, iterations: 10 } };
  for (let i = 0; i < argv.length; i++) {
    const arg = argv[i];
    const next = argv[i + 1];
    switch (arg) {
      case '--filter':
        out.filter = new RegExp(next, 'i');
        i++;
        break;
      case '--iterations':
        out.options.iterations = Number(next);
        i++;
        break;
      case '--warmup':
        out.options.warmup = Number(next);
        i++;
        break;
      case '--json':
        out.jsonPath = next;
        i++;
        break;
      case '--help':
      case '-h':
        process.stdout.write('Usage: bench [--filter <pattern>] [--iterations N] [--warmup N] [--json <path>]\n');
        process.exit(0);
        break;
      default:
        process.stderr.write(`Unknown argument: ${arg}\n`);
        process.exit(1);
    }
  }
  return out;
}

function loadScenarios(): void {
  const dir = path.join(__dirname, '..', 'scenarios');
  if (!fs.existsSync(dir)) {
    return;
  }
  for (const entry of fs.readdirSync(dir)) {
    if (entry.endsWith('.bench.js')) {
      require(path.join(dir, entry));
    }
  }
}

async function main(): Promise<void> {
  const args = parseArgs(process.argv.slice(2));
  loadScenarios();

  const cases = getRegistry().filter((c) => !args.filter || args.filter.test(c.name));
  if (cases.length === 0) {
    process.stderr.write('No benchmark cases matched.\n');
    process.exit(1);
  }

  process.stdout.write(
    `Running ${cases.length} case(s)  warmup=${args.options.warmup}  iterations=${args.options.iterations}\n`
  );

  const results: BenchResult[] = [];
  for (const c of cases) {
    const res = await runCase(c, args.options);
    printResult(res);
    results.push(res);
  }

  printIncrementalSummary(results);

  if (args.jsonPath) {
    writeJsonReport(args.jsonPath, results);
  }
}

main().catch((err) => {
  process.stderr.write(`Benchmark failed: ${err instanceof Error ? (err.stack ?? err.message) : String(err)}\n`);
  process.exit(1);
});

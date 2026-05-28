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
import { BenchResult } from './types';

function fmt(ms: number): string {
  return ms.toFixed(3);
}

export function printResult(result: BenchResult): void {
  const o = result.overall;
  process.stdout.write(`\n▶ ${result.name}\n`);
  process.stdout.write(
    `  samples=${o.samples}  mean=${fmt(o.meanMs)}ms  median=${fmt(o.medianMs)}ms  ` +
      `p95=${fmt(o.p95Ms)}ms  min=${fmt(o.minMs)}ms  max=${fmt(o.maxMs)}ms  ` +
      `stddev=${fmt(o.stddevMs)}ms\n`
  );
  if (result.stages.length > 0) {
    process.stdout.write('  Top stages by mean duration:\n');
    const top = result.stages.slice(0, 10);
    for (const s of top) {
      process.stdout.write(`    ${fmt(s.meanMs).padStart(10)}ms  ${s.name}\n`);
    }
  }
}

export function writeJsonReport(filePath: string, results: BenchResult[]): void {
  const abs = path.resolve(filePath);
  fs.mkdirSync(path.dirname(abs), { recursive: true });
  const payload = {
    timestamp: new Date().toISOString(),
    node: process.version,
    platform: `${process.platform}-${process.arch}`,
    results
  };
  fs.writeFileSync(abs, JSON.stringify(payload, null, 2));
  process.stdout.write(`\nJSON report written to ${abs}\n`);
}

/**
 * Pair `[full]` and `[incremental]` variants by their shared label and print
 * a wall-clock speedup table. Cases without a matching counterpart are
 * skipped silently.
 */
export function printIncrementalSummary(results: readonly BenchResult[]): void {
  const VARIANT_RE = /^(declgen\/[^\s]+)\s+\[(full|incremental)\]/;
  const pairs = new Map<string, { full?: BenchResult; incremental?: BenchResult }>();
  for (const r of results) {
    const m = VARIANT_RE.exec(r.name);
    if (!m) {
      continue;
    }
    const key = m[1];
    const slot = pairs.get(key) ?? {};
    if (m[2] === 'full') {
      slot.full = r;
    } else {
      slot.incremental = r;
    }
    pairs.set(key, slot);
  }

  const rows = [...pairs.entries()].filter(([, v]) => v.full && v.incremental);
  if (rows.length === 0) {
    return;
  }

  process.stdout.write('\n── Incremental vs full (mean wall-clock) ──\n');
  for (const [label, { full, incremental }] of rows) {
    const fMean = full!.overall.meanMs;
    const iMean = incremental!.overall.meanMs;
    const speedup = fMean / iMean;
    const saved = fMean - iMean;
    process.stdout.write(
      `  ${label.padEnd(40)} full=${fmt(fMean)}ms  inc=${fmt(iMean)}ms  ` +
        `saved=${fmt(saved)}ms  speedup=${speedup.toFixed(2)}x\n`
    );
  }
}

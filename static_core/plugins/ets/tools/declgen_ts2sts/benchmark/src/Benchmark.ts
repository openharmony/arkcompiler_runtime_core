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

import { performance } from 'perf_hooks';
import { StageTiming } from '../../src/stages/Stage';
import { BenchCase, BenchOptions, BenchResult, BenchStats, StageStat } from './types';

function computeStats(samples: number[]): BenchStats {
  const sorted = [...samples].sort((a, b) => a - b);
  const n = sorted.length;
  const mean = sorted.reduce((a, b) => a + b, 0) / n;
  const variance = sorted.reduce((acc, x) => acc + (x - mean) ** 2, 0) / n;
  const p95Index = Math.min(n - 1, Math.floor(n * 0.95));
  const medianIndex = Math.floor(n / 2);
  return {
    samples: n,
    minMs: sorted[0],
    maxMs: sorted[n - 1],
    meanMs: mean,
    medianMs: n % 2 === 0 ? (sorted[medianIndex - 1] + sorted[medianIndex]) / 2 : sorted[medianIndex],
    p95Ms: sorted[p95Index],
    stddevMs: Math.sqrt(variance)
  };
}

function aggregateStages(perRun: StageTiming[][]): StageStat[] {
  const acc = new Map<string, { total: number; samples: number }>();
  for (const run of perRun) {
    for (const t of run) {
      const cur = acc.get(t.path) ?? { total: 0, samples: 0 };
      cur.total += t.durationMs;
      cur.samples += 1;
      acc.set(t.path, cur);
    }
  }
  const out: StageStat[] = [];
  for (const [name, { total, samples }] of acc) {
    out.push({ name, meanMs: total / samples, totalMs: total, samples });
  }
  out.sort((a, b) => b.meanMs - a.meanMs);
  return out;
}

export async function runCase(bench: BenchCase, opts: BenchOptions): Promise<BenchResult> {
  if (bench.setup) {
    await bench.setup();
  }
  try {
    for (let i = 0; i < opts.warmup; i++) {
      await bench.fn();
    }

    const durations: number[] = [];
    const stageRuns: StageTiming[][] = [];

    for (let i = 0; i < opts.iterations; i++) {
      const start = performance.now();
      const result = await bench.fn();
      durations.push(performance.now() - start);
      if (Array.isArray(result)) {
        stageRuns.push(result);
      }
    }

    return {
      name: bench.name,
      overall: computeStats(durations),
      stages: aggregateStages(stageRuns)
    };
  } finally {
    if (bench.teardown) {
      await bench.teardown();
    }
  }
}

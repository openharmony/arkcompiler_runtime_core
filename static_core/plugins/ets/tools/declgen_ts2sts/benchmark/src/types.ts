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

import { StageTiming } from '../../src/stages/Stage';

export interface BenchOptions {
  warmup: number;
  iterations: number;
}

export type BenchFn = () => StageTiming[] | void | Promise<StageTiming[] | void>;
export type BenchHook = () => void | Promise<void>;

export interface BenchCase {
  name: string;
  fn: BenchFn;
  /** Runs once before warmup (e.g. prime an on-disk cache). */
  setup?: BenchHook;
  /** Runs once after the last measured iteration. */
  teardown?: BenchHook;
}

export interface BenchStats {
  samples: number;
  minMs: number;
  maxMs: number;
  meanMs: number;
  medianMs: number;
  p95Ms: number;
  stddevMs: number;
}

export interface StageStat {
  name: string;
  meanMs: number;
  totalMs: number;
  samples: number;
}

export interface BenchResult {
  name: string;
  overall: BenchStats;
  stages: StageStat[];
}

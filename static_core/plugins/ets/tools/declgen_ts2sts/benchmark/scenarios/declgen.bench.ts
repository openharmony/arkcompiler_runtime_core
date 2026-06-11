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
import { Declgen } from '../../src/Declgen';
import { defineBench } from '../src/registry';

// Resolve against the package root (npm scripts run from there) so this works
// whether we're executing from `src/` or compiled output under `build/`.
const FIXTURE_ROOT = path.join(process.cwd(), 'benchmark', 'fixtures');
const RESULTS_ROOT = path.join(process.cwd(), 'benchmark', 'results');

function collectTsFiles(dir: string): string[] {
  if (!fs.existsSync(dir)) {
    return [];
  }
  const out: string[] = [];
  for (const entry of fs.readdirSync(dir, { withFileTypes: true })) {
    const full = path.join(dir, entry.name);
    if (entry.isDirectory()) {
      out.push(...collectTsFiles(full));
    } else if (entry.isFile() && entry.name.endsWith('.ts') && !entry.name.endsWith('.d.ts')) {
      out.push(full);
    }
  }
  return out;
}

function rimraf(dir: string): void {
  if (fs.existsSync(dir)) {
    fs.rmSync(dir, { recursive: true, force: true });
  }
}

/**
 * Register two variants per fixture so the report can be diffed side-by-side:
 *  - `<name> [full]`        — no cache, every run goes through the whole pipeline.
 *  - `<name> [incremental]` — warm-cache best case. `setup()` wipes the cache /
 *    outDir, then runs Declgen once (with emit) to prime both. Each measured
 *    iteration re-runs against the same hot cache with no source changes, so
 *    every file should be a cache hit and downstream stages should skip
 *    non-affected files.
 */
function registerVariants(label: string, inputFiles: readonly string[]): void {
  if (inputFiles.length === 0) {
    return;
  }

  const fullOutDir = path.join(RESULTS_ROOT, '_emit', 'full', label);
  defineBench(`declgen/${label} [full] (${inputFiles.length} files)`, () => {
    const declgen = new Declgen({ inputFiles: [...inputFiles], outDir: fullOutDir });
    const { stageTimings } = declgen.run();
    // Emit is intentionally skipped to keep the timed region focused on pipeline work.
    return stageTimings;
  });

  const incOutDir = path.join(RESULTS_ROOT, '_emit', 'incremental', label);
  const incCacheDir = path.join(RESULTS_ROOT, '_cache', label);
  defineBench(
    `declgen/${label} [incremental] (${inputFiles.length} files)`,
    () => {
      const declgen = new Declgen({
        inputFiles: [...inputFiles],
        outDir: incOutDir,
        cacheDir: incCacheDir,
        incremental: true
      });
      const { stageTimings } = declgen.run();
      return stageTimings;
    },
    {
      setup: () => {
        rimraf(incOutDir);
        rimraf(incCacheDir);
        // Cold prime: populate cache + emit outputs so `verifyOutputs` passes
        // on subsequent runs. This is NOT timed.
        const primer = new Declgen({
          inputFiles: [...inputFiles],
          outDir: incOutDir,
          cacheDir: incCacheDir,
          incremental: true
        });
        primer.run();
        primer.emit();
      }
    }
  );
}

function registerSizeBucket(size: 'small' | 'medium' | 'large'): void {
  const dir = path.join(FIXTURE_ROOT, size);
  registerVariants(size, collectTsFiles(dir));
}

registerSizeBucket('small');
registerSizeBucket('medium');
registerSizeBucket('large');

// ---------------------------------------------------------------------------
// External real-world projects (fetched by `npm run bench:fetch`).
// Skipped silently if the project hasn't been cloned yet.
// ---------------------------------------------------------------------------

interface ExternalProject {
  name: string;
  subdir: string;
}

const EXTERNAL_PROJECTS: ExternalProject[] = [
  { name: 'zod', subdir: 'src' },
  { name: 'rxjs', subdir: 'src' },
  { name: 'typeorm', subdir: 'src' },
  { name: 'nestjs', subdir: 'packages' }
];

function isTestFile(file: string): boolean {
  const base = path.basename(file);
  if (base.endsWith('.spec.ts') || base.endsWith('.test.ts')) {
    return true;
  }
  // Drop common test/fixture directories so timings reflect production sources.
  return /\/(?:test|tests|__tests__|spec|fixtures?)\//.test(file);
}

for (const proj of EXTERNAL_PROJECTS) {
  const dir = path.join(FIXTURE_ROOT, 'external', proj.name, proj.subdir);
  const files = collectTsFiles(dir).filter((f) => !isTestFile(f));
  registerVariants(`external/${proj.name}`, files);
}

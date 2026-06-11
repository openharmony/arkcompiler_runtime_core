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

import * as assert from 'assert';
import * as fs from 'fs';
import * as os from 'os';
import * as path from 'path';
import { before, beforeEach, afterEach, describe, it } from 'mocha';

import { Declgen, DeclgenOptions } from '../../src/Declgen';
import { Logger } from '../../src/logger/Logger';
import { SilentLogger } from '../../src/logger/SilentLogger';

/**
 * Cross-read regression: an importer must be re-processed by SemanticFix
 * after a dependency changes, even when the importer's own text is unchanged.
 *
 * Scenario:
 *   - A.ts defines a concrete method `foo`; B.ts extends A without overriding.
 *   - The user edits A.ts to make `foo` abstract.
 *   - On the second run, B's own text is unchanged, so B enters the affected
 *     set only via the reverse-import closure.
 *   - SemanticFixStage / OverrideFixStage must still run on B and synthesize
 *     the missing override `foo(): void;` in B.d.ets.
 *
 * The pre-fix bug: AffectedAnalysisStage narrowed B out before SemanticFix
 * (because B's own stage outputs hadn't changed), so the synthesized member
 * never made it into B.d.ets.
 */
describe('incremental cross-read: abstract method override is synthesized after dep change', function () {
  this.timeout(60000);

  let rootDir: string;
  let outDir: string;
  let cacheDir: string;
  let aFile: string;
  let bFile: string;

  before(() => Logger.init(new SilentLogger()));

  beforeEach(() => {
    rootDir = fs.mkdtempSync(path.join(os.tmpdir(), 'declgen-selfchanged-'));
    outDir = path.join(rootDir, 'out');
    cacheDir = path.join(rootDir, '.cache');
    aFile = path.join(rootDir, 'A.ts');
    bFile = path.join(rootDir, 'B.ts');
  });

  afterEach(() => {
    fs.rmSync(rootDir, { recursive: true, force: true });
  });

  function write(file: string, content: string): void {
    fs.writeFileSync(file, content, 'utf8');
  }

  function runOnce(): void {
    const d = new Declgen(buildOptions());
    d.run();
    d.emit();
  }

  function buildOptions(): DeclgenOptions {
    return {
      rootDir,
      outDir,
      cacheDir,
      incremental: true,
      inputFiles: [aFile, bFile]
    };
  }

  it('B.d.ets gains the synthesized override on the second run when A becomes abstract', () => {
    // v1: A.foo is concrete. B does not need to override anything.
    write(
      aFile,
      `export class A {
         foo(p?: string): string { return p ?? ''; }
       }
      `
    );
    write(
      bFile,
      `import { A } from './A';
       export class B extends A {}
      `
    );
    runOnce();

    const bDets = path.join(outDir, 'B.d.ets');
    const bV1 = fs.readFileSync(bDets, 'utf8');
    assert.ok(
      !/\bfoo\s*\(\s*\)\s*:\s*void/.test(bV1),
      'sanity: v1 B.d.ets must not yet contain the synthesized override'
    );

    // v2: only A changes — foo becomes abstract. B.ts is untouched.
    write(
      aFile,
      `export abstract class A {
         abstract foo(): void;
       }
      `
    );
    runOnce();

    const bV2 = fs.readFileSync(bDets, 'utf8');
    assert.match(
      bV2,
      /foo\s*\(\s*\)\s*:\s*void/,
      'after A becomes abstract, B.d.ets must include the synthesized `foo(): void` override'
    );
  });
});

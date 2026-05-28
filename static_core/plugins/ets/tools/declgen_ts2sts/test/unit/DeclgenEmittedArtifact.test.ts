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
import { describe, it, before, beforeEach, afterEach } from 'mocha';

import { Declgen, DeclgenOptions } from '../../src/Declgen';
import { Logger } from '../../src/logger/Logger';
import { SilentLogger } from '../../src/logger/SilentLogger';

interface Layout {
  rootDir: string;
  outDir: string;
  cacheDir: string;
  entryFile: string;
  detsFile: string;
}

function createLayout(): Layout {
  const rootDir = fs.mkdtempSync(path.join(os.tmpdir(), 'declgen-emit-'));
  const outDir = path.join(rootDir, 'out');
  const cacheDir = path.join(rootDir, '.cache');
  const entryFile = path.join(rootDir, 'entry.ts');
  fs.writeFileSync(entryFile, 'export const value: number = 1;\n', 'utf8');
  return {
    rootDir,
    outDir,
    cacheDir,
    entryFile,
    detsFile: path.join(outDir, 'entry.d.ets')
  };
}

function buildOptions(layout: Layout, overrides?: Partial<DeclgenOptions>): DeclgenOptions {
  return {
    rootDir: layout.rootDir,
    inputFiles: [layout.entryFile],
    outDir: layout.outDir,
    cacheDir: layout.cacheDir,
    incremental: true,
    ...overrides
  };
}

function readManifestEntry(cacheDir: string, absSrc: string): Record<string, unknown> | undefined {
  const manifestPath = path.join(cacheDir, 'manifest.json');
  if (!fs.existsSync(manifestPath)) {
    return undefined;
  }
  const manifest = JSON.parse(fs.readFileSync(manifestPath, 'utf8'));
  return manifest.files?.[absSrc];
}

describe('Declgen emittedArtifact tracking (Phase 2/3)', function () {
  this.timeout(30000);

  let layout: Layout;

  before(() => {
    Logger.init(new SilentLogger());
  });

  beforeEach(() => {
    layout = createLayout();
  });

  afterEach(() => {
    fs.rmSync(layout.rootDir, { recursive: true, force: true });
  });

  it('records emittedArtifact for the default emit path', () => {
    const declgen = new Declgen(buildOptions(layout));
    declgen.run().emit();

    assert.strictEqual(fs.existsSync(layout.detsFile), true, '.d.ets should be written');

    const entry = readManifestEntry(layout.cacheDir, layout.entryFile);
    assert.ok(entry, 'manifest entry for entry.ts should exist');
    const emitted = entry!.emittedArtifact as Record<string, unknown> | undefined;
    assert.ok(emitted, 'emittedArtifact should be recorded');
    assert.strictEqual(emitted!.path, layout.detsFile);
    assert.strictEqual(typeof emitted!.sha256, 'string');
    assert.ok((emitted!.sha256 as string).length === 64);
    assert.strictEqual(typeof emitted!.size, 'number');
    assert.strictEqual(typeof emitted!.mtime, 'number');
  });

  it('re-emits a file when its previously-recorded .d.ets is deleted', () => {
    // First run: produces .d.ets and records emittedArtifact.
    new Declgen(buildOptions(layout)).run();
    const first = new Declgen(buildOptions(layout));
    first.run().emit();
    assert.strictEqual(fs.existsSync(layout.detsFile), true);

    // Simulate user deleting the output.
    fs.unlinkSync(layout.detsFile);
    assert.strictEqual(fs.existsSync(layout.detsFile), false);

    // Second run: even though source is unchanged, verifyOutputs should detect
    // the missing output and re-emit it.
    const second = new Declgen(buildOptions(layout));
    second.run().emit();

    assert.strictEqual(fs.existsSync(layout.detsFile), true, '.d.ets should be regenerated');
  });

  it('does not re-emit when verifyOutputs is disabled', () => {
    // Seed cache + emitted artifact record.
    const first = new Declgen(buildOptions(layout));
    first.run().emit();
    assert.strictEqual(fs.existsSync(layout.detsFile), true);

    fs.unlinkSync(layout.detsFile);

    // With verifyOutputs=false, the missing output should NOT mark the file
    // affected, AND emit is now incremental (only writes affected files), so
    // the deleted output is intentionally NOT regenerated. Callers that need
    // the regeneration behavior must keep verifyOutputs at its default (true).
    const second = new Declgen(buildOptions(layout, { verifyOutputs: false }));
    second.run().emit();
    assert.strictEqual(fs.existsSync(layout.detsFile), false);
  });

  it('honors the custom writeFile opt-in via { artifactPath }', () => {
    const declgen = new Declgen(buildOptions(layout));
    const result = declgen.run();

    const customPath = path.join(layout.rootDir, 'custom.d.ets');
    const writes: Array<{ fileName: string; content: string }> = [];
    result.emit((fileName, content) => {
      writes.push({ fileName, content });
      fs.writeFileSync(customPath, content, 'utf8');
      return { artifactPath: customPath };
    });

    assert.strictEqual(writes.length, 1);
    assert.strictEqual(writes[0].fileName, layout.entryFile);
    assert.ok(writes[0].content.startsWith("'use static'\n"));

    const entry = readManifestEntry(layout.cacheDir, layout.entryFile);
    const emitted = entry?.emittedArtifact as Record<string, unknown> | undefined;
    assert.ok(emitted, 'opt-in custom writeFile should record emittedArtifact');
    assert.strictEqual(emitted!.path, customPath);
  });

  it('skips emittedArtifact recording when custom writeFile returns undefined', () => {
    const declgen = new Declgen(buildOptions(layout));
    const result = declgen.run();

    result.emit(() => {
      // Returns undefined → no recording.
      return undefined;
    });

    const entry = readManifestEntry(layout.cacheDir, layout.entryFile);
    assert.strictEqual(
      entry?.emittedArtifact,
      undefined,
      'no emittedArtifact should be recorded when custom writeFile opts out'
    );
  });

  it('passes meta.hash equal to the on-disk file hash and the manifest entry', () => {
    const declgen = new Declgen(buildOptions(layout));
    const result = declgen.run();

    const customPath = path.join(layout.rootDir, 'custom.d.ets');
    const observed: Array<{ content: string; hash: string }> = [];
    result.emit((_fileName, content, meta) => {
      observed.push({ content, hash: meta.hash });
      fs.writeFileSync(customPath, content, 'utf8');
      return { artifactPath: customPath };
    });

    assert.strictEqual(observed.length, 1);
    const { content, hash } = observed[0];

    // meta.hash must be a lowercase sha256 hex of EXACTLY the bytes handed to writeFile.
    assert.strictEqual(hash.length, 64);
    assert.ok(/^[0-9a-f]{64}$/.test(hash), `meta.hash should be lowercase hex, got: ${hash}`);
    const recomputed = require('crypto').createHash('sha256').update(content).digest('hex');
    assert.strictEqual(hash, recomputed, 'meta.hash must equal sha256(content)');

    // And it must match what landed on disk.
    const onDisk = fs.readFileSync(customPath);
    const diskHash = require('crypto').createHash('sha256').update(onDisk).digest('hex');
    assert.strictEqual(hash, diskHash, 'meta.hash must equal sha256(file on disk)');

    // And the value declgen recorded into the manifest.
    const entry = readManifestEntry(layout.cacheDir, layout.entryFile);
    const emitted = entry?.emittedArtifact as Record<string, unknown> | undefined;
    assert.ok(emitted);
    assert.strictEqual(emitted!.sha256, hash, 'manifest emittedArtifact.sha256 must equal meta.hash');
  });

  // Regression: when the user deletes the previously-emitted `.d.ets`, the
  // re-emit must run the *full* transformation pipeline for that file, not
  // just print the cached declaration-stage AST. Otherwise the regenerated
  // file is missing the products of InteropTypesConversion / SemanticFix /
  // ASTFix and is observably different from a clean full build.
  describe('re-emit after artifact deletion must match a clean full build', function () {
    let multiRoot: string;
    let multiOut: string;
    let multiCache: string;
    let aFile: string;
    let bFile: string;

    beforeEach(() => {
      multiRoot = fs.mkdtempSync(path.join(os.tmpdir(), 'declgen-reemit-'));
      multiOut = path.join(multiRoot, 'out');
      multiCache = path.join(multiRoot, '.cache');
      aFile = path.join(multiRoot, 'A.ts');
      bFile = path.join(multiRoot, 'B.ts');
      // A: triggers InteropTypesConversion (Transferable → Any) and emits an
      //    abstract method that B must override.
      fs.writeFileSync(
        aFile,
        `export declare namespace A {
           class Array<T> {
             static from<T>(arrayLike: ArrayLike<T>, mapfn: (v: T, k: number) => T): Array<T>;
             static from<T, U>(arrayLike: ArrayLike<T>, mapfn: (v: T, k: number) => U): Array<U>;
           }
         }
         export abstract class B {
           some?: Transferable;
           abstract foo(): void;
         }
        `,
        'utf8'
      );
      // B: SemanticFix/OverrideFix must synthesize `foo(): void;` here.
      fs.writeFileSync(
        bFile,
        `import { B } from './A';
         export class C extends B {
           foo(p?: string): string { return p ?? 'default'; }
           bar(): void {}
         }
        `,
        'utf8'
      );
    });

    afterEach(() => {
      fs.rmSync(multiRoot, { recursive: true, force: true });
    });

    function runFresh(): { a: string; b: string } {
      // Clean cache + out → full pipeline.
      fs.rmSync(multiOut, { recursive: true, force: true });
      fs.rmSync(multiCache, { recursive: true, force: true });
      const d = new Declgen({
        rootDir: multiRoot,
        outDir: multiOut,
        cacheDir: multiCache,
        incremental: true,
        inputFiles: [aFile, bFile]
      });
      d.run();
      d.emit();
      return {
        a: fs.readFileSync(path.join(multiOut, 'A.d.ets'), 'utf8'),
        b: fs.readFileSync(path.join(multiOut, 'B.d.ets'), 'utf8')
      };
    }

    it('produces byte-identical outputs whether built fresh or after deleting both .d.ets', () => {
      // Baseline: a clean full build.
      const baseline = runFresh();
      // Sanity: the pipeline really did apply its cross-file transforms.
      assert.match(baseline.a, /some\?\s*:\s*Any/, 'baseline A.d.ets must show InteropTypesConversion (Transferable→Any)');
      assert.match(baseline.b, /foo\s*\(\s*\)\s*:\s*void/, 'baseline B.d.ets must contain the synthesized override');

      // Now simulate "incremental build after the user deleted both outputs":
      // keep the cache from the fresh run, drop only the user-visible .d.ets.
      fs.unlinkSync(path.join(multiOut, 'A.d.ets'));
      fs.unlinkSync(path.join(multiOut, 'B.d.ets'));

      const d = new Declgen({
        rootDir: multiRoot,
        outDir: multiOut,
        cacheDir: multiCache,
        incremental: true,
        inputFiles: [aFile, bFile]
      });
      d.run();
      d.emit();

      const aReemit = fs.readFileSync(path.join(multiOut, 'A.d.ets'), 'utf8');
      const bReemit = fs.readFileSync(path.join(multiOut, 'B.d.ets'), 'utf8');
      assert.strictEqual(aReemit, baseline.a, 'A.d.ets after artifact-deletion re-emit must match the clean build');
      assert.strictEqual(bReemit, baseline.b, 'B.d.ets after artifact-deletion re-emit must match the clean build');
    });
  });
});

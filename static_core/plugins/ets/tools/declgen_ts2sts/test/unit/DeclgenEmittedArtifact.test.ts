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

  // Regression: a source change that does NOT alter the declaration-stage
  // output text (e.g. literal initializer value of an exported `let`) used to
  // misbehave in 3-run sequences when `verifyOutputs` was disabled:
  //   Run 1: cold cache, emit + record emittedArtifact, then user deletes .d.ets.
  //   Run 2: input changed → file is affected and re-runs DeclarationConversion,
  //          but the printed declaration is identical, so AffectedAnalysis
  //          narrows it back out and emit is skipped. With verifyOutputs=false,
  //          the deleted .d.ets is intentionally NOT regenerated either.
  //   Run 3: nothing changed → full cache hit, emit should also be skipped.
  // The bug was that Run 2's CacheSaveStage call to `putStageArtifact` wiped
  // the inherited `emittedArtifact` record (ensureUpdate lost it), so Run 3
  // observed `emittedArtifact === undefined`, fell into buildFilesToEmit's
  // "never emitted" fallback, and re-emitted. With the fix, emit must stay
  // skipped on both Run 2 and Run 3.
  it('keeps emit skipped across runs when declaration output is unchanged and verifyOutputs is disabled', () => {
    // Run 1: cold cache.
    fs.writeFileSync(layout.entryFile, 'export let foo: number = 421;\n', 'utf8');
    new Declgen(buildOptions(layout, { verifyOutputs: false })).run().emit();
    assert.strictEqual(fs.existsSync(layout.detsFile), true, 'Run 1 must emit');

    // User deletes the output. With verifyOutputs=false we accept that this
    // deletion will not be detected; the test is about the *next two* runs
    // agreeing on the same answer.
    fs.unlinkSync(layout.detsFile);

    // Run 2: source changes but the declaration output (initializer stripped)
    // is byte-identical, so emit should be skipped.
    fs.writeFileSync(layout.entryFile, 'export let foo: number = 42;\n', 'utf8');
    new Declgen(buildOptions(layout, { verifyOutputs: false })).run().emit();
    assert.strictEqual(
      fs.existsSync(layout.detsFile),
      false,
      'Run 2 must NOT emit: input changed but declaration output is unchanged and verifyOutputs is off'
    );

    // Run 3: no change. Must agree with Run 2 — pre-fix this falsely emitted
    // because Run 2 had clobbered the emittedArtifact manifest record.
    new Declgen(buildOptions(layout, { verifyOutputs: false })).run().emit();
    assert.strictEqual(
      fs.existsSync(layout.detsFile),
      false,
      'Run 3 must NOT emit: full cache hit and Run 2 must have preserved the inherited emittedArtifact record'
    );

    // The manifest's emittedArtifact entry must still be the one written by Run 1.
    const entry = readManifestEntry(layout.cacheDir, layout.entryFile);
    const emitted = entry?.emittedArtifact as Record<string, unknown> | undefined;
    assert.ok(emitted, 'emittedArtifact from Run 1 must survive Runs 2 and 3');
    assert.strictEqual(emitted!.path, layout.detsFile);
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

/**
 * Scheme B (content-diffing emit): a root file that the pipeline marks
 * affected but whose final `.d.ets` output is byte-identical to the previous
 * emit must NOT be re-written, while a file whose output actually changed must
 * be re-written.
 *
 * Scenario:
 *   - B imports A and extends it. A's class shape (method `foo`) is fixed.
 *   - The user edits A.ts to add an unrelated top-level export (`helper`).
 *   - A's own output changes (gains `helper`), so A is emitted.
 *   - B is pulled into the affected closure (it imports A) but its output is
 *     unchanged because A's class shape did not change, so B must be skipped.
 */
describe('incremental emit only writes files whose output changed (Scheme B)', function () {
  this.timeout(60000);

  let rootDir: string;
  let outDir: string;
  let cacheDir: string;
  let aFile: string;
  let bFile: string;

  before(() => Logger.init(new SilentLogger()));

  beforeEach(() => {
    rootDir = fs.mkdtempSync(path.join(os.tmpdir(), 'declgen-schemeb-'));
    outDir = path.join(rootDir, 'out');
    cacheDir = path.join(rootDir, '.cache');
    aFile = path.join(rootDir, 'A.ts');
    bFile = path.join(rootDir, 'B.ts');
  });

  afterEach(() => {
    fs.rmSync(rootDir, { recursive: true, force: true });
  });

  function options(): DeclgenOptions {
    return {
      rootDir,
      outDir,
      cacheDir,
      incremental: true,
      inputFiles: [aFile, bFile]
    };
  }

  it('emits the changed dependency but skips an unchanged importer', () => {
    // v1: A has a fixed class shape; B extends A.
    fs.writeFileSync(
      aFile,
      `export class A {
         foo(): string { return ''; }
       }
       export const VERSION: number = 1;
      `,
      'utf8'
    );
    fs.writeFileSync(
      bFile,
      `import { A } from './A';
       export class B extends A {}
      `,
      'utf8'
    );
    new Declgen(options()).run().emit();

    const aDets = path.join(outDir, 'A.d.ets');
    const bDets = path.join(outDir, 'B.d.ets');
    assert.strictEqual(fs.existsSync(aDets), true);
    assert.strictEqual(fs.existsSync(bDets), true);
    const bV1 = fs.readFileSync(bDets, 'utf8');

    // v2: add an unrelated export to A. A's output changes; A's class shape
    // (and therefore B's output) does not.
    fs.writeFileSync(
      aFile,
      `export class A {
         foo(): string { return ''; }
       }
       export const VERSION: number = 1;
       export function helper(): void {}
      `,
      'utf8'
    );

    // Capture which files the second run actually writes via a custom
    // writeFile that records the emitted file names.
    const emitted: string[] = [];
    new Declgen(options()).run().emit((fileName, content) => {
      emitted.push(fileName);
      const detsName = path.basename(fileName).replace(/\.ts$/, '.d.ets');
      const outPath = path.join(outDir, detsName);
      fs.writeFileSync(outPath, content, 'utf8');
      return { artifactPath: outPath };
    });

    assert.ok(emitted.includes(aFile), 'A.ts (changed output) must be re-emitted');
    assert.ok(!emitted.includes(bFile), 'B.ts (unchanged output) must NOT be re-emitted');

    // The changed dependency picked up its new export.
    assert.match(fs.readFileSync(aDets, 'utf8'), /helper/, 'A.d.ets must contain the new export');
    // The skipped importer is untouched and still has its v1 content.
    assert.strictEqual(fs.readFileSync(bDets, 'utf8'), bV1, 'B.d.ets must be unchanged');
  });

  it('keeps the emit baseline so a later identical run still skips the importer', () => {
    fs.writeFileSync(
      aFile,
      `export class A { foo(): string { return ''; } }
       export function helper(): void {}
      `,
      'utf8'
    );
    fs.writeFileSync(
      bFile,
      `import { A } from './A';
       export class B extends A {}
      `,
      'utf8'
    );
    new Declgen(options()).run().emit();

    // Edit A so B is dragged into the affected closure but its output stays
    // identical; B is skipped (no emittedArtifact churn) yet its baseline must
    // survive into the next manifest.
    fs.writeFileSync(
      aFile,
      `export class A { foo(): string { return ''; } }
       export function helper(): void {}
       export const EXTRA: number = 0;
      `,
      'utf8'
    );
    new Declgen(options()).run().emit();

    // Third run: B is affected again (A changed once more) but its output is
    // still identical. With the baseline preserved, B must again be skipped.
    fs.writeFileSync(
      aFile,
      `export class A { foo(): string { return ''; } }
       export function helper(): void {}
       export const EXTRA: number = 0;
       export const EXTRA2: number = 0;
      `,
      'utf8'
    );
    const emitted: string[] = [];
    new Declgen(options()).run().emit((fileName, content) => {
      emitted.push(fileName);
      const detsName = path.basename(fileName).replace(/\.ts$/, '.d.ets');
      fs.writeFileSync(path.join(outDir, detsName), content, 'utf8');
      return { artifactPath: path.join(outDir, detsName) };
    });

    assert.ok(emitted.includes(aFile), 'A.ts must re-emit on the third run');
    assert.ok(
      !emitted.includes(bFile),
      'B.ts baseline must be preserved so the unchanged importer is skipped again'
    );
  });

  /**
   * With `--no-verify-outputs`, declgen trusts the manifest baseline and does
   * NOT stat the on-disk `.d.ets` files. So a user deleting the outputs between
   * runs is invisible: an unchanged source produces no re-emit. Once the source
   * actually changes (here: `B.foo` becomes abstract, which both rewrites A's
   * own declaration and forces B's subclass to synthesize the override), both
   * files are emitted again.
   */
  it('no-verify-outputs: deleted outputs are not regenerated until the source changes', () => {
    const noVerifyOptions = (): DeclgenOptions => {
      return { rootDir, outDir, cacheDir, incremental: true, verifyOutputs: false, inputFiles: [aFile, bFile] };
    };
    const aDets = path.join(outDir, 'A.d.ets');
    const bDets = path.join(outDir, 'B.d.ets');

    const recordingEmit = (declgen: Declgen): string[] => {
      const emitted: string[] = [];
      declgen.run().emit((fileName, content) => {
        emitted.push(fileName);
        const detsName = path.basename(fileName).replace(/\.ts$/, '.d.ets');
        const outPath = path.join(outDir, detsName);
        fs.mkdirSync(path.dirname(outPath), { recursive: true });
        fs.writeFileSync(outPath, content, 'utf8');
        return { artifactPath: outPath };
      });
      return emitted;
    };

    // Run 1: B.foo is concrete. Emit both, then delete the outputs.
    fs.writeFileSync(aFile, `export class B {\n  foo(): void {}\n}\n`, 'utf8');
    fs.writeFileSync(bFile, `import { B } from './A';\nexport class C extends B {}\n`, 'utf8');
    new Declgen(noVerifyOptions()).run().emit();
    assert.strictEqual(fs.existsSync(aDets), true, 'run 1 must emit A.d.ets');
    assert.strictEqual(fs.existsSync(bDets), true, 'run 1 must emit B.d.ets');
    fs.unlinkSync(aDets);
    fs.unlinkSync(bDets);

    // Run 2: no source change. With verifyOutputs off, the deleted outputs are
    // not detected, so nothing is re-emitted.
    const run2Emitted = recordingEmit(new Declgen(noVerifyOptions()));
    assert.strictEqual(run2Emitted.length, 0, 'run 2 must not regenerate the deleted outputs');
    assert.strictEqual(fs.existsSync(aDets), false, 'A.d.ets must stay deleted');
    assert.strictEqual(fs.existsSync(bDets), false, 'B.d.ets must stay deleted');

    // Run 3: B.foo becomes abstract. A's own output changes and B's subclass
    // gains the synthesized override, so both files are emitted.
    fs.writeFileSync(aFile, `export abstract class B {\n  abstract foo(): void;\n}\n`, 'utf8');
    const run3Emitted = recordingEmit(new Declgen(noVerifyOptions()));
    assert.ok(run3Emitted.includes(aFile), 'run 3 must re-emit A.ts (its declaration changed)');
    assert.ok(run3Emitted.includes(bFile), 'run 3 must re-emit B.ts (synthesized abstract override)');
    assert.match(fs.readFileSync(bDets, 'utf8'), /foo\s*\(\s*\)\s*:\s*void/, 'B.d.ets gains the override');
  });
});

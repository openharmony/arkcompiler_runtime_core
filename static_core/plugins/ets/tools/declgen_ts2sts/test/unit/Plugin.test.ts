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
import { Stage, StageContext, IStage } from '../../src/stages/Stage';
import { DeclgenPlugin, PluginStageSpec } from '../../src/plugin/Plugin';
import { Logger } from '../../src/logger/Logger';
import { SilentLogger } from '../../src/logger/SilentLogger';

interface Observation {
  file: string;
  affected: boolean;
}

class ObserverStage extends Stage<unknown, unknown> {
  override readonly cacheVersion = '1';
  readonly observations: Observation[] = [];

  constructor(private readonly stageName: string) {
    super();
  }

  override get name(): string {
    return this.stageName;
  }

  execute(ctx: StageContext<unknown>): StageContext<unknown> {
    ctx.transformTargetInfos.forEach((info, file) => {
      if (info.affected) {
        this.observations.push({ file, affected: true });
      }
    });
    return ctx;
  }
}

function plugin(name: string, specs: PluginStageSpec[], config?: unknown): DeclgenPlugin {
  return {
    name,
    stages: () => specs,
    cacheKeyContribution: config !== undefined ? () => config : undefined
  };
}

function makeStageInstance(name: string): IStage<unknown, unknown> {
  return new ObserverStage(name);
}

describe('plugin system', function () {
  this.timeout(60000);

  let rootDir: string;
  let outDir: string;
  let aFile: string;
  let bFile: string;

  before(() => Logger.init(new SilentLogger()));

  beforeEach(() => {
    rootDir = fs.mkdtempSync(path.join(os.tmpdir(), 'declgen-plugin-'));
    outDir = path.join(rootDir, 'out');
    aFile = path.join(rootDir, 'A.ts');
    bFile = path.join(rootDir, 'B.ts');
    fs.writeFileSync(aFile, 'export const A = 1;\n');
    fs.writeFileSync(bFile, "import { A } from './A';\nexport const B = A + 1;\n");
  });

  afterEach(() => {
    fs.rmSync(rootDir, { recursive: true, force: true });
  });

  function buildOptions(extra: Partial<DeclgenOptions> = {}): DeclgenOptions {
    return {
      rootDir,
      outDir,
      inputFiles: [aFile, bFile],
      ...extra
    };
  }

  it('rejects invalid plugin id', () => {
    const bad = plugin('bad', [
      { id: 'has spaces', version: '1', stage: makeStageInstance('S'), dependencyKind: 'local-readonly', anchor: 'after-ast-fix' }
    ]);
    assert.throws(() => new Declgen(buildOptions({ plugins: [bad] })), /must match/);
  });

  it('rejects duplicate plugin ids', () => {
    const spec = (): PluginStageSpec => ({
      id: 'com.acme.same',
      version: '1',
      stage: makeStageInstance('S1'),
      dependencyKind: 'local-readonly',
      anchor: 'after-ast-fix'
    });
    const p1 = plugin('p1', [spec()]);
    const p2 = plugin('p2', [spec()]);
    assert.throws(() => new Declgen(buildOptions({ plugins: [p1, p2] })), /duplicate stage id/);
  });

  it('rejects unknown extension point', () => {
    const p = plugin('p', [
      {
        id: 'com.acme.bad-anchor',
        version: '1',
        stage: makeStageInstance('S'),
        dependencyKind: 'local-readonly',
        anchor: 'after-interop-types'
      }
    ]);
    // enableInteropTypesFix not set → after-interop-types anchor missing
    assert.throws(() => new Declgen(buildOptions({ plugins: [p] })), /not available in the current pipeline/);
  });

  it('inserts plugin stage at the requested anchor and runs it', () => {
    const observer = new ObserverStage('Plugin(com.acme.audit)');
    const p = plugin('audit', [
      {
        id: 'com.acme.audit',
        version: '1',
        stage: observer,
        dependencyKind: 'local-readonly',
        anchor: 'after-ast-fix'
      }
    ]);
    const d = new Declgen(buildOptions({ plugins: [p] }));
    d.run();
    // Both source files should have been observed as affected on a fresh run.
    const seen = new Set(observer.observations.map((o) => path.basename(o.file)));
    assert.ok(seen.has('A.ts'), `expected A.ts to be observed, saw ${[...seen].join(',')}`);
    assert.ok(seen.has('B.ts'));
  });

  it('cross-read plugin gets an AffectedAnalysisStage(plugin:<id>) appended', () => {
    const cacheDir = path.join(rootDir, '.cache');
    const observer = new ObserverStage('Plugin(com.acme.cross)');
    const p = plugin('cross', [
      {
        id: 'com.acme.cross',
        version: '1',
        stage: observer,
        dependencyKind: 'cross-read',
        anchor: 'after-conversion',
        requiresFreshChecker: true
      }
    ]);

    // First run: prime cache.
    new Declgen(buildOptions({ plugins: [p], incremental: true, cacheDir })).run();

    // Second run with no source changes: observer should see no affected files
    // (narrowed by the auto-inserted AffectedAnalysisStage(plugin:com.acme.cross)).
    const observer2 = new ObserverStage('Plugin(com.acme.cross)');
    const p2 = plugin('cross', [
      {
        id: 'com.acme.cross',
        version: '1',
        stage: observer2,
        dependencyKind: 'cross-read',
        anchor: 'after-conversion',
        requiresFreshChecker: true
      }
    ]);
    new Declgen(buildOptions({ plugins: [p2], incremental: true, cacheDir })).run();
    assert.strictEqual(
      observer2.observations.length,
      0,
      `expected plugin stage to see no affected files on warm cache, got ${observer2.observations.length}`
    );
  });

  it('global plugin sees every root file after a no-change second run', () => {
    const cacheDir = path.join(rootDir, '.cache');
    const observer = new ObserverStage('Plugin(com.acme.global)');
    const p = plugin('global', [
      {
        id: 'com.acme.global',
        version: '1',
        stage: observer,
        dependencyKind: 'global',
        anchor: 'after-ast-fix'
      }
    ]);
    new Declgen(buildOptions({ plugins: [p], incremental: true, cacheDir })).run();

    const observer2 = new ObserverStage('Plugin(com.acme.global)');
    const p2 = plugin('global', [
      {
        id: 'com.acme.global',
        version: '1',
        stage: observer2,
        dependencyKind: 'global',
        anchor: 'after-ast-fix'
      }
    ]);
    new Declgen(buildOptions({ plugins: [p2], incremental: true, cacheDir })).run();
    const seen = new Set(observer2.observations.map((o) => path.basename(o.file)));
    assert.ok(
      seen.has('A.ts') && seen.has('B.ts'),
      `global plugin must see all files even on warm cache, saw ${[...seen].join(',')}`
    );
  });

  it('plugin config change invalidates the cache', () => {
    const cacheDir = path.join(rootDir, '.cache');

    // Run #1 with config v1.
    const p1 = plugin(
      'configurable',
      [
        {
          id: 'com.acme.cfg',
          version: '1',
          stage: makeStageInstance('Plugin(com.acme.cfg)'),
          dependencyKind: 'local-readonly',
          anchor: 'after-ast-fix'
        }
      ],
      { rule: 'v1' }
    );
    new Declgen(buildOptions({ plugins: [p1], incremental: true, cacheDir })).run();
    const manifest1 = JSON.parse(fs.readFileSync(path.join(cacheDir, 'manifest.json'), 'utf8'));

    // Run #2 with config v2 — same plugin id, different cacheKeyContribution.
    const p2 = plugin(
      'configurable',
      [
        {
          id: 'com.acme.cfg',
          version: '1',
          stage: makeStageInstance('Plugin(com.acme.cfg)'),
          dependencyKind: 'local-readonly',
          anchor: 'after-ast-fix'
        }
      ],
      { rule: 'v2' }
    );
    new Declgen(buildOptions({ plugins: [p2], incremental: true, cacheDir })).run();
    const manifest2 = JSON.parse(fs.readFileSync(path.join(cacheDir, 'manifest.json'), 'utf8'));

    assert.notStrictEqual(
      manifest1.globalKey,
      manifest2.globalKey,
      'plugin config change must invalidate the global cache key'
    );
  });

  it('plugin version bump invalidates the cache', () => {
    const cacheDir = path.join(rootDir, '.cache');
    const makePlugin = (version: string): DeclgenPlugin =>
      plugin('versioned', [
        {
          id: 'com.acme.ver',
          version,
          stage: makeStageInstance('Plugin(com.acme.ver)'),
          dependencyKind: 'local-readonly',
          anchor: 'after-ast-fix'
        }
      ]);

    new Declgen(buildOptions({ plugins: [makePlugin('1.0.0')], incremental: true, cacheDir })).run();
    const k1 = JSON.parse(fs.readFileSync(path.join(cacheDir, 'manifest.json'), 'utf8')).globalKey;
    new Declgen(buildOptions({ plugins: [makePlugin('1.0.1')], incremental: true, cacheDir })).run();
    const k2 = JSON.parse(fs.readFileSync(path.join(cacheDir, 'manifest.json'), 'utf8')).globalKey;
    assert.notStrictEqual(k1, k2);
  });
});

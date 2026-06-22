/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 */

import * as assert from 'assert';
import * as fs from 'fs';
import * as os from 'os';
import * as path from 'path';
import { describe, it } from 'mocha';

import { cacheStore } from '../../src/cache/CacheStore';
import { CACHE_FORMAT_VERSION } from '../../src/cache/CacheVersion';

function tmp(prefix: string): string {
  return fs.mkdtempSync(path.join(os.tmpdir(), prefix));
}

describe('CacheStore', function () {
  this.timeout(5000);

  it('opens a fresh cache directory and reports an empty session', () => {
    const dir = tmp('cache-store-empty-');
    try {
      const session = cacheStore.open({ cacheDir: dir, globalKey: 'k1' });
      assert.strictEqual(session.cacheDir, dir);
      assert.strictEqual(session.globalKey, 'k1');
      assert.strictEqual(session.getFile('/abs/foo.ts'), undefined);
    } finally {
      fs.rmSync(dir, { recursive: true, force: true });
    }
  });

  it('round-trips artifact + manifest across commit and re-open with same globalKey', async () => {
    const dir = tmp('cache-store-roundtrip-');
    try {
      const s1 = cacheStore.open({ cacheDir: dir, globalKey: 'k1' });
      s1.putStageArtifact('/abs/foo.ts', 'declaration', 'export const x: number;', {
        fileKey: 'fk1',
        interopSigHash: null,
        contentHash: ''
      });
      s1.putStageOutputHash('/abs/foo.ts', 'declaration', 'h1');
      await s1.commit();

      const manifestPath = path.join(dir, 'manifest.json');
      assert.ok(fs.existsSync(manifestPath));
      const manifest = JSON.parse(fs.readFileSync(manifestPath, 'utf8'));
      assert.strictEqual(manifest.cacheFormatVersion, CACHE_FORMAT_VERSION);
      assert.strictEqual(manifest.globalKey, 'k1');
      assert.ok(manifest.files['/abs/foo.ts']);

      const s2 = cacheStore.open({ cacheDir: dir, globalKey: 'k1' });
      const entry = s2.getFile('/abs/foo.ts');
      assert.ok(entry);
      assert.strictEqual(entry!.fileKey, 'fk1');
      assert.strictEqual(entry!.stageOutputHashes.declaration, 'h1');
      const declArt = entry!.stageArtifacts.declaration!;
      assert.ok(fs.existsSync(declArt));
      assert.strictEqual(fs.readFileSync(declArt, 'utf8'), 'export const x: number;');
    } finally {
      fs.rmSync(dir, { recursive: true, force: true });
    }
  });

  it('wipes the cache when globalKey changes', async () => {
    const dir = tmp('cache-store-wipe-');
    try {
      const s1 = cacheStore.open({ cacheDir: dir, globalKey: 'k1' });
      s1.putStageArtifact('/abs/foo.ts', 'declaration', 'A', { fileKey: 'fk1', interopSigHash: null, contentHash: '' });
      await s1.commit();

      const s2 = cacheStore.open({ cacheDir: dir, globalKey: 'k2-different' });
      assert.strictEqual(s2.getFile('/abs/foo.ts'), undefined);
      // Manifest is gone after wipe.
      assert.strictEqual(fs.existsSync(path.join(dir, 'manifest.json')), false);
    } finally {
      fs.rmSync(dir, { recursive: true, force: true });
    }
  });

  it('prunes orphan artifacts on commit when a file is forgotten', async () => {
    const dir = tmp('cache-store-prune-');
    try {
      const s1 = cacheStore.open({ cacheDir: dir, globalKey: 'k1' });
      s1.putStageArtifact('/abs/a.ts', 'declaration', 'A', { fileKey: 'fka', interopSigHash: null, contentHash: '' });
      s1.putStageArtifact('/abs/b.ts', 'declaration', 'B', { fileKey: 'fkb', interopSigHash: null, contentHash: '' });
      await s1.commit();

      const s2 = cacheStore.open({ cacheDir: dir, globalKey: 'k1' });
      s2.forget('/abs/a.ts');
      // Re-record b so its manifest entry is kept.
      const b = s2.getFile('/abs/b.ts')!;
      const bArt = b.stageArtifacts.declaration!;
      await s2.commit();

      assert.strictEqual(fs.existsSync(bArt), true, 'b.ts artifact should remain');
      const remainingArtifacts: string[] = [];
      walk(path.join(dir, 'stages'), remainingArtifacts);
      assert.deepStrictEqual(remainingArtifacts, [bArt]);
    } finally {
      fs.rmSync(dir, { recursive: true, force: true });
    }
  });

  it('round-trips contentHash + interopSigHash across commit and reopen', async () => {
    const dir = tmp('cache-store-meta-');
    try {
      const s1 = cacheStore.open({ cacheDir: dir, globalKey: 'k1' });
      s1.putStageArtifact('/abs/foo.ts', 'declaration', 'X', {
        fileKey: 'fk1',
        interopSigHash: 'sig-abc',
        contentHash: 'content-abc'
      });
      await s1.commit();

      const s2 = cacheStore.open({ cacheDir: dir, globalKey: 'k1' });
      const entry = s2.getFile('/abs/foo.ts');
      assert.ok(entry);
      assert.strictEqual(entry!.interopSigHash, 'sig-abc');
      assert.strictEqual(entry!.contentHash, 'content-abc');
    } finally {
      fs.rmSync(dir, { recursive: true, force: true });
    }
  });

  it('replaces the on-disk artifact when content changes for the same file', async () => {
    const dir = tmp('cache-store-replace-');
    try {
      const s1 = cacheStore.open({ cacheDir: dir, globalKey: 'k1' });
      s1.putStageArtifact('/abs/foo.ts', 'declaration', 'first', {
        fileKey: 'fk1',
        interopSigHash: null,
        contentHash: ''
      });
      await s1.commit();
      const firstArt = cacheStore
        .open({ cacheDir: dir, globalKey: 'k1' })
        .getFile('/abs/foo.ts')!.stageArtifacts.declaration!;

      const s2 = cacheStore.open({ cacheDir: dir, globalKey: 'k1' });
      s2.putStageArtifact('/abs/foo.ts', 'declaration', 'second', {
        fileKey: 'fk2',
        interopSigHash: null,
        contentHash: ''
      });
      await s2.commit();
      const secondArt = cacheStore
        .open({ cacheDir: dir, globalKey: 'k1' })
        .getFile('/abs/foo.ts')!.stageArtifacts.declaration!;

      assert.notStrictEqual(firstArt, secondArt, 'artifact path should change on content change');
      assert.strictEqual(fs.existsSync(secondArt), true);
      assert.strictEqual(fs.readFileSync(secondArt, 'utf8'), 'second');
      assert.strictEqual(fs.existsSync(firstArt), false, 'old artifact should be pruned');
    } finally {
      fs.rmSync(dir, { recursive: true, force: true });
    }
  });

  it('treats a corrupt manifest as a fresh cache', () => {
    const dir = tmp('cache-store-corrupt-');
    try {
      fs.mkdirSync(dir, { recursive: true });
      fs.writeFileSync(path.join(dir, 'manifest.json'), '{not valid json', 'utf8');
      const session = cacheStore.open({ cacheDir: dir, globalKey: 'k1' });
      assert.strictEqual(session.getFile('/abs/anything.ts'), undefined);
    } finally {
      fs.rmSync(dir, { recursive: true, force: true });
    }
  });

  it('ignores a manifest with mismatched cacheFormatVersion', () => {
    const dir = tmp('cache-store-format-');
    try {
      fs.mkdirSync(dir, { recursive: true });
      const bogus = {
        cacheFormatVersion: CACHE_FORMAT_VERSION + 999,
        globalKey: 'k1',
        createdAt: new Date().toISOString(),
        tsBuildInfoPath: 'tsbuildinfo',
        files: {
          '/abs/foo.ts': {
            fileKey: 'fk1',
            interopSigHash: null,
            contentHash: '',
            stageArtifacts: {},
            stageOutputHashes: {},
            mtime: 0
          }
        }
      };
      fs.writeFileSync(path.join(dir, 'manifest.json'), JSON.stringify(bogus), 'utf8');
      const session = cacheStore.open({ cacheDir: dir, globalKey: 'k1' });
      assert.strictEqual(session.getFile('/abs/foo.ts'), undefined);
    } finally {
      fs.rmSync(dir, { recursive: true, force: true });
    }
  });

  it('supports multiple commits on the same session, accumulating updates', async () => {
    const dir = tmp('cache-store-multi-commit-');
    try {
      const s1 = cacheStore.open({ cacheDir: dir, globalKey: 'k1' });
      s1.putStageArtifact('/abs/foo.ts', 'declaration', 'A', {
        fileKey: 'fk1',
        interopSigHash: null,
        contentHash: 'c1'
      });
      await s1.commit();

      // Second commit on same session adds a new file; both must survive.
      s1.putStageArtifact('/abs/bar.ts', 'declaration', 'B', {
        fileKey: 'fk2',
        interopSigHash: null,
        contentHash: 'c2'
      });
      await s1.commit();

      const s2 = cacheStore.open({ cacheDir: dir, globalKey: 'k1' });
      assert.ok(s2.getFile('/abs/foo.ts'));
      assert.ok(s2.getFile('/abs/bar.ts'));
    } finally {
      fs.rmSync(dir, { recursive: true, force: true });
    }
  });

  describe('recordEmittedArtifact', () => {
    it('round-trips emittedArtifact across commit and reopen', async () => {
      const dir = tmp('cache-store-emit-roundtrip-');
      try {
        const s1 = cacheStore.open({ cacheDir: dir, globalKey: 'k1' });
        s1.putStageArtifact('/abs/foo.ts', 'declaration', 'X', {
          fileKey: 'fk1',
          interopSigHash: null,
          contentHash: 'c1'
        });
        s1.recordEmittedArtifact('/abs/foo.ts', {
          path: '/out/foo.d.ets',
          sha256: 'deadbeef',
          mtime: 1234567,
          size: 42
        });
        await s1.commit();

        const s2 = cacheStore.open({ cacheDir: dir, globalKey: 'k1' });
        const entry = s2.getFile('/abs/foo.ts');
        assert.ok(entry);
        assert.deepStrictEqual(entry!.emittedArtifact, {
          path: '/out/foo.d.ets',
          sha256: 'deadbeef',
          mtime: 1234567,
          size: 42
        });
      } finally {
        fs.rmSync(dir, { recursive: true, force: true });
      }
    });

    it('overwrites a previously recorded emittedArtifact', async () => {
      const dir = tmp('cache-store-emit-overwrite-');
      try {
        const s1 = cacheStore.open({ cacheDir: dir, globalKey: 'k1' });
        s1.putStageArtifact('/abs/foo.ts', 'declaration', 'X', {
          fileKey: 'fk1',
          interopSigHash: null,
          contentHash: ''
        });
        s1.recordEmittedArtifact('/abs/foo.ts', {
          path: '/out/foo.d.ets',
          sha256: 'aaaa',
          mtime: 1,
          size: 1
        });
        await s1.commit();

        const s2 = cacheStore.open({ cacheDir: dir, globalKey: 'k1' });
        s2.recordEmittedArtifact('/abs/foo.ts', {
          path: '/out/foo.d.ets',
          sha256: 'bbbb',
          mtime: 2,
          size: 2
        });
        await s2.commit();

        const s3 = cacheStore.open({ cacheDir: dir, globalKey: 'k1' });
        assert.strictEqual(s3.getFile('/abs/foo.ts')!.emittedArtifact!.sha256, 'bbbb');
      } finally {
        fs.rmSync(dir, { recursive: true, force: true });
      }
    });

    it('records emittedArtifact even when no putStageArtifact was called for the file', async () => {
      // Defensive: caller could record an emit for a file the cache has no prior knowledge of.
      const dir = tmp('cache-store-emit-orphan-');
      try {
        const s1 = cacheStore.open({ cacheDir: dir, globalKey: 'k1' });
        s1.recordEmittedArtifact('/abs/foo.ts', {
          path: '/out/foo.d.ets',
          sha256: 'cccc',
          mtime: 3,
          size: 3
        });
        await s1.commit();

        const s2 = cacheStore.open({ cacheDir: dir, globalKey: 'k1' });
        const entry = s2.getFile('/abs/foo.ts');
        assert.ok(entry);
        assert.strictEqual(entry!.emittedArtifact!.sha256, 'cccc');
      } finally {
        fs.rmSync(dir, { recursive: true, force: true });
      }
    });

    it('forget() drops the emittedArtifact along with the rest of the entry', async () => {
      const dir = tmp('cache-store-emit-forget-');
      try {
        const s1 = cacheStore.open({ cacheDir: dir, globalKey: 'k1' });
        s1.putStageArtifact('/abs/foo.ts', 'declaration', 'X', {
          fileKey: 'fk1',
          interopSigHash: null,
          contentHash: ''
        });
        s1.recordEmittedArtifact('/abs/foo.ts', {
          path: '/out/foo.d.ets',
          sha256: 'eeee',
          mtime: 4,
          size: 4
        });
        await s1.commit();

        const s2 = cacheStore.open({ cacheDir: dir, globalKey: 'k1' });
        s2.forget('/abs/foo.ts');
        await s2.commit();

        const s3 = cacheStore.open({ cacheDir: dir, globalKey: 'k1' });
        assert.strictEqual(s3.getFile('/abs/foo.ts'), undefined);
      } finally {
        fs.rmSync(dir, { recursive: true, force: true });
      }
    });

    it('leaves emittedArtifact undefined on entries that never had one recorded', async () => {
      const dir = tmp('cache-store-emit-absent-');
      try {
        const s1 = cacheStore.open({ cacheDir: dir, globalKey: 'k1' });
        s1.putStageArtifact('/abs/foo.ts', 'declaration', 'X', {
          fileKey: 'fk1',
          interopSigHash: null,
          contentHash: ''
        });
        await s1.commit();

        const s2 = cacheStore.open({ cacheDir: dir, globalKey: 'k1' });
        assert.strictEqual(s2.getFile('/abs/foo.ts')!.emittedArtifact, undefined);
      } finally {
        fs.rmSync(dir, { recursive: true, force: true });
      }
    });

    it('preserves inherited emittedArtifact when a later run rewrites the stage artifact without re-recording the emit', async () => {
      // Reproduces the incremental bug where:
      //   Run 1: putStageArtifact + recordEmittedArtifact + commit
      //   Run 2: putStageArtifact only (e.g. declaration stage re-ran but
      //          downstream narrowing decided no emit was needed)
      // After Run 2 commits, the manifest must still carry the emit record
      // from Run 1; otherwise Run 3 would think the file was never emitted
      // and re-emit it unnecessarily, while Run 2 itself skipped the emit.
      const dir = tmp('cache-store-emit-preserve-');
      try {
        const s1 = cacheStore.open({ cacheDir: dir, globalKey: 'k1' });
        s1.putStageArtifact('/abs/foo.ts', 'declaration', 'export let foo: number;', {
          fileKey: 'fk1',
          interopSigHash: null,
          contentHash: 'c1'
        });
        s1.recordEmittedArtifact('/abs/foo.ts', {
          path: '/out/foo.d.ets',
          sha256: 'emit-sha-1',
          mtime: 1000,
          size: 24
        });
        await s1.commit();

        const s2 = cacheStore.open({ cacheDir: dir, globalKey: 'k1' });
        // Source content changed but declaration output happens to be identical,
        // so the stage writes a (same-text) artifact and the emit phase is skipped.
        s2.putStageArtifact('/abs/foo.ts', 'declaration', 'export let foo: number;', {
          fileKey: 'fk2',
          interopSigHash: null,
          contentHash: 'c2'
        });
        await s2.commit();

        const s3 = cacheStore.open({ cacheDir: dir, globalKey: 'k1' });
        const entry = s3.getFile('/abs/foo.ts');
        assert.ok(entry);
        assert.strictEqual(entry!.fileKey, 'fk2', 'meta from latest run should win');
        assert.deepStrictEqual(
          entry!.emittedArtifact,
          { path: '/out/foo.d.ets', sha256: 'emit-sha-1', mtime: 1000, size: 24 },
          'emittedArtifact from Run 1 must survive a stage-artifact-only Run 2'
        );
      } finally {
        fs.rmSync(dir, { recursive: true, force: true });
      }
    });
  });
});

function walk(dir: string, out: string[]): void {
  if (!fs.existsSync(dir)) {
    return;
  }
  for (const e of fs.readdirSync(dir, { withFileTypes: true })) {
    const p = path.join(dir, e.name);
    if (e.isDirectory()) {
      walk(p, out);
    } else {
      out.push(p);
    }
  }
}

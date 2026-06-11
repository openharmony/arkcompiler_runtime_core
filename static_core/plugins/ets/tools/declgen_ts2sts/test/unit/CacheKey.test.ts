/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 */

import * as assert from 'assert';
import { describe, it } from 'mocha';

import { canonicalJson, sha256 } from '../../src/cache/Hash';
import { computeFileKey, computeGlobalKey, pickRelevantCompilerOptions } from '../../src/cache/CacheKey';
import { computeInteropSignature } from '../../src/cache/InteropSignature';

describe('canonicalJson', () => {
  it('serializes object keys in sorted order', () => {
    const a = canonicalJson({ b: 1, a: 2 });
    const b = canonicalJson({ a: 2, b: 1 });
    assert.strictEqual(a, b);
    assert.strictEqual(a, '{"a":2,"b":1}');
  });

  it('handles nested objects and arrays', () => {
    const v = canonicalJson({ x: [3, 2, 1], y: { z: null } });
    assert.strictEqual(v, '{"x":[3,2,1],"y":{"z":null}}');
  });
});

describe('CacheKey', () => {
  it('produces identical globalKey for equivalent inputs in any property order', () => {
    const base = {
      declgenVersion: '1.0.0',
      compilerOptions: { target: 99, module: 1, strict: true },
      features: { enableInteropTypesFix: false, removeReservedKeywordIdentifier: true },
      pipelineSignature: 'A@1|B@1',
      rootDir: '/r',
      outDir: '/o'
    };
    const k1 = computeGlobalKey(base);
    const k2 = computeGlobalKey({
      ...base,
      compilerOptions: { strict: true, module: 1, target: 99 }
    });
    assert.strictEqual(k1, k2);
  });

  it('changes globalKey when pipelineSignature bumps', () => {
    const base = {
      declgenVersion: '1.0.0',
      compilerOptions: {},
      features: { enableInteropTypesFix: false, removeReservedKeywordIdentifier: false },
      pipelineSignature: 'A@1',
      rootDir: '/r',
      outDir: '/o'
    };
    const k1 = computeGlobalKey(base);
    const k2 = computeGlobalKey({ ...base, pipelineSignature: 'A@2' });
    assert.notStrictEqual(k1, k2);
  });

  it('computes a stable fileKey insensitive to import order', () => {
    const k1 = computeFileKey({ contentHash: 'h', depsSignature: 's', resolvedImports: ['/a', '/b'] });
    const k2 = computeFileKey({ contentHash: 'h', depsSignature: 's', resolvedImports: ['/b', '/a'] });
    assert.strictEqual(k1, k2);
  });

  it('pickRelevantCompilerOptions excludes path-only options', () => {
    const opts = pickRelevantCompilerOptions({
      target: 99,
      outDir: '/x',
      tsBuildInfoFile: '/y',
      incremental: true,
      strict: true
    } as any);
    assert.deepStrictEqual(opts, { target: 99, strict: true });
  });
});

describe('computeInteropSignature', () => {
  it('returns empty string when no tags are present', () => {
    assert.strictEqual(computeInteropSignature('const x = 1;'), '');
  });

  it('is sensitive to tag presence', () => {
    const a = computeInteropSignature('// @interop any');
    const b = computeInteropSignature('// other comment');
    assert.notStrictEqual(a, b);
  });

  it('is insensitive to ordering between tags', () => {
    const a = computeInteropSignature('/** @interop any */\n/** @noninterop */');
    const b = computeInteropSignature('/** @noninterop */\n/** @interop any */');
    assert.strictEqual(a, b);
  });

  it('returns a sha256 hex digest', () => {
    const v = computeInteropSignature('// @interop break-extends');
    assert.match(v, /^[0-9a-f]{64}$/);
    void sha256;
  });
});

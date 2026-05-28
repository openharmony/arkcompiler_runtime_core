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
import * as crypto from 'crypto';
import { describe, it } from 'mocha';

const SCRIPT = path.resolve(__dirname, '..', '..', '..', 'script', 'write-impl-hash.js');

describe('write-impl-hash script', function () {
  this.timeout(5000);

  it('produces a stable hash and rewrites ImplHash.generated.js', () => {
    const tmpRoot = fs.mkdtempSync(path.join(os.tmpdir(), 'impl-hash-'));
    try {
      const buildSrc = path.join(tmpRoot, 'build', 'src');
      const cacheDir = path.join(buildSrc, 'cache');
      fs.mkdirSync(cacheDir, { recursive: true });
      fs.writeFileSync(path.join(buildSrc, 'a.js'), 'module.exports = 1;\n');
      fs.writeFileSync(path.join(buildSrc, 'b.js'), 'module.exports = 2;\n');
      // Pre-existing placeholder to be overwritten.
      fs.writeFileSync(
        path.join(cacheDir, 'ImplHash.generated.js'),
        "exports.DECLGEN_IMPL_HASH = 'placeholder';\n"
      );

      // Stand-in __dirname so the script picks up our temp tree.
      // The script resolves BUILD_SRC as ../build/src relative to itself; we
      // execute it under a fake `script/` sub-dir so the relative path lands
      // on our temp root.
      const fakeScripts = path.join(tmpRoot, 'script');
      fs.mkdirSync(fakeScripts);
      fs.copyFileSync(SCRIPT, path.join(fakeScripts, 'write-impl-hash.js'));

      // Run by `require()`-ing — the script computes and writes synchronously.
      require(path.join(fakeScripts, 'write-impl-hash.js'));

      const out = require(path.join(cacheDir, 'ImplHash.generated.js')) as { DECLGEN_IMPL_HASH: string };
      assert.notStrictEqual(out.DECLGEN_IMPL_HASH, 'placeholder');
      assert.match(out.DECLGEN_IMPL_HASH, /^[0-9a-f]{64}$/);

      // Recompute manually to confirm the algorithm.
      const h = crypto.createHash('sha256');
      const files = ['a.js', 'b.js'].sort().map((n) => path.join(buildSrc, n));
      for (const f of files) {
        h.update(path.relative(buildSrc, f));
        h.update('\0');
        h.update(fs.readFileSync(f));
        h.update('\0');
      }
      assert.strictEqual(out.DECLGEN_IMPL_HASH, h.digest('hex'));
    } finally {
      fs.rmSync(tmpRoot, { recursive: true, force: true });
    }
  });
});

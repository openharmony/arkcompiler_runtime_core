/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 */

// Computes DECLGEN_IMPL_HASH = sha256(sorted concat of all build/src/**/*.js)
// and writes build/src/cache/ImplHash.generated.js, replacing the dev
// placeholder so the runtime constant reflects the actual compiled artefacts.
'use strict';

const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');

const BUILD_SRC = path.resolve(__dirname, '..', 'build', 'src');
const OUT_FILE = path.join(BUILD_SRC, 'cache', 'ImplHash.generated.js');
const OUT_DTS = path.join(BUILD_SRC, 'cache', 'ImplHash.generated.d.ts');

function walk(dir, out) {
  for (const entry of fs.readdirSync(dir, { withFileTypes: true })) {
    const p = path.join(dir, entry.name);
    if (entry.isDirectory()) {
      walk(p, out);
    } else if (entry.isFile() && p.endsWith('.js') && p !== OUT_FILE) {
      out.push(p);
    }
  }
}

function main() {
  if (!fs.existsSync(BUILD_SRC)) {
    console.error(`[declgen] write-impl-hash: ${BUILD_SRC} does not exist; run \`tsc\` first.`);
    process.exit(1);
  }

  const files = [];
  walk(BUILD_SRC, files);
  files.sort();

  const h = crypto.createHash('sha256');
  for (const f of files) {
    h.update(path.relative(BUILD_SRC, f));
    h.update('\0');
    h.update(fs.readFileSync(f));
    h.update('\0');
  }
  const hash = h.digest('hex');

  fs.mkdirSync(path.dirname(OUT_FILE), { recursive: true });
  fs.writeFileSync(
    OUT_FILE,
    `'use strict';\nObject.defineProperty(exports, "__esModule", { value: true });\nexports.DECLGEN_IMPL_HASH = ${JSON.stringify(hash)};\n`
  );
  fs.writeFileSync(OUT_DTS, 'export declare const DECLGEN_IMPL_HASH: string;\n');
  console.log(`[declgen] impl hash = ${hash}`);
}

main();

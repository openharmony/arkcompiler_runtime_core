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
import * as ts from 'typescript';
import { describe, it } from 'mocha';
import { VirtualFileHost, normalizePath, INTERNAL_PREFIX } from '../../src/compiler/VirtualFileHost';

function createVirtualHost(): VirtualFileHost {
  return new VirtualFileHost({
    target: ts.ScriptTarget.ES2021,
    module: ts.ModuleKind.CommonJS,
  });
}

describe('VirtualFileHost', () => {
  it('should update and read virtual file content', () => {
    const virtualHost = createVirtualHost();
    const fileName = '/virtual/sample.d.ts';
    const content = 'declare const foo: number;';

    virtualHost.updateFile(fileName, content);

    assert.strictEqual(virtualHost.readFile(fileName), content);
    assert.strictEqual(virtualHost.fileExists(fileName), true);
  });

  it('should use virtual content first, then fallback to base readFile', () => {
    const tempDir = fs.mkdtempSync(path.join(os.tmpdir(), 'vfh-test-'));
    const baseFileName = path.join(tempDir, 'base.d.ts');
    const overrideFileName = path.join(tempDir, 'override.d.ts');

    fs.writeFileSync(baseFileName, 'declare const fromBase: string;', 'utf8');
    fs.writeFileSync(overrideFileName, 'declare const oldValue: string;', 'utf8');

    const virtualHost = createVirtualHost();
    virtualHost.updateFile(overrideFileName, 'declare const newValue: string;');

    const compilerHost = virtualHost.getCompilerHost();

    assert.strictEqual(compilerHost.readFile(overrideFileName), 'declare const newValue: string;');
    assert.strictEqual(compilerHost.readFile(baseFileName), 'declare const fromBase: string;');

    fs.rmSync(tempDir, { recursive: true, force: true });
  });

  it('should check file existence using virtual files and base fallback', () => {
    const tempDir = fs.mkdtempSync(path.join(os.tmpdir(), 'vfh-test-'));
    const baseFileName = path.join(tempDir, 'existing.d.ts');
    const virtualFileName = path.join(tempDir, 'virtual-existing.d.ts');

    fs.writeFileSync(baseFileName, 'declare const baseExists: boolean;', 'utf8');

    const virtualHost = createVirtualHost();
    virtualHost.updateFile(virtualFileName, 'declare const virtualExists: boolean;');

    const compilerHost = virtualHost.getCompilerHost();

    assert.strictEqual(compilerHost.fileExists(virtualFileName), true);
    assert.strictEqual(compilerHost.fileExists(baseFileName), true);
    assert.strictEqual(compilerHost.fileExists(path.join(tempDir, 'missing-file.d.ts')), false);

    fs.rmSync(tempDir, { recursive: true, force: true });
  });

  it('should create SourceFile from virtual content and fallback to base host', () => {
    const tempDir = fs.mkdtempSync(path.join(os.tmpdir(), 'vfh-test-'));
    const baseFileName = path.join(tempDir, 'source.d.ts');
    const virtualFileName = path.join(tempDir, 'virtual-source.d.ts');

    fs.writeFileSync(baseFileName, 'declare const fromBase: string;', 'utf8');

    const virtualHost = createVirtualHost();
    virtualHost.updateFile(virtualFileName, 'declare const fromVirtual: number;');

    const compilerHost = virtualHost.getCompilerHost();

    const virtualSource = compilerHost.getSourceFile(virtualFileName, ts.ScriptTarget.ES2021);
    assert.ok(virtualSource);
    assert.strictEqual(virtualSource?.text.includes('fromVirtual'), true);

    const baseSource = compilerHost.getSourceFile(baseFileName, ts.ScriptTarget.ES2021);
    assert.ok(baseSource);
    assert.strictEqual(baseSource?.text.includes('fromBase'), true);

    fs.rmSync(tempDir, { recursive: true, force: true });
  });

  it('should expose createHash and match ts.sys.createHash', () => {
    const virtualHost = createVirtualHost();
    const input = 'hash-input';
    assert.strictEqual(virtualHost.createHash(input), ts.sys.createHash!(input));
  });
});

describe('normalizePath', () => {
  it('should return an absolute path with forward slashes only', () => {
    const resolved = normalizePath('foo/bar/baz.ts');
    assert.strictEqual(resolved.includes('\\'), false, `normalizePath returned a backslash: ${resolved}`);
    assert.strictEqual(path.isAbsolute(resolved), true);
  });

  it('should normalize Windows-style backslashes to forward slashes', () => {
    // Use posix.resolve to construct a deterministic absolute path, then convert
    // separators so the input mimics what path.resolve would produce on Windows.
    const posixAbs = path.posix.resolve('/some/project/src/foo.ts');
    const winStyle = posixAbs.replace(/\//g, '\\');
    // Round-trip through normalizePath: even if the OS-native path.resolve
    // returns backslashes, normalizePath must surface forward slashes.
    const resolved = normalizePath(winStyle);
    assert.strictEqual(resolved.includes('\\'), false, `normalizePath returned a backslash: ${resolved}`);
  });

  it('should produce keys consistent with TypeScript SourceFile.fileName on the same input', () => {
    // ts.createSourceFile stores fileName as-is, but when TypeScript emits a
    // path it always uses forward slashes (see ts.normalizePath). Our
    // normalizePath must produce the same forward-slash representation so that
    // Map/Set lookups keyed by normalizePath(...) match SourceFile.fileName.
    const input = 'src/foo.ts';
    const resolved = normalizePath(input);
    const sf = ts.createSourceFile(resolved, '', ts.ScriptTarget.ES2021);
    assert.strictEqual(sf.fileName, resolved);
    assert.strictEqual(sf.fileName.includes('\\'), false);
  });

  it('should leave INTERNAL_PREFIX paths unchanged', () => {
    const internal = `${INTERNAL_PREFIX}/lib.foo.d.ts`;
    assert.strictEqual(normalizePath(internal), internal);
  });
});

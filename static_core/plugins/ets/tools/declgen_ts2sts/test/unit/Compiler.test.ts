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
import { Compiler, defaultCompilerOptions } from '../../src/compiler/Compiler';

function createTempDir(prefix: string): string {
	return fs.mkdtempSync(path.join(os.tmpdir(), prefix));
}

function writeFile(filePath: string, content: string): void {
	fs.mkdirSync(path.dirname(filePath), { recursive: true });
	fs.writeFileSync(filePath, content, 'utf8');
}

function createCompilerOptions(outDir: string, overrides?: ts.CompilerOptions): ts.CompilerOptions {
	return {
		...defaultCompilerOptions(),
		outDir,
		declaration: true,
		emitDeclarationOnly: true,
		noEmit: false,
		...overrides
	};
}

describe('Compiler', function () {
	this.timeout(10000);

	it('should compile in non-incremental mode and expose program/typeChecker', () => {
		const tempDir = createTempDir('compiler-non-incremental-');
		const entryFile = path.join(tempDir, 'entry.ts');
		const outDir = path.join(tempDir, 'out');

		writeFile(entryFile, 'export const value: number = 1;');

		const compiler = new Compiler([entryFile], [], createCompilerOptions(outDir));
		compiler.compile();

		const sourceFile = compiler.getSourceFile(entryFile);
		assert.ok(sourceFile);
		assert.ok(compiler.program);
		assert.ok(compiler.typeChecker);

		fs.rmSync(tempDir, { recursive: true, force: true });
	});

	it('should persist tsbuildinfo in incremental mode after emit', () => {
		const tempDir = createTempDir('compiler-incremental-buildinfo-');
		const entryFile = path.join(tempDir, 'entry.ts');
		const outDir = path.join(tempDir, 'out');
		const tsBuildInfoFile = path.join(tempDir, '.tsbuildinfo');

		writeFile(entryFile, 'export const value: number = 1;');

		const compiler = new Compiler(
			[entryFile],
			[],
			createCompilerOptions(outDir, {
				incremental: true,
				tsBuildInfoFile
			})
		);

		compiler.compile();
		compiler.emit(() => {
			return;
		});

		assert.strictEqual(fs.existsSync(tsBuildInfoFile), true);

		fs.rmSync(tempDir, { recursive: true, force: true });
	});

	it('should treat all files as affected on first incremental compile, then track affected set on second compile', () => {
		const tempDir = createTempDir('compiler-incremental-affected-');
		const entryFile = path.join(tempDir, 'entry.ts');
		const outDir = path.join(tempDir, 'out');
		const tsBuildInfoFile = path.join(tempDir, '.tsbuildinfo');

		writeFile(entryFile, 'export const value: number = 1;');

		const compiler = new Compiler(
			[entryFile],
			[],
			createCompilerOptions(outDir, {
				incremental: true,
				tsBuildInfoFile
			})
		);

		compiler.compile();
		assert.strictEqual(compiler.isFileAffected(entryFile), true);

		compiler.emit(() => {
			return;
		});
		compiler.compile();

		assert.strictEqual(compiler.isFileAffected(entryFile), false);

		fs.rmSync(tempDir, { recursive: true, force: true });
	});

	it('should mark downstream file as affected after dependency file changes', () => {
		const tempDir = createTempDir('compiler-incremental-dependency-');
		const baseFile = path.join(tempDir, 'base.ts');
		const consumerFile = path.join(tempDir, 'consumer.ts');
		const outDir = path.join(tempDir, 'out');
		const tsBuildInfoFile = path.join(tempDir, '.tsbuildinfo');

		writeFile(baseFile, 'export interface Model { value: number; }');
		writeFile(consumerFile, 'import { Model } from "./base"; export const data: Model = { value: 1 };');

		const options = createCompilerOptions(outDir, {
			incremental: true,
			tsBuildInfoFile
		});

		const initialCompiler = new Compiler([baseFile, consumerFile], [], options);
		initialCompiler.compile();
		initialCompiler.emit(() => {
			return;
		});

		writeFile(baseFile, 'export interface Model { value: number; flag?: boolean; }');

		const incrementalCompiler = new Compiler([baseFile, consumerFile], [], options);
		incrementalCompiler.compile();

		assert.strictEqual(incrementalCompiler.isFileAffected(baseFile), true);
		assert.strictEqual(incrementalCompiler.isFileAffected(consumerFile), true);

		fs.rmSync(tempDir, { recursive: true, force: true });
	});

	it('should refresh program with updated source files from cache', () => {
		const tempDir = createTempDir('compiler-refresh-');
		const entryFile = path.join(tempDir, 'entry.ts');
		const outDir = path.join(tempDir, 'out');

		writeFile(entryFile, 'export const oldName = 1;');

		const compiler = new Compiler([entryFile], [], createCompilerOptions(outDir));
		compiler.compile();

		const updatedSource = ts.createSourceFile(
			entryFile,
			'export const newName = 2;',
			ts.ScriptTarget.Latest,
			true,
			ts.ScriptKind.TS
		);

		compiler.updateFile(entryFile, updatedSource);
		compiler.refreshProgram();

		const refreshed = compiler.getSourceFile(entryFile);
		assert.ok(refreshed);
		assert.strictEqual(refreshed?.text.includes('newName'), true);

		fs.rmSync(tempDir, { recursive: true, force: true });
	});

	describe('emit onEmitted hook', () => {
		it('invokes onEmitted with the default-writer path and final content', () => {
			const tempDir = createTempDir('compiler-emit-default-');
			const entryFile = path.join(tempDir, 'entry.ts');
			const outDir = path.join(tempDir, 'out');

			writeFile(entryFile, 'export const value: number = 1;');

			const compiler = new Compiler([entryFile], [], createCompilerOptions(outDir));
			compiler.compile();

			const calls: Array<{ absSrc: string; path: string; finalContent: string }> = [];
			compiler.emit(undefined, (absSrc, info) => {
				calls.push({ absSrc, ...info });
			});

			assert.strictEqual(calls.length, 1);
			assert.strictEqual(calls[0].absSrc, entryFile);
			assert.ok(calls[0].path.startsWith(outDir));
			assert.strictEqual(fs.existsSync(calls[0].path), true);
			assert.strictEqual(fs.readFileSync(calls[0].path, 'utf8'), calls[0].finalContent);

			fs.rmSync(tempDir, { recursive: true, force: true });
		});

		it('skips onEmitted when a custom writeFile returns void', () => {
			const tempDir = createTempDir('compiler-emit-custom-void-');
			const entryFile = path.join(tempDir, 'entry.ts');
			const outDir = path.join(tempDir, 'out');

			writeFile(entryFile, 'export const value: number = 1;');

			const compiler = new Compiler([entryFile], [], createCompilerOptions(outDir));
			compiler.compile();

			let onEmittedCalls = 0;
			compiler.emit(
				() => {
					return;
				},
				() => {
					onEmittedCalls++;
				}
			);

			assert.strictEqual(onEmittedCalls, 0);
			fs.rmSync(tempDir, { recursive: true, force: true });
		});

		it('invokes onEmitted when a custom writeFile opts in via { artifactPath, finalContent }', () => {
			const tempDir = createTempDir('compiler-emit-custom-optin-');
			const entryFile = path.join(tempDir, 'entry.ts');
			const outDir = path.join(tempDir, 'out');

			writeFile(entryFile, 'export const value: number = 1;');

			const compiler = new Compiler([entryFile], [], createCompilerOptions(outDir));
			compiler.compile();

			const calls: Array<{ absSrc: string; path: string; finalContent: string }> = [];
			compiler.emit(
				() => {
					return { artifactPath: '/virtual/foo.d.ets', finalContent: '<<final>>' };
				},
				(absSrc, info) => {
					calls.push({ absSrc, ...info });
				}
			);

			assert.strictEqual(calls.length, 1);
			assert.strictEqual(calls[0].absSrc, entryFile);
			assert.strictEqual(calls[0].path, '/virtual/foo.d.ets');
			assert.strictEqual(calls[0].finalContent, '<<final>>');

			fs.rmSync(tempDir, { recursive: true, force: true });
		});
	});
});


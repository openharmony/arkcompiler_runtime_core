/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

import * as ts from 'typescript';
import * as fs from 'node:fs';
import * as path from 'node:path';

import { logTscDiagnostics } from '../utils/LogTscDiagnostics';
import { LogLevel, Logger } from '../logger/Logger';
import { Extension } from '../utils/Extension';
import { VirtualFileHost, File, normalizePath, convertToDetsFileName, INTERNAL_PREFIX } from './VirtualFileHost';

/**
 * This is a workaround to make typechecker to be valid for arkts-sta builtins types.
 */
const PREDEFINED_GLOBAL_LIBS: File[] = [
  {
    meta: {
      filePath: `${INTERNAL_PREFIX}@global.static.arkts-builtins@.ts`,
      noChange: true,
      noWrite: true
    },
    content: `declare global { export type Any = any; export class Class {} export class MethodType {} }; export {};`
  }
];

export type ModuleNameResolver = (
  moduleNames: string[],
  containingFile: string
) => (ts.ResolvedModuleFull | undefined)[];

export function defaultCompilerOptions(): ts.CompilerOptions {
  return {
    target: ts.ScriptTarget.Latest,
    module: ts.ModuleKind.CommonJS,
    allowJs: true,
    checkJs: true
  };
}

export class Compiler {
  compilerProgram?: ts.Program;
  private builderProgram?: ts.EmitAndSemanticDiagnosticsBuilderProgram;
  private affectedFiles?: Set<string>;
  public rootFileNames: readonly string[];
  private compileFileNames: string[];
  private libFileNames: string[];
  private compilerOptions: ts.CompilerOptions;
  private compilerHost: ts.CompilerHost;
  private fileHost: VirtualFileHost;
  private defaultPrinter: ts.Printer;
  private sourceFilesCache: Map<string, ts.SourceFile>;
  private renderCache: WeakMap<ts.SourceFile, string>;
  public rootDir: string;

  constructor(
    rootFileNames: readonly string[],
    libFileNames: readonly string[],
    compilerOptions: ts.CompilerOptions,
    customResolveModuleNames?: ModuleNameResolver
  ) {
    this.rootFileNames = rootFileNames;
    this.rootDir = this.computeCommonSourceDirectory(rootFileNames);
    this.rootFileNames = rootFileNames.map((file) => normalizePath(file));
    this.libFileNames = libFileNames.map((file) => normalizePath(file));
    this.compilerOptions = compilerOptions;
    this.fileHost = new VirtualFileHost(compilerOptions);
    this.compilerHost = this.fileHost.getCompilerHost();
    this.defaultPrinter = ts.createPrinter();
    this.sourceFilesCache = new Map<string, ts.SourceFile>();
    this.renderCache = new WeakMap<ts.SourceFile, string>();
    if (customResolveModuleNames) {
      this.compilerHost.resolveModuleNames = customResolveModuleNames;
    }
    for (const lib of PREDEFINED_GLOBAL_LIBS) {
      this.fileHost.cache(lib.content, lib.meta);
      this.libFileNames.push(lib.meta.filePath);
    }
    this.compileFileNames = [...this.rootFileNames, ...this.libFileNames];
  }

  private computeCommonSourceDirectory(rootNames: readonly string[]): string {
    let commonComponents: string[] | undefined;
    for (const fileName of rootNames) {
      const absolute = path.resolve(fileName);
      const components = path.dirname(absolute).split(path.sep);
      if (!commonComponents) {
        commonComponents = components;
        continue;
      }
      const n = Math.min(commonComponents.length, components.length);
      let i = 0;
      for (; i < n; i++) {
        if (commonComponents[i].toLowerCase() !== components[i].toLowerCase()) {
          break;
        }
      }
      if (i === 0) {
        return '';
      }
      commonComponents.length = i;
      if (components.length < commonComponents.length) {
        commonComponents.length = components.length;
      }
    }
    if (!commonComponents) {
      return process.cwd();
    }
    return commonComponents.join(path.sep);
  }

  get program(): ts.Program {
    if (!this.compilerProgram) {
      throw new Error('Program is not initialized. Please call compile() first.');
    }
    return this.compilerProgram;
  }

  private set program(program: ts.Program) {
    this.compilerProgram = program;
  }

  compile(): void {
    if (this.compilerOptions.incremental) {
      // In incremental mode, diagnostics are collected from the builder so
      // that unchanged files reuse their cached semantic diagnostics from
      // `.tsbuildinfo`. Calling `getPreEmitDiagnostics` here would force a
      // full re-check on every file and defeat the incremental savings.
      this.compileIncremental();
      return;
    }

    this.compilerProgram = ts.createProgram(
      this.compileFileNames,
      this.compilerOptions ?? defaultCompilerOptions(),
      this.compilerHost
    );
    const diagnostics = ts.getPreEmitDiagnostics(this.compilerProgram!);
    logTscDiagnostics(diagnostics, LogLevel.ERROR);
  }

  /**
   * Create the program in incremental mode, leveraging tsc's `.tsbuildinfo`
   * to reuse parse/type-check results from the previous run and to detect
   * the set of files that were affected by changes since then.
   */
  private compileIncremental(): void {
    // Try to load the prior builder state from .tsbuildinfo (if any).
    let oldProgram: ts.EmitAndSemanticDiagnosticsBuilderProgram | undefined;
    try {
      oldProgram = ts.readBuilderProgram(
        this.compilerOptions,
        this.compilerHost as unknown as ts.ReadBuildProgramHost
      ) as ts.EmitAndSemanticDiagnosticsBuilderProgram | undefined;
    } catch {
      oldProgram = undefined;
    }

    this.builderProgram = ts.createIncrementalProgram({
      rootNames: this.compileFileNames,
      options: this.compilerOptions,
      host: this.compilerHost,
      configFileParsingDiagnostics: undefined,
      projectReferences: undefined
    });

    this.compilerProgram = this.builderProgram.getProgram();

    // Collect diagnostics through the builder API instead of
    // `getPreEmitDiagnostics` so that unchanged files reuse the cached
    // semantic diagnostics from `.tsbuildinfo` (these files won't be
    // re-checked). Syntactic / options / global diagnostics are always
    // recomputed because they are cheap.
    const diagnostics: ts.Diagnostic[] = [
      ...this.builderProgram.getConfigFileParsingDiagnostics(),
      ...this.builderProgram.getOptionsDiagnostics(),
      ...this.builderProgram.getSyntacticDiagnostics(),
      ...this.builderProgram.getGlobalDiagnostics()
    ];

    // If there is no prior `.tsbuildinfo`, every file is treated as affected.
    const hasPriorState = !!oldProgram;
    const affected = new Set<string>();
    let allAffected = !hasPriorState;

    while (true) {
      const next = this.builderProgram.getSemanticDiagnosticsOfNextAffectedFile();
      if (!next) {
        break;
      }
      diagnostics.push(...next.result);
      const target = next.affected as ts.SourceFile | ts.Program;
      if ((target as ts.SourceFile).fileName !== undefined) {
        affected.add(normalizePath((target as ts.SourceFile).fileName));
      } else {
        // Whole program affected - fall back to non-incremental emit.
        allAffected = true;
      }
    }
    this.affectedFiles = allAffected ? undefined : affected;
    logTscDiagnostics(diagnostics, LogLevel.ERROR);
  }

  /**
   * Returns true when the given root file changed (or its dependencies did)
   * and therefore needs to be re-emitted. When incremental tracking is not
   * available, every file is considered affected.
   */
  isFileAffected(fileName: string): boolean {
    if (!this.affectedFiles) {
      return true;
    }
    return this.affectedFiles.has(normalizePath(fileName));
  }

  /** All files tsc considered affected during the last incremental compile, or undefined for full builds. */
  getTscAffectedFiles(): ReadonlySet<string> | undefined {
    return this.affectedFiles;
  }

  /**
   * Cached export signature of a file as computed by tsc's BuilderProgram
   * (sha of the file's emitted declaration text). Returns undefined when no
   * builder program is available (non-incremental mode).
   */
  getSignatureOfFile(file: ts.SourceFile | string): string | undefined {
    const builder = this.builderProgram;
    if (!builder) {
      return undefined;
    }
    const sf = typeof file === 'string' ? this.getSourceFile(file) : file;
    if (!sf) {
      return undefined;
    }
    const api = builder as unknown as {
      getSignatureOfFile?: (sf: ts.SourceFile) => string | undefined;
    };
    return api.getSignatureOfFile?.(sf);
  }

  /**
   * Resolved absolute paths of all modules imported by `file`. Used to build
   * the per-file cache key and the forward import graph for affected-closure
   * computation.
   */
  getResolvedImports(file: ts.SourceFile): string[] {
    const out: string[] = [];
    const seen = new Set<string>();
    const resolved = (
      file as ts.SourceFile & {
        resolvedModules?: Map<string, ts.ResolvedModuleFull | undefined>;
      }
    ).resolvedModules;
    if (resolved && typeof resolved.forEach === 'function') {
      resolved.forEach((mod) => {
        if (mod?.resolvedFileName) {
          const abs = normalizePath(mod.resolvedFileName);
          if (!seen.has(abs)) {
            seen.add(abs);
            out.push(abs);
          }
        }
      });
    }
    return out;
  }

  /**
   * Persist the incremental build state to `.tsbuildinfo`. Safe to call when
   * incremental mode is disabled (it is a no-op in that case).
   */
  emitBuildInfo(): void {
    if (!this.builderProgram) {
      return;
    }
    // emitBuildInfo writes the .tsbuildinfo file via the compiler host's
    // writeFile callback (see VirtualFileHost).
    const emit = (this.builderProgram as unknown as { emitBuildInfo?: () => void }).emitBuildInfo;
    if (typeof emit === 'function') {
      emit.call(this.builderProgram);
    }
  }

  get printer(): ts.Printer {
    return this.defaultPrinter;
  }

  /**
   * Render a source file to the exact `.d.ets` text that gets written to disk:
   * the printed AST prefixed with the `'use static'` directive. This is the
   * single source of truth for the final emitted content — all writers and
   * content-hash computations must go through here so they stay byte-identical.
   *
   * The result is memoized per `SourceFile` instance: during the emit phase the
   * same file is rendered both to decide whether its output changed and to
   * actually write it, and `printFile` is the costly part. A `WeakMap` is used
   * (keyed on the AST instance) so cache entries are garbage-collected once an
   * old AST is no longer referenced, and `updateFile` evicts the entry when a
   * file is replaced.
   *
   * IMPORTANT: because the cache is keyed on the AST instance (not its
   * contents), it does NOT detect in-place mutations of the same `SourceFile`.
   * Only call this AFTER all stages have run and the AST is final/frozen
   * (i.e. during emit). Calling it while stages may still mutate a node in
   * place would cache stale text. Stages that need fresh output must replace
   * the file via {@link updateFile} (which evicts the entry) rather than
   * mutating the existing node.
   */
  renderDeclaration(sourceFile: ts.SourceFile): string {
    const cached = this.renderCache.get(sourceFile);
    if (cached !== undefined) {
      return cached;
    }
    const rendered = `'use static'\n${this.defaultPrinter.printFile(sourceFile)}`;
    this.renderCache.set(sourceFile, rendered);
    return rendered;
  }

  updateFile(fileName: string, content: ts.SourceFile): void {
    this.renderCache.delete(content);
    this.sourceFilesCache.set(normalizePath(fileName), content);
  }

  emit(
    writeFile?: (
      fileName: string,
      sourceFile: ts.SourceFile
    ) => void | { artifactPath?: string; finalContent?: string },
    onEmitted?: (absSrc: string, info: { path: string; finalContent: string }) => void,
    filesToEmit?: ReadonlySet<string>
  ): void {
    const defaultWriter = (
      fileName: string,
      sourceFile: ts.SourceFile
    ): { artifactPath: string; finalContent: string } => {
      const rootDir = this.rootDir;
      const outDir = this.compilerOptions.outDir ?? this.rootDir;
      const relativePath = path.relative(rootDir, fileName);
      const detsFileName = convertToDetsFileName(relativePath);
      const dEtsFilePath = path.join(outDir, detsFileName);
      const outPath = path.dirname(dEtsFilePath);
      const finalCode = this.renderDeclaration(sourceFile);
      if (!fs.existsSync(outPath)) {
        fs.mkdirSync(outPath, { recursive: true });
      }
      fs.writeFileSync(dEtsFilePath, finalCode, { encoding: 'utf8' });
      return { artifactPath: dEtsFilePath, finalContent: finalCode };
    };
    for (const fileName of this.rootFileNames) {
      // Skip files outside the affected set when incremental filtering is active.
      if (filesToEmit && !filesToEmit.has(normalizePath(fileName))) {
        continue;
      }
      const sourceFile = this.getSourceFile(fileName);
      if (!sourceFile) {
        continue;
      }
      if (writeFile) {
        const result = writeFile(fileName, sourceFile);
        if (onEmitted && result && result.artifactPath && typeof result.finalContent === 'string') {
          onEmitted(fileName, { path: result.artifactPath, finalContent: result.finalContent });
        }
      } else {
        const result = defaultWriter(fileName, sourceFile);
        if (onEmitted) {
          onEmitted(fileName, { path: result.artifactPath, finalContent: result.finalContent });
        }
      }
    }
    // Persist incremental state (no-op when incremental is disabled).
    this.emitBuildInfo();
  }

  /**
   * In incremental mode, skip emitting root files that were not affected by
   * any change since the previous run, provided their declaration output
   * already exists on disk.
   */
  private shouldEmit(fileName: string): boolean {
    if (!this.compilerOptions.incremental || !this.affectedFiles) {
      return true;
    }
    if (this.isFileAffected(fileName)) {
      return true;
    }
    const outDir = this.compilerOptions.outDir ?? this.rootDir;
    const relativePath = path.relative(this.rootDir, fileName);
    const detsFileName = convertToDetsFileName(relativePath);
    const dEtsFilePath = path.join(outDir, detsFileName);
    return !fs.existsSync(dEtsFilePath);
  }

  /**
   * Rebuild the in-memory ts.Program after AST has been mutated by stages.
   *
   * Reuses the previous Program as `oldProgram` so unchanged SourceFiles keep
   * their AST/typeCheck cache (matched by `SourceFile.version`).
   *
   * NOTE: This is NOT a builder/incremental rebuild:
   *   - it does NOT refresh `affectedFiles`
   *   - it does NOT update the .tsbuildinfo on disk
   *   - it only exists so that the type checker can see the new AST
   * If necessary, call this after stages have mutated SourceFiles via updateFile, before querying typeChecker again.
   */
  refreshProgram(): void {
    this.syncToFileHost();

    this.program = ts.createProgram(
      this.compileFileNames,
      this.compilerOptions,
      this.compilerHost,
      this.compilerProgram
    );
  }

  syncToFileHost(): void {
    for (const [fileName, sourceFile] of this.sourceFilesCache.entries()) {
      const printed = this.printer.printFile(sourceFile);
      this.fileHost.updateFile(fileName, printed);
    }
    this.sourceFilesCache.clear();
  }

  getSourceFile(fileName: string): ts.SourceFile | undefined {
    const resolved = normalizePath(fileName);
    if (this.sourceFilesCache.has(resolved)) {
      return this.sourceFilesCache.get(resolved);
    }
    return this.program.getSourceFile(fileName);
  }

  get typeChecker(): ts.TypeChecker {
    return this.program.getTypeChecker();
  }
}

export function parseConfigFile(tsconfig: string): ts.ParsedCommandLine | undefined {
  const host: ts.ParseConfigFileHost = ts.sys as ts.System & ts.ParseConfigFileHost;

  const diagnostics: ts.Diagnostic[] = [];

  let parsedConfigFile: ts.ParsedCommandLine | undefined;

  try {
    const oldUnrecoverableDiagnostic = host.onUnRecoverableConfigFileDiagnostic;
    host.onUnRecoverableConfigFileDiagnostic = (diagnostic: ts.Diagnostic): void => {
      diagnostics.push(diagnostic);
    };
    parsedConfigFile = ts.getParsedCommandLineOfConfigFile(tsconfig, {}, host);
    host.onUnRecoverableConfigFileDiagnostic = oldUnrecoverableDiagnostic;

    if (parsedConfigFile) {
      diagnostics.push(...ts.getConfigFileParsingDiagnostics(parsedConfigFile));
    }

    if (diagnostics.length > 0) {
      // Log all diagnostic messages and exit program.
      Logger.error('Failed to read config file.');
      logTscDiagnostics(diagnostics, LogLevel.ERROR);
      process.exit(-1);
    }
  } catch (error) {
    Logger.error('Failed to read config file: ' + error);
    process.exit(-1);
  }

  return parsedConfigFile;
}

export function getSourceFilesFromDir(dir: string): string[] {
  const resultFiles: string[] = [];

  for (const entity of fs.readdirSync(dir)) {
    const entityName = path.join(dir, entity);

    if (fs.statSync(entityName).isFile()) {
      const extension = path.extname(entityName);
      if (extension === ts.Extension.Ts || extension === ts.Extension.Tsx || extension === Extension.ETS) {
        resultFiles.push(entityName);
      }
    }

    if (fs.statSync(entityName).isDirectory()) {
      resultFiles.push(...getSourceFilesFromDir(entityName));
      continue;
    }
  }

  return resultFiles;
}

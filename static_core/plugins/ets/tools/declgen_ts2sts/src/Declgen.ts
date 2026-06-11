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

import * as path from 'node:path';
import * as fs from 'node:fs';
import * as ts from 'typescript';
import { performance } from 'perf_hooks';

import { defaultCompilerOptions, Compiler, ModuleNameResolver } from './compiler/Compiler';
import { Pipeline } from './Pipeline';

import { IStage, StageContext, StageTiming, buildInitialStageContext } from './stages/Stage';
import { CompileStage } from './stages/CompileStage';
import { InteropTagsStage } from './stages/interop-tags-stage/InteropTagsStage';
import { DeclarationConversionStage } from './stages/declaration-conversion-stage/DeclarationConversionStage';
import { RecheckStage } from './stages/recheck-stage/RecheckStage';
import { ConversionStage } from './stages/conversion-stage/ConversionStage';
import { SemanticFixStage } from './stages/semantic-fix-stage/SemanticFixStage';
import { ASTFixStage } from './stages/ast-fix-stage/ASTFixStage';
import { InteropTypesConversionStage } from './stages/interop-types-conversion-stage/InteropTypesConversionStage';
import { InitialAffectedAnalysisStage } from './stages/affected/InitialAffectedAnalysisStage';
import { AffectedAnalysisStage } from './stages/affected/AffectedAnalysisStage';
import { CacheLoadStage } from './stages/cache/CacheLoadStage';
import { CacheSaveStage } from './stages/cache/CacheSaveStage';
import { cacheStore, CacheSession } from './cache/CacheStore';
import { computeGlobalKey, PluginCacheKeyEntry } from './cache/CacheKey';
import { sha256 } from './cache/Hash';
import { DeclgenPlugin } from './plugin/Plugin';
import { applyPlugins } from './plugin/PluginRegistry';

export type CheckerResult = ts.Diagnostic[];

/**
 * Metadata declgen passes alongside the emitted content to a custom
 * {@link DeclgenWriteFile} callback.
 *
 * Wrapped in an object instead of being a positional argument so new fields
 * (size, affected, cached, …) can be added later without breaking callers.
 */
export interface DeclgenWriteFileMeta {
  /**
   * sha256 of the exact bytes passed in `content` (i.e. `'use static'` prefix
   * + printed AST). Equal to the value declgen records in
   * `manifest.files[*].emittedArtifact.sha256` and to `sha256(fs.readFileSync(artifactPath))`
   * after the callback writes the bytes verbatim.
   */
  hash: string;
}

/**
 * Callback signature accepted by {@link DeclgenResult.emit}.
 *
 * Receives the resolved source file name, the final `.d.ets` content (already
 * prefixed with `'use static'`), and per-file metadata. Return `{ artifactPath }`
 * to opt in to cache-side artifact tracking (declgen will stat the path + hash
 * the content); return `undefined` to skip recording (e.g. when piping to memory).
 */
export type DeclgenWriteFile = (
  fileName: string,
  content: string,
  meta: DeclgenWriteFileMeta
) => undefined | { artifactPath?: string };

export interface DeclgenResult {
  checkResult: CheckerResult;
  stageTimings: StageTiming[];
  /** Root files the pipeline considered affected at the end of the run. */
  affectedFiles: ReadonlySet<string>;
  /**
   * Emit the user-visible `.d.ets` outputs for the affected files of this run.
   *
   * Safe to call multiple times. Calling it on an obsolete `DeclgenResult`
   * (i.e. after another `run()` has executed on the same `Declgen`) still
   * emits using the AST as it currently lives in the shared `Compiler`, so
   * mixing stale results with new runs is not supported.
   */
  emit(writeFile?: DeclgenWriteFile): void;
}

export interface DeclgenFeatureOptions {
  enableInteropTypesFix?: boolean;
  removeReservedKeywordIdentifier?: boolean;
}
export interface DeclgenOptions {
  rootDir?: string;
  inputFiles: readonly string[];
  libFiles?: readonly string[];
  outDir?: string;
  features?: DeclgenFeatureOptions;
  /**
   * Custom directory for declgen's incremental cache. When set, enables both
   * cache reuse and tsc's `.tsbuildinfo` (managed by declgen at
   * `<cacheDir>/tsbuildinfo`). Defaults to `<outDir>/.declgen-cache/` when
   * `incremental` is true and no explicit path is provided.
   */
  cacheDir?: string;
  /** Disable cache use even when `cacheDir` is set. Mirrors `DECLGEN_NO_CACHE=1`. */
  noCache?: boolean;
  /** Enable incremental compilation backed by the declgen cache + tsc tsbuildinfo. */
  incremental?: boolean;
  /**
   * When true (default), the initial affected-analysis stage verifies the
   * previously-emitted `.d.ets` outputs (recorded by the last emit) are still
   * present and unmodified on disk. Files whose output is missing or has
   * different size/mtime+hash are marked affected so they are re-emitted.
   * Disable to skip the extra `stat`/`hash` calls.
   */
  verifyOutputs?: boolean;
  /**
   * Optional list of plugins that contribute extra stages to the pipeline.
   * Each plugin participates in the global cache key, so adding / removing
   * / version-bumping a plugin invalidates the cache automatically.
   * See `src/plugin/Plugin.ts`.
   */
  plugins?: readonly DeclgenPlugin[];
}

const DEFAULT_CACHE_DIR_NAME = '.declgen-cache';

export class Declgen {
  private readonly rootFiles: readonly string[];
  private readonly libFiles: readonly string[];
  private readonly compilerOptions: ts.CompilerOptions;
  private readonly compiler: Compiler;
  private readonly pipeline: Pipeline<IStage<unknown, unknown>[]>;
  private features: Required<DeclgenFeatureOptions>;
  private readonly cacheDir: string | undefined;
  private cacheSession: CacheSession | undefined;
  private readonly declgenVersion: string;
  /** The {@link DeclgenResult} returned by the most recent `run()`, kept so the deprecated `Declgen.emit()` shim can delegate. */
  private lastRunResult: DeclgenResult | undefined;
  private readonly pluginCacheKeyEntries: readonly PluginCacheKeyEntry[];

  constructor(
    declgenOptions: DeclgenOptions,
    compilerOptions?: ts.CompilerOptions,
    customResolveModuleNames?: ModuleNameResolver
  ) {
    const options = defaultCompilerOptions();
    this.rootFiles = declgenOptions.inputFiles;
    this.libFiles = declgenOptions.libFiles ?? [];
    this.features = Object.assign(
      {
        enableInteropTypesFix: false,
        removeReservedKeywordIdentifier: false
      },
      declgenOptions.features
    );
    this.declgenVersion = readDeclgenVersion();

    const noCache = declgenOptions.noCache || process.env.DECLGEN_NO_CACHE === '1';
    const cacheEnabled = !noCache && (declgenOptions.incremental || !!declgenOptions.cacheDir);
    this.cacheDir = cacheEnabled
      ? path.resolve(declgenOptions.cacheDir ?? path.join(declgenOptions.outDir ?? '.', DEFAULT_CACHE_DIR_NAME))
      : undefined;

    const tsBuildInfoFile = this.cacheDir ? path.join(this.cacheDir, 'tsbuildinfo') : undefined;

    this.compilerOptions = Object.assign({}, options, {
      declaration: true,
      emitDeclarationOnly: true,
      outDir: declgenOptions.outDir,
      ...(declgenOptions.rootDir ? { rootDir: declgenOptions.rootDir } : {}),
      ...(compilerOptions ?? {}),
      ...(cacheEnabled ? { incremental: true } : {}),
      ...(tsBuildInfoFile ? { tsBuildInfoFile } : {})
    });
    // Prevent the noemit of the passed compilerOptions from being true
    this.compilerOptions.noEmit = false;
    this.compilerOptions.experimentalDecorators = true;

    this.compiler = new Compiler(this.rootFiles, this.libFiles, this.compilerOptions, customResolveModuleNames);

    /**
     * Pipeline assembly:
     *   CompileStage
     *   → InitialAffectedAnalysisStage
     *   → CacheLoad(declaration)
     *   → DeclarationConversionStage   (only emits affected files — cache hits
     *                                    are non-affected, so safe on stale checker)
     *   → CacheSave(declaration)        (pure writer)
     *   → InteropTagsStage + AffectedAnalysis  (AST/JSDoc only; no checker)
     *   → RecheckStage                  (first checker barrier: refreshes both
     *                                    cache-load swapped SourceFiles AND newly
     *                                    written interop-tag JSDoc, before
     *                                    ConversionStage queries the checker)
     *   → ConversionStage + AffectedAnalysis
     *   → RecheckStage
     *   → SemanticFixStage + AffectedAnalysis
     *   → [InteropTypesConversionStage + AffectedAnalysis]
     *   → ASTFixStage + AffectedAnalysis
     */
    this.pipeline = new Pipeline();
    this.pipeline.add(new CompileStage());
    this.pipeline.add(new InitialAffectedAnalysisStage({ verifyOutputs: declgenOptions.verifyOutputs ?? true }));
    this.pipeline.add(new CacheLoadStage('declaration'));
    this.pipeline.add(new DeclarationConversionStage());
    this.pipeline.add(new CacheSaveStage('declaration'));
    this.pipeline.add(new InteropTagsStage());
    this.pipeline.add(new AffectedAnalysisStage('interopTags'));
    this.pipeline.add(new RecheckStage());
    this.pipeline.add(new ConversionStage(this.features.removeReservedKeywordIdentifier));
    this.pipeline.add(new AffectedAnalysisStage('conversion'));
    this.pipeline.add(new RecheckStage());
    this.pipeline.add(new SemanticFixStage());
    this.pipeline.add(new AffectedAnalysisStage('semanticFix'));
    if (this.features.enableInteropTypesFix) {
      this.pipeline.add(new InteropTypesConversionStage());
      this.pipeline.add(new AffectedAnalysisStage('interopTypes'));
    }
    this.pipeline.add(new ASTFixStage());
    this.pipeline.add(new AffectedAnalysisStage('astFix'));

    // Plugin-contributed stages are appended after the core pipeline is
    // assembled so anchor lookup operates on the final shape.
    const applied = applyPlugins(this.pipeline, declgenOptions.plugins ?? []);
    this.pluginCacheKeyEntries = applied.cacheKeyEntries;
  }

  /**
   * @deprecated Use plugin-contributed stages instead of these ad-hoc hooks
   */
  get addStage(): {
    before: (index: number, stage: IStage<unknown, unknown>) => Declgen;
    after: (index: number, stage: IStage<unknown, unknown>) => Declgen;
  } {
    return {
      before: (index: number, stage: IStage<unknown, unknown>): this => {
        this.pipeline.insertBefore(index, stage);
        return this;
      },
      after: (index: number, stage: IStage<unknown, unknown>): this => {
        this.pipeline.insertAfter(index, stage);
        return this;
      }
    };
  }

  /**
   * Execute the full declgen pipeline for the current set of input files and
   * return a {@link DeclgenResult} describing what was affected.
   *
   * The pipeline runs the following steps in order:
   * 1. **CompileStage** — drives `Compiler.compile()` so every downstream
   *    stage can access `compiler.program` and `typeChecker`.
   * 2. **InitialAffectedAnalysisStage** — determines which root files need to
   *    be re-processed (incremental mode) or marks all files affected (first
   *    run / no cache).
   * 3. **CacheLoad → transform stages → CacheSave** — applies declaration
   *    conversion, interop-tags, type conversion, semantic fixes, AST fixes,
   *    and any plugin-contributed stages; each stage is preceded by an
   *    `AffectedAnalysisStage` that propagates changes across the import graph.
   *
   * After the pipeline completes, the cache session is committed to disk.
   * Actual `.d.ets` output files are **not** written here; call
   * {@link DeclgenResult.emit} on the returned object to write them.
   *
   * Safe to call multiple times on the same `Declgen` instance (e.g. in a
   * watch-mode loop). Each call opens a fresh cache session and replaces the
   * previous {@link lastRunResult}.
   *
   * @returns A {@link DeclgenResult} containing the set of affected files,
   *   per-stage timing data, and an `emit` closure for writing outputs.
   */
  run(): DeclgenResult {
    const pipeline = this.pipeline;

    const initialContext: StageContext<undefined> = buildInitialStageContext(this.compiler);
    this.cacheSession = this.openCacheSession();
    initialContext.cacheSession = this.cacheSession;
    // Open the cache session BEFORE compile so CompileStage can rely on a
    // valid tsbuildinfo path being on disk.
    const finalContext = pipeline.run(initialContext);

    const affectedFiles: ReadonlySet<string> = collectAffected(finalContext);

    // Persist cache (artifacts + manifest) once the whole pipeline succeeds.
    if (this.cacheSession) {
      void this.cacheSession.commit();
    }

    const result: DeclgenResult = {
      checkResult: [],
      stageTimings: finalContext.stageTimings,
      affectedFiles,
      emit: this.buildEmit(affectedFiles, this.cacheSession)
    };
    this.lastRunResult = result;
    return result;
  }

  /**
   * @deprecated Call {@link DeclgenResult.emit} on the value returned by
   * {@link run} instead. This shim delegates to the most recent run and
   * exists only for backwards compatibility.
   */
  emit(writeFile?: DeclgenWriteFile): void {
    if (!this.lastRunResult) {
      throw new Error('Declgen.emit() called before run(). Call run() first and use the returned DeclgenResult.emit.');
    }
    this.lastRunResult.emit(writeFile);
  }

  /**
   * Build the `emit` closure stored on `DeclgenResult`. Captures the minimal
   * dependencies (compiler, cache session, affected set) instead of leaking
   * the whole `StageContext` to keep the public surface narrow.
   *
   * The files actually written are `affectedFiles` ∪ `missingEmitArtifact`:
   * a root file whose previous run never reached emit (no `emittedArtifact`
   * recorded in the cache) must be (re-)emitted even though the pipeline
   * itself was a cache hit — otherwise the user sees no `.d.ets` on disk.
   */
  private buildEmit(
    affectedFiles: ReadonlySet<string>,
    session: CacheSession | undefined
  ): (writeFile?: DeclgenWriteFile) => void {
    const compiler = this.compiler;
    const recordEmitted = session ? this.buildRecordEmitted(session) : undefined;
    const filesToEmit = this.buildFilesToEmit(affectedFiles, session);

    const emitWithWriteFile = (writeFile: DeclgenWriteFile) => {
      return (fileName: string, sourceFile: ts.SourceFile) => {
        const printed = compiler.printer.printFile(sourceFile);
        const finalCode = `'use static'\n${printed}`;
        const hash = sha256(finalCode);
        const result = writeFile(fileName, finalCode, { hash });
        if (result && typeof result === 'object' && result.artifactPath) {
          return { artifactPath: result.artifactPath, finalContent: finalCode };
        }
        return undefined;
      };
    }

    return (writeFile?: DeclgenWriteFile): void => {
      if (writeFile) {
        compiler.emit(
          emitWithWriteFile(writeFile),
          recordEmitted,
          filesToEmit
        );
      } else {
        compiler.emit(undefined, recordEmitted, filesToEmit);
      }

      // Flush any emit-side cache updates (artifact tracking metadata) to disk.
      if (session) {
        void session.commit();
      }
    };
  }

  /**
   * Returns a callback that records the emitted artifact fingerprint into the
   * cache session after each file is written to disk.
   */
  private buildRecordEmitted(
    session: CacheSession
  ): (absSrc: string, info: { path: string; finalContent: string }) => void {
    return (absSrc: string, info: { path: string; finalContent: string }): void => {
      try {
        const stat = fs.statSync(info.path);
        session.recordEmittedArtifact(absSrc, {
          path: info.path,
          sha256: sha256(info.finalContent),
          mtime: stat.mtimeMs,
          size: stat.size
        });
      } catch {
        // Path inaccessible — silently skip recording.
      }
    };
  }

  /**
   * Computes the set of root files to emit: the pipeline-affected files union
   * any root that has never had an emitted artifact recorded (covers the
   * "run() without emit()" case where a prior process built the cache but
   * never wrote outputs to disk).
   */
  private buildFilesToEmit(
    affectedFiles: ReadonlySet<string>,
    session: CacheSession | undefined
  ): Set<string> {
    const filesToEmit = new Set<string>(affectedFiles);
    if (!session) {
      return filesToEmit;
    }
    for (const root of this.compiler.rootFileNames) {
      if (!filesToEmit.has(root) && !session.getFile(root)?.emittedArtifact) {
        filesToEmit.add(root);
      }
    }
    return filesToEmit;
  }

  private openCacheSession(): CacheSession | undefined {
    if (!this.cacheDir) {
      return undefined;
    }
    const globalKey = computeGlobalKey({
      declgenVersion: this.declgenVersion,
      compilerOptions: this.compilerOptions,
      // Spread `this.features` (Required<DeclgenFeatureOptions>) so new flags
      // automatically participate in the cache key instead of silently being
      // ignored when someone forgets to update this call site.
      features: { ...this.features },
      pipelineSignature: this.pipeline.signature(),
      rootDir: this.compiler.rootDir,
      outDir: this.compilerOptions.outDir ?? this.compiler.rootDir,
      plugins: this.pluginCacheKeyEntries
    });
    return cacheStore.open({ cacheDir: this.cacheDir, globalKey });
  }
}

function readDeclgenVersion(): string {
  try {
    // eslint-disable-next-line @typescript-eslint/no-var-requires
    const pkg = require('../package.json') as { version?: string };
    return pkg.version ?? '0.0.0';
  } catch {
    return '0.0.0';
  }
}

function collectAffected(ctx: StageContext<unknown>): ReadonlySet<string> {
  const set = new Set<string>();
  ctx.transformTargetInfos.forEach((info) => {
    if (info.affected) {
      set.add(info.fileName);
    }
  });
  return set;
}

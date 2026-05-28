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

import * as ts from 'typescript';
import { Stage, StageContext } from '../Stage';
import { normalizePath } from '../../compiler/VirtualFileHost';
import { computeFileKey } from '../../cache/CacheKey';
import { computeInteropSignature } from '../../cache/InteropSignature';
import { sha256 } from '../../cache/Hash';
import type { EmittedArtifactInfo } from '../../cache/CacheStore';
import * as fs from 'node:fs';

export interface InitialAffectedAnalysisOptions {
  /** When true, verify previously-emitted user outputs are still on disk and untouched. */
  verifyOutputs?: boolean;
}

/**
 * Computes the entry affected set as `tsc.affected ∪ closure(interopSigChanged)`
 * and additionally pre-expands the set with the reverse-import closure so that
 * cross-file-reading stages (semantic-fix / interop-types-conversion) catch
 * type changes that flow across module boundaries (cross-read risk +
 * Step 6).
 *
 * Updates `ctx.transformTargetInfos[*].affected` in place.
 */
export class InitialAffectedAnalysisStage extends Stage<undefined, undefined> {
  override readonly cacheVersion = '1';
  private readonly verifyOutputs: boolean;

  constructor(options: InitialAffectedAnalysisOptions = {}) {
    super();
    this.verifyOutputs = options.verifyOutputs ?? true;
  }

  execute(ctx: StageContext<undefined>): StageContext<undefined> {
    const compiler = ctx.compiler;
    const cacheSession = ctx.cacheSession;

    // Without a cache session, every file is treated as affected (default behaviour).
    if (!cacheSession) {
      ctx.transformTargetInfos.forEach((info) => {
        info.affected = true;
      });
      return ctx;
    }

    const program = compiler.program;
    const rootFiles = Array.from(ctx.transformTargetInfos.keys());

    // Per-file inputs we need below.
    const sourceFileByPath = new Map<string, ts.SourceFile>();
    for (const f of rootFiles) {
      const sf = program.getSourceFile(f);
      if (sf) {
        sourceFileByPath.set(normalizePath(f), sf);
      }
    }

    // 1) Compute fileKey for each root, compare to cache.
    const tscAffected = compiler.getTscAffectedFiles();
    const affected = new Set<string>();
    // Files affected solely because the user-visible `.d.ets` is missing or
    // was tampered with. They will be re-emitted unconditionally, so the
    // narrow stage MUST keep them in `affected` end-to-end — otherwise we
    // would print a partially-transformed AST. See `forceFullPipeline`.
    const mustEmit = new Set<string>();

    for (const [absPath, sf] of sourceFileByPath) {
      const contentHash = sha256(sf.text);
      const depsSig = compiler.getSignatureOfFile(sf) ?? null;
      const resolvedImports = compiler.getResolvedImports(sf);
      const fileKey = computeFileKey({ contentHash, depsSignature: depsSig, resolvedImports });
      const interopSigHash = computeInteropSignature(sf.text) || null;

      const cached = cacheSession.getFile(absPath);

      let isAffected = false;
      // First run / no prior entry → must rebuild.
      if (!cached) {
        isAffected = true;
      } else if (cached.fileKey !== fileKey) {
        isAffected = true;
      } else if ((cached.interopSigHash ?? null) !== interopSigHash) {
        isAffected = true;
      } else if (tscAffected?.has(absPath)) {
        isAffected = true;
      } else if (!this.outputArtifactsPresent(cached)) {
        // The declgen-internal declaration cache artifact is missing on disk.
        // NOTE: this check does NOT cover the user-visible `outDir/*.d.ets` —
        // see `outputArtifactsPresent` for the precise scope.
        isAffected = true;
      } else if (this.verifyOutputs && !this.emittedArtifactStillValid(cached.emittedArtifact)) {
        // User-visible `.d.ets` was deleted or modified since last emit.
        // Clear the manifest entry so the emit side re-treats this root as
        // never-emitted, AND force the full transformation pipeline so the
        // re-emitted text is the post-ASTFix product, not whatever partial
        // AST a narrow-skipped intermediate stage would leave behind.
        cacheSession.clearEmittedArtifact(absPath);
        isAffected = true;
        mustEmit.add(absPath);
      }

      if (isAffected) {
        affected.add(absPath);
      }
    }

    // 2) Interop-sig closure: any file whose interop signature changed forces
    //    its reverse-import closure into affected (interop tags change exported
    //    function signatures from declgen's POV; importers must rebuild).
    const interopChanged = new Set<string>();
    for (const [absPath, sf] of sourceFileByPath) {
      const cur = computeInteropSignature(sf.text) || null;
      const prev = cacheSession.getFile(absPath)?.interopSigHash ?? null;
      if (cur !== prev) {
        interopChanged.add(absPath);
      }
    }

    const reverseImporters = buildReverseImportGraph(sourceFileByPath, compiler);
    // Publish the graph so every downstream AffectedAnalysisStage can reuse it
    // when expanding its per-stage "actually changed" set into a closure.
    ctx.reverseImporters = reverseImporters;

    const seed = new Set<string>([...affected, ...interopChanged]);
    const closure = computeReverseClosure(seed, reverseImporters);

    // 3) Apply to transformTargetInfos. `closure` = entry affected set
    //    expanded by reverse-import closure (any importer of a changed file
    //    must rebuild so cross-read stages observe the upstream change).
    ctx.transformTargetInfos.forEach((info, key) => {
      const abs = normalizePath(key);
      info.affected = closure.has(abs);
      if (mustEmit.has(abs)) {
        info.forceFullPipeline = true;
      }
    });

    return ctx;
  }

  /**
   * Checks whether the **declgen-internal cache artifact** written by the
   * previous run is still present on disk.
   *
   * Scope (what "artifact" means here):
   *   The single file `<cacheDir>/stages/declaration/<xx>/<sha256>.dets`
   *   produced by `CacheSaveStage('declaration')`. It is the only stage whose
   *   output text is persisted across processes; every other stage keeps its
   *   product only in memory. `CacheLoadStage('declaration')` re-parses this
   *   file back into a `SourceFile` and swaps it into the in-memory program,
   *   which is how we skip re-running `DeclarationConversionStage` on a hit.
   *
   *   Cache layout for reference:
   *     <cacheDir>/
   *       manifest.json
   *       tsbuildinfo
   *       stages/declaration/<xx>/<sha256>.dets   <-- the artifact this guards
   *
   * What this check explicitly does NOT cover:
   *   1. User-visible output files (e.g. `outDir/B.d.ets`). Those are written
   *      by `Compiler.emit`; the destination path is only computed at emit
   *      time and callers may supply their own `writeFile` callback that
   *      routes the bytes anywhere (memory, RPC, a different directory, …).
   *      declgen therefore cannot statically verify "the user's .d.ets is
   *      still on disk" from this stage.
   *   2. Intermediate per-stage ASTs for InteropTags / Conversion /
   *      SemanticFix / InteropTypesConversion / ASTFix. By design only the
   *      declaration stage is cached on disk; the rest are reproduced in
   *      memory each run.
   *
   * Known limitation arising from (1): if the user manually deletes
   * `outDir/B.d.ets` while the cache is intact, this function still returns
   * `true`, B is not re-marked affected, and `emit` will not rewrite it. This
   * cannot be fixed inside this stage alone — the planned remedy is to add an
   * `emittedArtifacts: { path, sha256, mtime }` record to the manifest, have
   * `Compiler.emit` (and any caller-supplied `writeFile` hook) populate it
   * after a successful write, and verify those entries here on the next run.
   */
  private outputArtifactsPresent(cached: { stageArtifacts: Record<string, string | undefined> }): boolean {
    const declArt = cached.stageArtifacts.declaration;
    if (!declArt) {
      return false;
    }
    try {
      return fs.existsSync(declArt);
    } catch {
      return false;
    }
  }

  /**
   * Verifies the user-visible `.d.ets` recorded by the previous emit still
   * exists and matches the recorded fingerprint. Returns `true` when:
   *   - no `emittedArtifact` was recorded (nothing to verify — `Declgen.emit`
   *     compensates by writing such files unconditionally on the next emit),
   *     OR
   *   - the file is present and (size+mtime match) OR (content sha256 matches).
   *
   * Cheap path first: size+mtime; only hash on mismatch.
   */
  private emittedArtifactStillValid(info: EmittedArtifactInfo | undefined): boolean {
    if (!info) {
      return true;
    }
    let stat: fs.Stats;
    try {
      stat = fs.statSync(info.path);
    } catch {
      return false;
    }
    if (stat.size === info.size && stat.mtimeMs === info.mtime) {
      return true;
    }
    try {
      const content = fs.readFileSync(info.path, 'utf8');
      return sha256(content) === info.sha256;
    } catch {
      return false;
    }
  }
}

function buildReverseImportGraph(
  rootFiles: Map<string, ts.SourceFile>,
  compiler: { getResolvedImports(sf: ts.SourceFile): string[] }
): Map<string, Set<string>> {
  const reverse = new Map<string, Set<string>>();
  for (const [absPath, sf] of rootFiles) {
    for (const dep of compiler.getResolvedImports(sf)) {
      let s = reverse.get(dep);
      if (!s) {
        s = new Set();
        reverse.set(dep, s);
      }
      s.add(absPath);
    }
  }
  return reverse;
}

export function computeReverseClosure(seed: Set<string>, reverse: Map<string, Set<string>>): Set<string> {
  const out = new Set<string>(seed);
  const stack = [...seed];
  while (stack.length) {
    const cur = stack.pop()!;
    const importers = reverse.get(cur);
    if (!importers) {
      continue;
    }
    for (const imp of importers) {
      if (!out.has(imp)) {
        out.add(imp);
        stack.push(imp);
      }
    }
  }
  return out;
}

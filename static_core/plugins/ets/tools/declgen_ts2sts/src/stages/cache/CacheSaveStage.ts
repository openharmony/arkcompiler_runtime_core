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

import { Stage, StageContext } from '../Stage';
import type { StageName } from '../../cache/CacheStore';
import { STAGES_WITH_TEXT_ARTIFACT } from '../../cache/CacheStore';
import { computeFileKey } from '../../cache/CacheKey';
import { computeInteropSignature } from '../../cache/InteropSignature';
import { sha256 } from '../../cache/Hash';
import { normalizePath } from '../../compiler/VirtualFileHost';

/**
 * For every affected file, write its current printed output as the cached
 * artifact for this stage and record both the artifact and the file's
 * (fileKey, interopSigHash) metadata in the cache session. Counterpart of
 * `CacheLoadStage` — they always come in pairs around a transformation stage.
 */
export class CacheSaveStage extends Stage<undefined, undefined> {
  override readonly cacheVersion = '1';

  constructor(private readonly stageName: StageName) {
    super();
    if (!STAGES_WITH_TEXT_ARTIFACT.has(stageName)) {
      throw new Error(`CacheSaveStage: stage '${stageName}' does not persist text artifacts.`);
    }
  }

  override get name(): string {
    return `CacheSaveStage(${this.stageName})`;
  }

  execute(ctx: StageContext<undefined>): StageContext<undefined> {
    const cache = ctx.cacheSession;
    if (!cache) {
      return ctx;
    }
    const compiler = ctx.compiler;
    const printer = compiler.printer;
    // Use the original (pre-transform) program to feed the per-file cache key:
    // it represents the input to this run, not the transformed output.
    const program = compiler.program;

    ctx.transformTargetInfos.forEach((info, absPath) => {
      if (!info.affected) {
        return;
      }
      const sf = compiler.getSourceFile(absPath);
      if (!sf) {
        return;
      }
      const text = printer.printFile(sf);

      // Recompute fileKey from the ORIGINAL source — needed because the
      // SourceFile in the compiler is now the post-declaration AST.
      const origSf = program.getSourceFile(absPath);
      const origText = origSf?.text ?? sf.text;
      const contentHash = sha256(origText);
      const fileKey = computeFileKey({
        contentHash,
        depsSignature: compiler.getSignatureOfFile(origSf ?? sf) ?? null,
        resolvedImports: compiler.getResolvedImports(origSf ?? sf)
      });
      const interopSigHash = computeInteropSignature(origText) || null;

      cache.putStageArtifact(normalizePath(absPath), this.stageName, text, {
        fileKey,
        interopSigHash,
        contentHash
      });
      cache.putStageOutputHash(normalizePath(absPath), this.stageName, sha256(text));
    });

    return ctx;
  }
}

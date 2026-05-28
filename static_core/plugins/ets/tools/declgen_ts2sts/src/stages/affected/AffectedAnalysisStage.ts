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
import { sha256 } from '../../cache/Hash';
import { computeReverseClosure } from './InitialAffectedAnalysisStage';

export interface AffectedAnalysisStageOptions {
  /**
   * When `true` (default), recompute the affected set after the preceding
   * transformation stage by reverse-closure of files whose stage-output hash
   * actually changed.
   *
   * When `false`, skip the affected-set rewrite and only record the per-file
   * output hashes for the next run. Useful when the stage that follows must
   * see the same affected set as the preceding transformation produced (e.g.
   * back-to-back debugging, or when temporarily disabling narrowing).
   */
  narrow?: boolean;
}

/**
 * Inserted after every transformation stage. For each file currently in the
 * affected set, re-prints its source and compares the sha to the hash
 * recorded for this stage in the previous run. The set of files whose hash
 * differs is then closure-expanded over the reverse-import graph; that
 * closure becomes the new affected set.
 *
 * Rationale: a local hash equality after stage S means "this file's
 * downstream behaviour for the next run, restricted to its own AST, is
 * unchanged". Closure-expanding over reverse imports lifts that local
 * argument to the cross-file case: a file F is safe to drop from affected
 * iff F itself is unchanged and none of F's direct imports changed either
 * (transitively, via the closure). This is the property cross-read stages
 * (SemanticFix / InteropTypesConversion) need.
 *
 * Files whose previous-run hash is absent are conservatively treated as
 * actually-changed so a missing baseline never leads to a wrong narrow.
 */
export class AffectedAnalysisStage extends Stage<unknown, unknown> {
  override readonly cacheVersion = '2';
  private readonly narrow: boolean;

  constructor(
    private readonly stageName: StageName,
    options: AffectedAnalysisStageOptions = {}
  ) {
    super();
    this.narrow = options.narrow ?? true;
  }

  override get name(): string {
    return `AffectedAnalysisStage(${this.stageName})`;
  }

  execute(ctx: StageContext<unknown>): StageContext<unknown> {
    const cacheSession = ctx.cacheSession;
    if (!cacheSession) {
      return ctx;
    }
    const compiler = ctx.compiler;
    const printer = compiler.printer;

    const actuallyChanged = new Set<string>();
    ctx.transformTargetInfos.forEach((info, absPath) => {
      if (!info.affected) {
        return;
      }
      const sf = compiler.getSourceFile(absPath);
      if (!sf) {
        return;
      }
      const text = printer.printFile(sf);
      const newHash = sha256(text);
      const prevHash = cacheSession.getFile(absPath)?.stageOutputHashes[this.stageName];
      // No baseline → conservatively treat as changed.
      if (!prevHash || prevHash !== newHash) {
        actuallyChanged.add(absPath);
      }
      cacheSession.putStageOutputHash(absPath, this.stageName, newHash);
    });

    if (!this.narrow) {
      return ctx;
    }

    const reverse = ctx.reverseImporters ?? new Map<string, Set<string>>();
    const closure = computeReverseClosure(actuallyChanged, reverse);

    ctx.transformTargetInfos.forEach((info, absPath) => {
      if (!info.affected) {
        return;
      }
      // Pinned by an upstream signal (e.g. user-visible artifact missing).
      // These files MUST emit at the end of the pipeline, so they cannot
      // be narrowed mid-pipeline regardless of their per-stage output hash.
      if (info.forceFullPipeline) {
        return;
      }
      info.affected = closure.has(absPath);
    });

    return ctx;
  }
}

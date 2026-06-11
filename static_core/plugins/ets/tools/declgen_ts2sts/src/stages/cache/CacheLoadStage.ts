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

import * as fs from 'node:fs';
import * as ts from 'typescript';
import { Stage, StageContext } from '../Stage';
import type { StageName } from '../../cache/CacheStore';
import { STAGES_WITH_TEXT_ARTIFACT } from '../../cache/CacheStore';
import { normalizePath } from '../../compiler/VirtualFileHost';

/**
 * For every NON-affected root file with a cached text artifact for this stage,
 * load the text from disk, re-parse it as a SourceFile and inject it into the
 * compiler. A `RecheckStage` must follow before any downstream stage queries
 * the type checker (cache-load reparse risk) — Pipeline static check enforces
 * this.
 */
export class CacheLoadStage extends Stage<undefined, undefined> {
  override readonly cacheVersion = '1';

  constructor(private readonly stageName: StageName) {
    super();
    if (!STAGES_WITH_TEXT_ARTIFACT.has(stageName)) {
      throw new Error(`CacheLoadStage: stage '${stageName}' does not persist text artifacts.`);
    }
  }

  override get name(): string {
    return `CacheLoadStage(${this.stageName})`;
  }

  execute(ctx: StageContext<undefined>): StageContext<undefined> {
    const cache = ctx.cacheSession;
    if (!cache) {
      return ctx;
    }
    const compiler = ctx.compiler;

    ctx.transformTargetInfos.forEach((info, absPath) => {
      if (info.affected) {
        return;
      }
      const cached = cache.getFile(normalizePath(absPath));
      const artifactPath = cached?.stageArtifacts[this.stageName];
      if (!artifactPath || !fs.existsSync(artifactPath)) {
        // Cache miss for an entry the affected analysis thought was safe;
        // promote to affected so it goes through the normal transform path.
        info.affected = true;
        return;
      }

      try {
        const text = fs.readFileSync(artifactPath, 'utf8');
        const sourceFile = ts.createSourceFile(absPath, text, ts.ScriptTarget.Latest, true);
        compiler.updateFile(absPath, sourceFile);
      } catch {
        info.affected = true;
      }
    });

    return ctx;
  }
}

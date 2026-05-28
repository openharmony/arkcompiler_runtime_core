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

import { Pipeline } from '../Pipeline';
import { IStage } from '../stages/Stage';
import { Stage, StageContext } from '../stages/Stage';
import { AffectedAnalysisStage } from '../stages/affected/AffectedAnalysisStage';
import { RecheckStage } from '../stages/recheck-stage/RecheckStage';
import { DeclgenPlugin, ExtensionPoint, PLUGIN_STAGE_ID_REGEX, PluginStageSpec } from './Plugin';
import type { PluginCacheKeyEntry } from '../cache/CacheKey';
import type { PluginStageName } from '../cache/CacheStore';

/**
 * Internal stage automatically inserted before any `global`-kind plugin
 * stage. Forces every known root file back into the affected set so the
 * downstream plugin sees the complete project, even when prior stages had
 * narrowed it down.
 */
class GlobalPreparationStage extends Stage<unknown, unknown> {
  override readonly cacheVersion = '1';

  constructor(private readonly pluginId: string) {
    super();
  }

  override get name(): string {
    return `GlobalPreparation(plugin:${this.pluginId})`;
  }

  execute(ctx: StageContext<unknown>): StageContext<unknown> {
    ctx.transformTargetInfos.forEach((info) => {
      info.affected = true;
    });
    return ctx;
  }
}

/**
 * Map an `ExtensionPoint` to the index of the trailing stage that defines
 * the anchor. Returns `-1` when the anchor isn't present in the pipeline
 * (e.g. `after-interop-types` without the `enableInteropTypesFix` feature).
 */
function findAnchorIndex(pipeline: Pipeline<IStage<unknown, unknown>[]>, point: ExtensionPoint): number {
  const target = ANCHOR_STAGE_NAME[point];
  const stages = pipeline.stagesSnapshot;
  for (let i = stages.length - 1; i >= 0; i--) {
    if (stages[i].name === target) {
      return i;
    }
  }
  return -1;
}

const ANCHOR_STAGE_NAME: Record<ExtensionPoint, string> = {
  'after-declaration': 'CacheSaveStage(declaration)',
  'after-interop-tags': 'AffectedAnalysisStage(interopTags)',
  'after-conversion': 'AffectedAnalysisStage(conversion)',
  'after-semantic-fix': 'AffectedAnalysisStage(semanticFix)',
  'after-interop-types': 'AffectedAnalysisStage(interopTypes)',
  'after-ast-fix': 'AffectedAnalysisStage(astFix)'
};

export interface AppliedPluginInfo {
  /** Stable, sorted entries to feed into `computeGlobalKey({ plugins })`. */
  cacheKeyEntries: PluginCacheKeyEntry[];
}

/**
 * Insert every plugin's stage(s) into the pipeline at the right anchor,
 * surround them with the barrier / narrowing stages dictated by their
 * `dependencyKind`, and return the metadata needed to fold the plugins
 * into the cache global key.
 *
 * Validation (fail-fast at construction time):
 *  - duplicate plugin ids are rejected;
 *  - unknown / disabled extension points are rejected;
 *  - non-conforming ids are rejected.
 */
export function applyPlugins(
  pipeline: Pipeline<IStage<unknown, unknown>[]>,
  plugins: readonly DeclgenPlugin[]
): AppliedPluginInfo {
  const seenIds = new Set<string>();
  const entries: PluginCacheKeyEntry[] = [];

  for (const plugin of plugins) {
    const specs = plugin.stages();
    const config = plugin.cacheKeyContribution ? plugin.cacheKeyContribution() : null;

    for (const spec of specs) {
      validateSpec(plugin.name, spec, seenIds);
      seenIds.add(spec.id);

      const insertion = insertSpec(pipeline, plugin.name, spec);
      entries.push({
        id: spec.id,
        version: spec.version,
        cacheVersion: spec.stage.cacheVersion ?? '0',
        config
      });
      void insertion;
    }
  }

  // Sort once for a stable cache-key contribution.
  entries.sort((a, b) => a.id.localeCompare(b.id));
  return { cacheKeyEntries: entries };
}

function validateSpec(pluginName: string, spec: PluginStageSpec, seenIds: ReadonlySet<string>): void {
  if (!PLUGIN_STAGE_ID_REGEX.test(spec.id)) {
    throw new Error(
      `Plugin '${pluginName}': stage id '${spec.id}' must match ${PLUGIN_STAGE_ID_REGEX} (reverse-DNS-style, no whitespace).`
    );
  }
  if (seenIds.has(spec.id)) {
    throw new Error(`Plugin '${pluginName}': duplicate stage id '${spec.id}'.`);
  }
  if (!spec.version) {
    throw new Error(`Plugin '${pluginName}': stage '${spec.id}' requires a non-empty 'version'.`);
  }
  if (!(ANCHOR_STAGE_NAME as Record<string, string>)[spec.anchor]) {
    throw new Error(`Plugin '${pluginName}': unknown extension point '${spec.anchor}' for stage '${spec.id}'.`);
  }
}

function insertSpec(
  pipeline: Pipeline<IStage<unknown, unknown>[]>,
  pluginName: string,
  spec: PluginStageSpec
): { indexAfterInsertion: number } {
  const anchorIdx = findAnchorIndex(pipeline, spec.anchor);
  if (anchorIdx < 0) {
    throw new Error(
      `Plugin '${pluginName}': extension point '${spec.anchor}' is not available in the current pipeline ` +
        `(stage '${spec.id}' cannot be inserted). Hint: check feature flags such as enableInteropTypesFix.`
    );
  }
  const position = spec.position ?? 'after';
  // We build the inserted sequence in order:
  //   [RecheckStage?]  pluginStage  [AffectedAnalysisStage(plugin:<id>)?]
  // Built around a `GlobalPreparationStage` when dependencyKind === 'global'.
  const inserted: IStage<unknown, unknown>[] = [];
  if (spec.requiresFreshChecker) {
    inserted.push(new RecheckStage());
  }
  if (spec.dependencyKind === 'global') {
    inserted.push(new GlobalPreparationStage(spec.id));
  }
  inserted.push(spec.stage);
  if (spec.dependencyKind === 'local-write' || spec.dependencyKind === 'cross-read') {
    const stageName = `plugin:${spec.id}` as PluginStageName;
    inserted.push(new AffectedAnalysisStage(stageName));
  }

  // Insert sequence preserving order. `insertBefore(anchorIdx + 1, …)`
  // is equivalent to "insert after anchor".
  let insertAt = position === 'before' ? anchorIdx : anchorIdx + 1;
  for (const stage of inserted) {
    pipeline.insertBefore(insertAt, stage);
    // Mark every plugin-injected stage as checker-safe for the validate
    // barrier walk. `requiresFreshChecker === true` already prepends a
    // `RecheckStage` (which terminates the walk) for cases that DO need a
    // fresh checker — for the others the author asserts the stage is safe.
    pipeline.registerCheckerSafeStageName(stage.name);
    insertAt++;
  }
  return { indexAfterInsertion: insertAt };
}

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

import { IStage } from '../stages/Stage';

/**
 * Describes how a plugin stage interacts with file state. Drives the framework's
 * automatic decisions about reverse-import-closure expansion and per-stage
 * affected narrowing.
 *
 *  - `local-readonly`  : pure observer. Does not mutate any SourceFile and does
 *                        not query the TypeChecker / read other files. The
 *                        framework does NOT append an `AffectedAnalysisStage`
 *                        after this kind because there is nothing to narrow.
 *  - `local-write`     : mutates the AST of files that are already in the
 *                        affected set, but never reads other files. Safe to
 *                        narrow on a per-file output-hash basis.
 *  - `cross-read`      : reads other files (through the TypeChecker, or by
 *                        walking imports). The reverse-import closure built by
 *                        `InitialAffectedAnalysisStage` (and reused by the
 *                        auto-inserted `AffectedAnalysisStage(plugin:<id>)`)
 *                        keeps any importer of a changed file in the affected
 *                        set, so cross-file signals reach this stage.
 *  - `global`          : has cross-file side effects (e.g. aggregates
 *                        diagnostics, writes a roll-up file). The framework
 *                        force-marks every root file as affected before the
 *                        stage runs and skips narrowing afterwards, so the
 *                        stage always sees the complete project.
 */
export type StageDependencyKind = 'local-readonly' | 'local-write' | 'cross-read' | 'global';

/**
 * Symbolic anchors inside the core pipeline. Plugins position themselves
 * relative to one of these instead of using raw indices, so the core can
 * reorder stages without breaking out-of-tree extensions.
 *
 * Each anchor is keyed by the *trailing* stage of a logical step:
 *  - `after-declaration`   → right after `CacheSaveStage(declaration)`
 *  - `after-interop-tags`  → right after `AffectedAnalysisStage(interopTags)`
 *  - `after-conversion`    → right after `AffectedAnalysisStage(conversion)`
 *  - `after-semantic-fix`  → right after `AffectedAnalysisStage(semanticFix)`
 *  - `after-interop-types` → right after `AffectedAnalysisStage(interopTypes)`
 *                            (only present when `enableInteropTypesFix` is on;
 *                             see `PluginAnchorError` below)
 *  - `after-ast-fix`       → right after `AffectedAnalysisStage(astFix)`
 */
export type ExtensionPoint =
  | 'after-declaration'
  | 'after-interop-tags'
  | 'after-conversion'
  | 'after-semantic-fix'
  | 'after-interop-types'
  | 'after-ast-fix';

export interface PluginStageSpec {
  /**
   * Globally unique stage id. Folded into the cache global key as
   * `plugin:<id>` and used as the manifest key for this stage's per-file
   * output hash. Convention: reverse-DNS, e.g. `com.acme.my-rule`.
   * Must match `/^[A-Za-z0-9._\-]+$/`.
   */
  id: string;
  /**
   * Plugin behaviour version. Bump on any change to the stage's output
   * contract. Any change invalidates the cache.
   */
  version: string;
  /** Stage instance. Its `cacheVersion` is also folded into the cache key. */
  stage: IStage<unknown, unknown>;
  dependencyKind: StageDependencyKind;
  anchor: ExtensionPoint;
  /** Whether to insert this stage before or after the anchor. Default 'after'. */
  position?: 'before' | 'after';
  /**
   * If true, the framework guarantees a `RecheckStage` runs immediately
   * before this plugin's stage. Set this whenever the stage reads the
   * TypeChecker and may have been preceded by a cache-load barrier.
   */
  requiresFreshChecker?: boolean;
}

export interface DeclgenPlugin {
  /** Human-readable plugin name. Used only for diagnostics. */
  readonly name: string;
  stages(): readonly PluginStageSpec[];
  /**
   * Optional opaque value folded into the global cache key. Any change to
   * the serialized form invalidates the whole cache. Must be JSON-stable.
   */
  cacheKeyContribution?(): unknown;
}

export const PLUGIN_STAGE_ID_REGEX = /^[A-Za-z0-9._\-]+$/;

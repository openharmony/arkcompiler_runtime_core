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

import { IStage, StageContext } from './stages/Stage';

type FirstInput<S extends readonly IStage<unknown, unknown>[]> = S[0] extends IStage<infer I, unknown> ? I : never;

type LastOutput<S extends readonly IStage<unknown, unknown>[]> = S extends readonly [...unknown[], infer Last]
  ? Last extends IStage<unknown, infer O>
    ? O
    : never
  : never;

export class Pipeline<Stages extends readonly IStage<unknown, unknown>[]> {
  private stages: IStage<unknown, unknown>[];
  private validated = false;
  /**
   * Plugin-injected stage names that the `CacheLoadStage → RecheckStage`
   * barrier walk in `validate()` may step over (the plugin author asserted
   * the stage is checker-safe by not setting `requiresFreshChecker`, in
   * which case the plugin registry registers it here).
   */
  private readonly pluginInjectedNames: Set<string> = new Set();

  constructor(stages?: Stages) {
    this.stages = stages ? [...stages] : [];
  }

  run(initial: StageContext<FirstInput<Stages>>): StageContext<LastOutput<Stages>> {
    this.validate();
    let ctx: StageContext<unknown> = initial;
    for (const stage of this.stages) {
      ctx = stage.run(ctx);
    }
    return ctx as StageContext<LastOutput<Stages>>;
  }

  add(stage: IStage<unknown, unknown>): this {
    this.stages.push(stage);
    this.validated = false;
    return this;
  }

  insertBefore(index: number, stage: IStage<unknown, unknown>): this {
    if (index < 0 || index > this.stages.length) {
      throw new RangeError(`Pipeline.insertBefore: index ${index} is out of range [0, ${this.stages.length}]`);
    }
    this.stages.splice(index, 0, stage);
    this.validated = false;
    return this;
  }

  insertAfter(index: number, stage: IStage<unknown, unknown>): this {
    if (index < 0 || index >= this.stages.length) {
      throw new RangeError(`Pipeline.insertAfter: index ${index} is out of range [0, ${this.stages.length - 1}]`);
    }
    this.stages.splice(index + 1, 0, stage);
    this.validated = false;
    return this;
  }

  get size(): number {
    return this.stages.length;
  }

  /**
   * Read-only snapshot of the current stage order. Exposed for the plugin
   * registry which needs to locate anchor stages by name; consumers must
   * NOT mutate the array.
   */
  get stagesSnapshot(): readonly IStage<unknown, unknown>[] {
    return this.stages;
  }

  /**
   * Register a stage name that is safe to appear between a `CacheLoadStage`
   * and the next `RecheckStage`. Used by the plugin registry when the
   * plugin author declares the stage does not query the TypeChecker.
   */
  registerCheckerSafeStageName(name: string): void {
    this.pluginInjectedNames.add(name);
    this.validated = false;
  }

  /**
   * Per-stage cacheVersion signature joined deterministically. Used by cache
   * key computation: bumping any stage's `cacheVersion` will invalidate the
   * shared global cache key.
   */
  signature(): string {
    return this.stages.map((s) => `${s.name}@${s.cacheVersion ?? '0'}`).join('|');
  }

  /**
   * Static-shape checks the pipeline requires:
   *  - `CompileStage` must be the first stage.
   *  - Each `CacheLoadStage` (which swaps cached `.dets` AST into the Program
   *    via `compiler.updateFile`, leaving the existing `TypeChecker` stale for
   *    those files) must be followed by a `RecheckStage` before any subsequent
   *    stage that queries the type checker. The matching `CacheSaveStage`,
   *    `DeclarationConversionStage` (only emits affected files, which are not
   *    cache hits, so safe on a stale checker) and any `AffectedAnalysisStage`
   *    are allowed between `CacheLoadStage` and the required `RecheckStage`.
   *    `CacheSaveStage` itself is a pure writer and imposes no barrier.
   *
   * Cheap to do at run-start; failures throw so misconfigured pipelines
   * fail fast.
   */
  validate(): void {
    if (this.validated) {
      return;
    }
    if (this.stages.length === 0) {
      this.validated = true;
      return;
    }

    const first = this.stages[0];
    if (first.name !== 'CompileStage') {
      throw new Error(`Pipeline: the first stage must be CompileStage, got '${first.name}'.`);
    }

    for (let i = 0; i < this.stages.length; i++) {
      const s = this.stages[i];
      if (!s.name.startsWith('CacheLoadStage')) {
        continue;
      }
      let sawRecheck = false;
      for (let j = i + 1; j < this.stages.length; j++) {
        const nm = this.stages[j].name;
        if (nm === 'RecheckStage') {
          sawRecheck = true;
          break;
        }
        if (
          nm.startsWith('CacheLoadStage') ||
          nm.startsWith('CacheSaveStage') ||
          nm.startsWith('DeclarationConversionStage') ||
          nm.startsWith('AffectedAnalysisStage') ||
          // InteropTagsStage only walks AST/JSDoc text; it never queries
          // the TypeChecker, so a stale checker is harmless to it.
          nm.startsWith('InteropTagsStage') ||
          // Plugin-injected stages explicitly marked checker-safe by the
          // registry (`local-readonly` / `local-write` kinds, or stages that
          // got their own `RecheckStage` barrier inserted upstream).
          this.pluginInjectedNames.has(nm) ||
          nm.startsWith('GlobalPreparation(')
        ) {
          continue;
        }
        throw new Error(
          `Pipeline: ${s.name} at index ${i} must be followed by a RecheckStage ` +
            `before the next type-checker-querying stage '${nm}' at index ${j}.`
        );
      }
      if (!sawRecheck) {
        throw new Error(
          `Pipeline: ${s.name} at index ${i} must be followed by a RecheckStage somewhere later in the pipeline.`
        );
      }
    }

    this.validated = true;
  }
}

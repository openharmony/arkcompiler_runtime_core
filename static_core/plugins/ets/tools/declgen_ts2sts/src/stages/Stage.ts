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
import { performance } from 'perf_hooks';
import { TraverserConstructor } from './Traverser';
import { Compiler } from '../compiler/Compiler';
import { normalizePath } from '../compiler/VirtualFileHost';
import { TransformationDiagnostic } from './TransformationDiagnostic';
import type { CacheSession } from '../cache/CacheStore';

export interface TransformTargetInfo {
  fileName: string;
  affected: boolean;
  /**
   * When true, `AffectedAnalysisStage` must NOT narrow this file out of the
   * affected set. Set when the file is affected for a reason that requires
   * the full pipeline to re-run end-to-end even though the per-stage output
   * hash would otherwise match the previous run — currently: the user-visible
   * `.d.ets` artifact was deleted/modified, so the file must traverse every
   * transformation stage so the final printed text is well-formed.
   */
  forceFullPipeline?: boolean;
}

export interface StageTiming {
  /** Stage class name. */
  name: string;
  /** Full path through the stage stack, e.g. `Outer/Inner`. */
  path: string;
  /** Wall-clock duration in milliseconds. */
  durationMs: number;
}

export class StageStack {
  private readonly stack: string[] = [];

  push(name: string): void {
    this.stack.push(name);
  }

  pop(): void {
    this.stack.pop();
  }

  /** Returns the name of the innermost active stage, or `undefined` if the stack is empty. */
  top(): string | undefined {
    return this.stack[this.stack.length - 1];
  }

  toArray(): readonly string[] {
    return this.stack;
  }
}

export interface StageContext<S = unknown> {
  compiler: Compiler;
  stageStack: StageStack;
  prevState: S;
  diagnostics: TransformationDiagnostic[];
  transformTargetInfos: Map<string, TransformTargetInfo>;
  stageTimings: StageTiming[];
  /**
   * Cache session for the current run, present only when the cache is enabled.
   * CacheLoad/Save stages, AffectedAnalysisStage and InitialAffectedAnalysisStage
   * all access it through here.
   */
  cacheSession?: CacheSession;
  /**
   * Reverse-import graph (`dependency → importers`) over the root files.
   * Populated by `InitialAffectedAnalysisStage` and reused by every
   * `AffectedAnalysisStage` to expand a per-stage "actually changed" set
   * into its cross-file closure. Absent on the no-cache / first-run path.
   */
  reverseImporters?: Map<string, Set<string>>;
}

/**
 * Convenience helpers to view / mutate the set of affected root files via the
 * existing `transformTargetInfos` map without duplicating state.
 */
export function getAffectedSet(ctx: StageContext<unknown>): Set<string> {
  const set = new Set<string>();
  ctx.transformTargetInfos.forEach((info) => {
    if (info.affected) {
      set.add(info.fileName);
    }
  });
  return set;
}

export function setAffected(ctx: StageContext<unknown>, fileName: string, affected: boolean): void {
  const info = ctx.transformTargetInfos.get(fileName);
  if (info) {
    info.affected = affected;
  }
}

export function buildInitialStageContext(compiler: Compiler): StageContext<undefined> {
  return {
    compiler,
    stageStack: new StageStack(),
    prevState: undefined,
    diagnostics: [],
    transformTargetInfos: new Map<string, TransformTargetInfo>(),
    stageTimings: []
  };
}

/**
 * A stage represents a scan or a transformation of the ast. It takes a StageContext as input and produces a new StageContext as output.
 */
export interface IStage<I = unknown, Q = unknown> {
  get name(): string;
  /**
   * Per-stage cache version. Combined into Pipeline's globalKey so changing
   * a stage's behaviour can selectively invalidate cached artefacts without
   * forcing a full wipe (the auto-generated DECLGEN_IMPL_HASH already covers
   * unbumped code changes, this is only used for explicit, finer-grained
   * invalidation by Step 5 / 6 stages).
   */
  readonly cacheVersion?: string;
  run(stageContext: StageContext<I>): StageContext<Q>;
}

export type StateFactory<S> = () => S;

export interface StageOptions {}

export abstract class Stage<I = unknown, Q = unknown> implements IStage<I, Q> {
  options: StageOptions;
  /** Override in subclasses to bump the per-stage cache signature. */
  readonly cacheVersion: string = '1';
  constructor(options?: StageOptions) {
    this.options = options || {};
  }

  get name(): string {
    return this.constructor.name;
  }

  abstract execute(stageContext: StageContext<I>): StageContext<Q>;

  run(stageContext: StageContext<I>): StageContext<Q> {
    stageContext.stageStack.push(this.name);
    const start = performance.now();
    let next: StageContext<Q>;
    try {
      next = this.execute(stageContext);
    } finally {
      const durationMs = performance.now() - start;
      stageContext.stageTimings.push({
        name: this.name,
        path: stageContext.stageStack.toArray().join('/'),
        durationMs
      });
      stageContext.stageStack.pop();
    }
    return next;
  }
}
export abstract class TransformationStage<I, Q, S> extends Stage<I, Q> {
  constructor(
    private traversers: TraverserConstructor<I, S>[],
    private traverserStateFactory: () => S,
    private traverserReturnStateFactory: (stageContext: StageContext<I>, localState: Map<string, S>) => Q,
    private readonly: boolean = false,
    options?: StageOptions
  ) {
    super(options);
  }

  execute(stageContext: StageContext<I>): StageContext<Q> {
    const compiler = stageContext.compiler;
    const program = compiler.program;
    const sourceFiles = this.collectAffectedSourceFiles(stageContext, compiler);
    const uniqueStateMap = new Map<string, S>();
    const visitors = this.buildVisitors(uniqueStateMap, program, stageContext);
    const result = ts.transform(sourceFiles, visitors, program.getCompilerOptions());
    if (!this.readonly) {
      this.applyTransformResults(result.transformed, compiler);
    }
    return { ...stageContext, prevState: this.traverserReturnStateFactory(stageContext, uniqueStateMap) };
  }

  private collectAffectedSourceFiles(stageContext: StageContext<I>, compiler: Compiler): ts.SourceFile[] {
    const sourceFiles: ts.SourceFile[] = [];
    stageContext.transformTargetInfos.forEach((info) => {
      if (info.affected) {
        const sf = compiler.getSourceFile(info.fileName);
        if (sf) {
          sourceFiles.push(sf);
        }
      }
    });
    return sourceFiles;
  }

  private buildVisitors(
    uniqueStateMap: Map<string, S>,
    program: ts.Program,
    stageContext: StageContext<I>
  ): ts.TransformerFactory<ts.SourceFile>[] {
    return this.traversers.map((traverser) => (context: ts.TransformationContext) =>
      this.makeTransformer(traverser, context, program, stageContext, uniqueStateMap)
    );
  }

  private makeTransformer(
    traverser: TraverserConstructor<I, S>,
    context: ts.TransformationContext,
    program: ts.Program,
    stageContext: StageContext<I>,
    uniqueStateMap: Map<string, S>
  ): ts.Transformer<ts.SourceFile> {
    return (sourceFile: ts.SourceFile): ts.SourceFile => {
      if (!uniqueStateMap.has(sourceFile.fileName)) {
        uniqueStateMap.set(sourceFile.fileName, this.traverserStateFactory());
      }
      const traverserInstance = new traverser(context, program.getTypeChecker(), {
        stageContext,
        localState: uniqueStateMap.get(sourceFile.fileName)!
      });
      return traverserInstance.traverse(sourceFile) as ts.SourceFile;
    };
  }

  private applyTransformResults(transformed: ts.Node[], compiler: Compiler): void {
    for (const sourceFile of transformed) {
      if (ts.isSourceFile(sourceFile)) {
        compiler.updateFile(sourceFile.fileName, sourceFile);
      }
    }
  }
}

type StageInput<Stages extends readonly IStage<unknown, unknown>[]> = Stages extends readonly [
  IStage<infer Input, unknown>,
  ...IStage<unknown, unknown>[]
]
  ? Input
  : unknown;

type StageOutput<Stages extends readonly IStage<unknown, unknown>[]> = Stages extends readonly [
  ...IStage<unknown, unknown>[],
  IStage<unknown, infer Output>
]
  ? Output
  : unknown;

export class StageList<Stages extends readonly IStage<unknown, unknown>[]>
  implements IStage<StageInput<Stages>, StageOutput<Stages>>
{
  private readonly stages: readonly [...Stages];
  private readonly stageName: string;

  protected static makeListName(prefix: string, stages: readonly IStage<unknown, unknown>[]): string {
    return `${prefix}[${stages.map((s) => s.name).join(', ')}]`;
  }

  constructor(stages: readonly [...Stages], name?: string) {
    this.stages = stages;
    this.stageName = name ?? StageList.makeListName('StageList', stages);
  }

  get name(): string {
    return this.stageName;
  }

  run(stageContext: StageContext<StageInput<Stages>>): StageContext<StageOutput<Stages>> {
    stageContext.stageStack.push(this.name);
    const start = performance.now();
    let currentContext: StageContext<unknown> = stageContext;
    try {
      for (const stage of this.stages) {
        currentContext = stage.run(currentContext);
      }
    } finally {
      const durationMs = performance.now() - start;
      stageContext.stageTimings.push({
        name: this.name,
        path: stageContext.stageStack.toArray().join('/'),
        durationMs
      });
      stageContext.stageStack.pop();
    }
    return currentContext as StageContext<StageOutput<Stages>>;
  }
}

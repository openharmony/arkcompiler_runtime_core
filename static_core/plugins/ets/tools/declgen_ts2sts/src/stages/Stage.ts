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
import { TraverserConstructor } from './Traverser';
import { Compiler } from '../compiler/Compiler';
import { resolvePath } from '../compiler/VirtualFileHost';
import { TransformationDiagnostic } from './TransformationDiagnostic';

export interface TransformTargetInfo {
  fileName: string;
  affected: boolean;
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
}

export function buildInitialStageContext(
  compiler: Compiler,
  transformTargets?: readonly string[]
): StageContext<undefined> {
  const transformedFiles = transformTargets
    ? transformTargets.map((file) => resolvePath(file))
    : compiler.rootFileNames;

  const transformTargetInfos = new Map<string, TransformTargetInfo>();
  for (const file of transformedFiles) {
    transformTargetInfos.set(file, { fileName: file, affected: true });
  }

  return {
    compiler,
    stageStack: new StageStack(),
    prevState: undefined,
    diagnostics: [],
    transformTargetInfos
  };
}

/**
 * A stage represents a scan or a transformation of the ast. It takes a StageContext as input and produces a new StageContext as output.
 */
export interface IStage<I = unknown, Q = unknown> {
  get name(): string;
  run(stageContext: StageContext<I>): StageContext<Q>;
}

export type StateFactory<S> = () => S;

export interface StageOptions {}

export abstract class Stage<I = unknown, Q = unknown> implements IStage<I, Q> {
  options: StageOptions;
  constructor(options?: StageOptions) {
    this.options = options || {};
  }

  get name(): string {
    return this.constructor.name;
  }

  abstract execute(stageContext: StageContext<I>): StageContext<Q>;

  run(stageContext: StageContext<I>): StageContext<Q> {
    stageContext.stageStack.push(this.name);
    const next = this.execute(stageContext);
    stageContext.stageStack.pop();
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
    const program = stageContext.compiler.program;
    const sourceFiles: ts.SourceFile[] = [];
    stageContext.transformTargetInfos.forEach((info) => {
      if (info.affected) {
        const sourceFile = compiler.getSourceFile(info.fileName);
        if (sourceFile) {
          sourceFiles.push(sourceFile);
        }
      }
    });

    const uniqueStateMap = new Map<string, S>();

    const visiters = this.traversers.map((traverser) => {
      return (context: ts.TransformationContext): ts.Transformer<ts.SourceFile> => {
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
      };
    });

    const result = ts.transform(sourceFiles, visiters, program.getCompilerOptions());
    if (!this.readonly) {
      result.transformed.forEach((sourceFile) => {
        if (ts.isSourceFile(sourceFile)) {
          compiler.updateFile(sourceFile.fileName, sourceFile);
        }
      });
    }

    const nextContext: StageContext<Q> = {
      ...stageContext,
      prevState: this.traverserReturnStateFactory(stageContext, uniqueStateMap)
    };

    return nextContext;
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
    let currentContext: StageContext<unknown> = stageContext;

    for (const stage of this.stages) {
      currentContext = stage.run(currentContext);
    }

    const next = currentContext as StageContext<StageOutput<Stages>>;
    stageContext.stageStack.pop();
    return next;
  }
}

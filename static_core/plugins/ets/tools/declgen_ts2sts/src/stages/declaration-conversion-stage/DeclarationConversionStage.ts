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
import { Stage, StageContext, getAffectedSet } from '../Stage';

/**
 * This stage is to convert all files into declaration files.
 */
export class DeclarationConversionStage extends Stage<undefined, undefined> {
  constructor() {
    super();
  }

  execute(stageContext: StageContext<undefined>): StageContext<undefined> {
    const compiler = stageContext.compiler;
    const program = stageContext.compiler.program;

    // Only the files marked affected go through declaration emit; the rest
    // were already injected from cache by an upstream `CacheLoadStage`.
    const affected = getAffectedSet(stageContext);
    const rootFiles = compiler.rootFileNames;

    // Inject already-declaration root files into the compiler's virtual FS.
    for (const file of rootFiles) {
      const sf = program.getSourceFile(file);
      if (sf?.isDeclarationFile && affected.has(sf.fileName)) {
        compiler.updateFile(sf.fileName, sf);
      }
    }

    // Hoist the per-file capture callback so the transformer factory stays flat.
    const captureIntoCompiler = (sf: ts.SourceFile): ts.SourceFile => {
      if (affected.has(sf.fileName)) {
        compiler.updateFile(sf.fileName, sf);
      }
      return sf;
    };

    const afterDeclarations: ts.CustomTransformerFactory[] = [
      (ctx: ts.TransformationContext): ts.CustomTransformer => {
        void ctx;
        return { transformSourceFile: captureIntoCompiler, transformBundle: (b: ts.Bundle) => b };
      }
    ];

    // Per-file emit so we don't waste work on non-affected files. Passing
    // `targetSourceFile` to `program.emit` restricts emission to one file
    // (tsc does NOT use BuilderProgram-style skipping here, but the cost
    // becomes proportional to |affected| instead of |all root files|).
    for (const file of rootFiles) {
      const sf = program.getSourceFile(file);
      if (!sf || sf.isDeclarationFile || !affected.has(sf.fileName)) {
        continue;
      }
      program.emit(sf, undefined, undefined, true, { before: [], after: [], afterDeclarations });
    }

    return { ...stageContext, prevState: undefined };
  }
}

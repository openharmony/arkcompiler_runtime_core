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

    const rootFiles = program.getRootFileNames();
    const declarationFiles = rootFiles
      .filter((file) => {
        const sourceFile = program.getSourceFile(file);
        return sourceFile?.isDeclarationFile;
      })
      .map((file) => program.getSourceFile(file)!) as ts.SourceFile[];

    declarationFiles.forEach((sourceFile) => {
      if (ts.isSourceFile(sourceFile)) {
        compiler.updateFile(sourceFile.fileName, sourceFile);
      }
    });

    const afterDeclarations: ts.CustomTransformerFactory[] = [
      (ctx: ts.TransformationContext): ts.CustomTransformer => {
        void ctx;
        return {
          transformSourceFile: (sourceFile: ts.SourceFile): ts.SourceFile => {
            compiler.updateFile(sourceFile.fileName, sourceFile);
            return sourceFile;
          },
          transformBundle: (bundle: ts.Bundle): ts.Bundle => bundle
        };
      }
    ];

    program.emit(undefined, undefined, undefined, true, {
      before: [],
      after: [],
      afterDeclarations
    });

    const nextContext: StageContext<undefined> = {
      ...stageContext,
      prevState: undefined
    };

    return nextContext;
  }
}

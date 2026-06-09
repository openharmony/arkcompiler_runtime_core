/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

import { defaultCompilerOptions, Compiler, ModuleNameResolver } from './compiler/Compiler';
import { Pipeline } from './Pipeline';

import { IStage, StageContext, buildInitialStageContext } from './stages/Stage';
import { InteropTagsStage } from './stages/interop-tags-stage/InteropTagsStage';
import { DeclarationConversionStage } from './stages/declaration-conversion-stage/DeclarationConversionStage';
import { RecheckStage } from './stages/recheck-stage/RecheckStage';
import { ConversionStage } from './stages/conversion-stage/ConversionStage';
import { SemanticFixStage } from './stages/semantic-fix-stage/SemanticFixStage';
import { ASTFixStage } from './stages/ast-fix-stage/ASTFixStage';
import { InteropTypesConversionStage } from './stages/interop-types-conversion-stage/InteropTypesConversionStage';

export type CheckerResult = ts.Diagnostic[];

export interface DeclgenResult {
  checkResult: CheckerResult;
}

export interface DeclgenFeatureOptions {
  enableInteropTypesFix?: boolean;
  removeReservedKeywordIdentifier?: boolean;
}
export interface DeclgenOptions {
  rootDir?: string;
  inputFiles: readonly string[];
  libFiles?: readonly string[];
  outDir?: string;
  features?: DeclgenFeatureOptions;
  /**
   * Enable incremental compilation backed by tsc's `.tsbuildinfo`.
   * When enabled, the second and subsequent runs reuse parse / type-check
   * results and skip re-emitting `.d.ets` outputs for files that were not
   * affected by any changes since the previous run.
   */
  incremental?: boolean;
  /**
   * Custom path to the `.tsbuildinfo` file. When omitted, tsc derives a
   * default location from `outDir`.
   */
  tsBuildInfoFile?: string;
}

export class Declgen {
  private readonly rootFiles: readonly string[];
  private readonly libFiles: readonly string[];
  private readonly compilerOptions: ts.CompilerOptions;
  private readonly compiler: Compiler;
  private readonly pipeline: Pipeline<IStage<unknown, unknown>[]>;
  private features: Required<DeclgenFeatureOptions>;

  constructor(
    declgenOptions: DeclgenOptions,
    compilerOptions?: ts.CompilerOptions,
    customResolveModuleNames?: ModuleNameResolver
  ) {
    const options = defaultCompilerOptions();
    this.rootFiles = declgenOptions.inputFiles;
    this.libFiles = declgenOptions.libFiles ?? [];
    this.features = Object.assign(
      {
        enableInteropTypesFix: false,
        removeReservedKeywordIdentifier: false
      },
      declgenOptions.features
    );
    this.compilerOptions = Object.assign({}, options, {
      declaration: true,
      emitDeclarationOnly: true,
      outDir: declgenOptions.outDir,
      ...(declgenOptions.rootDir ? { rootDir: declgenOptions.rootDir } : {}),
      ...(compilerOptions ?? {}),
      ...(declgenOptions.incremental ? { incremental: true } : {}),
      ...(declgenOptions.tsBuildInfoFile ? { tsBuildInfoFile: declgenOptions.tsBuildInfoFile } : {})
    });
    // Prevent the noemit of the passed compilerOptions from being true
    this.compilerOptions.noEmit = false;
    this.compilerOptions.experimentalDecorators = true;

    this.compiler = new Compiler(this.rootFiles, this.libFiles, this.compilerOptions, customResolveModuleNames);

    /**
     * The transformation process contains many stages.
     * 1. Convert all files into declaration.
     * 2. Convert statements which contains @noninterop or @interop tags.
     * 3. Conversion stages.
     * 4. Semantic fix stages. Fix semantic that are not supported in static arkts.
     * 5. Interop types conversion stage.
     * 6. AST fix stage. Fix the AST structure to make it more suitable for declaration file generation.
     */
    this.pipeline = new Pipeline();
    this.pipeline.add(new DeclarationConversionStage());
    this.pipeline.add(new InteropTagsStage());
    this.pipeline.add(new RecheckStage());
    this.pipeline.add(new ConversionStage(this.features.removeReservedKeywordIdentifier));
    this.pipeline.add(new RecheckStage());
    this.pipeline.add(new SemanticFixStage());
    if (this.features.enableInteropTypesFix) {
      this.pipeline.add(new InteropTypesConversionStage());
    }
    this.pipeline.add(new ASTFixStage());
  }

  get addStage(): {
    before: (index: number, stage: IStage<unknown, unknown>) => Declgen;
    after: (index: number, stage: IStage<unknown, unknown>) => Declgen;
  } {
    return {
      before: (index: number, stage: IStage<unknown, unknown>): this => {
        this.pipeline.insertBefore(index, stage);
        return this;
      },
      after: (index: number, stage: IStage<unknown, unknown>): this => {
        this.pipeline.insertAfter(index, stage);
        return this;
      }
    };
  }

  run(): DeclgenResult {
    const pipeline = this.pipeline;

    this.compiler.compile();

    const initialContext: StageContext<undefined> = buildInitialStageContext(this.compiler, this.rootFiles);
    pipeline.run(initialContext);
    return {} as DeclgenResult;
  }

  emit(writeFile?: (fileName: string, content: string) => void): void {
    if (writeFile) {
      this.compiler.emit((fileName: string, sourceFile: ts.SourceFile) => {
        const printer = this.compiler.printer;
        const printed = printer.printFile(sourceFile);
        const finalCode = `'use static'\n${printed}`;
        writeFile(fileName, finalCode);
      });
    } else {
      this.compiler.emit();
    }
  }
}

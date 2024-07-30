/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
import * as path from 'path';

import { defaultCompilerOptions, getSourceFilesFromDir, compile, parseConfigFile } from './compiler/Compiler';
import type { DeclgenCLIOptions } from './cli/DeclgenCLI';
import { Logger } from '../utils/logger/Logger';

import { Transformer } from './ASTTransformer';
import { Autofixer } from './ASTAutofixer';
import { Checker } from './ASTChecker';
import { Extension } from './utils/Extension';

export type CheckerResult = ts.Diagnostic[];

export interface DeclgenResult {
  emitResult: ts.EmitResult;
  checkResult: CheckerResult;
}

export class Declgen {
  private readonly sourceFileMap: Map<string, ts.SourceFile>;
  private readonly hookedHost: ts.CompilerHost;
  private readonly rootFiles: readonly string[];
  private readonly compilerOptions: ts.CompilerOptions;

  constructor(declgenOptions: DeclgenCLIOptions) {
    const { rootNames, options } = Declgen.parseDeclgenOptions(declgenOptions);

    this.rootFiles = rootNames;

    this.sourceFileMap = new Map<string, ts.SourceFile>();

    this.compilerOptions = Object.assign(options, {
      declaration: true,
      emitDeclarationOnly: true,
      outDir: declgenOptions.outDir
    });

    this.hookedHost = Declgen.createHookedCompilerHost(this.sourceFileMap, this.compilerOptions);
  }

  run(): DeclgenResult {

    /**
     * First compilation with the hooked CompilerHost:
     * collect the SourceFiles after transformation to the hooked Map
     */
    let program = this.recompile();

    const emitResult = program.emit(undefined, undefined, undefined, true, {
      before: [],
      after: [],
      afterDeclarations: [
        (ctx: ts.TransformationContext): ts.CustomTransformer => {
          const typeChecker = program.getTypeChecker();
          const autofixer = new Autofixer(typeChecker, ctx);
          const transformer = new Transformer(ctx, this.sourceFileMap, [autofixer.fixNode.bind(autofixer)]);

          return transformer.createCustomTransformer();
        }
      ]
    });

    /**
     * Second compilation with the hooked CompilerHost:
     * use transformed source files and perform type check upon them
     */
    program = this.recompile();

    const checkResult = this.checkProgram(program);

    return {
      emitResult: emitResult,
      checkResult: checkResult
    };
  }

  private recompile(): ts.Program {
    return compile(this.rootFiles, this.compilerOptions, this.hookedHost);
  }

  private checkProgram(program: ts.Program): CheckerResult {
    const checker = new Checker(program.getTypeChecker());

    void checker;
    void this;

    return [];
  }

  private static createHookedCompilerHost(
    sourceFileMap: Map<string, ts.SourceFile>,
    compilerOptions: ts.CompilerOptions
  ): ts.CompilerHost {
    const host = ts.createCompilerHost(compilerOptions);
    const fallbackGetSourceFile = host.getSourceFile;
    const fallbackWriteFile = host.writeFile;

    return Object.assign(host, {
      getSourceFile(
        fileName: string,
        languageVersionOrOptions: ts.ScriptTarget | ts.CreateSourceFileOptions,
        onError?: (message: string) => void,
        shouldCreateNewSourceFile?: boolean
      ): ts.SourceFile | undefined {
        return (
          sourceFileMap.get(fileName) ??
          fallbackGetSourceFile(fileName, languageVersionOrOptions, onError, shouldCreateNewSourceFile)
        );
      },
      writeFile(
        fileName: string,
        text: string,
        writeByteOrderMark: boolean,
        onError?: (message: string) => void,
        sourceFiles?: readonly ts.SourceFile[],
        data?: ts.WriteFileCallbackData
      ) {
        const parsedPath = path.parse(fileName);
        fallbackWriteFile(

          /*
           * Since `.d` part of `.d.ts` extension is a part of the parsedPath.name,
           * use `Extension.Ets` for output file name generation.
           */
          path.join(parsedPath.dir, `${parsedPath.name}${Extension.STS}`),
          text,
          writeByteOrderMark,
          onError,
          sourceFiles,
          data
        );
      }
    });
  }

  static parseDeclgenOptions(opts: DeclgenCLIOptions): ts.CreateProgramOptions {
    const parsedConfigFile = opts.tsconfig ? parseConfigFile(opts.tsconfig) : undefined;

    if (parsedConfigFile) {
      return {
        rootNames: parsedConfigFile.fileNames,
        options: parsedConfigFile.options,
        projectReferences: parsedConfigFile.projectReferences,
        configFileParsingDiagnostics: ts.getConfigFileParsingDiagnostics(parsedConfigFile)
      };
    }
    return {
      rootNames: Declgen.collectInputFiles(opts),
      options: defaultCompilerOptions()
    };
  }

  private static collectInputFiles(opts: DeclgenCLIOptions): string[] {
    const inputFiles = [] as string[];

    if (opts.inputFiles) {
      inputFiles.push(...opts.inputFiles);
    }

    if (opts.inputDirs) {
      for (const dir of opts.inputDirs) {
        try {
          inputFiles.push(...getSourceFilesFromDir(dir));
        } catch (error) {
          Logger.error('Failed to read folder: ' + error);
          process.exit(-1);
        }
      }
    }

    return inputFiles;
  }
}

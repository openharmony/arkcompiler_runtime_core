/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

import * as fs from 'fs';
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
  private readonly declgenOptions: DeclgenCLIOptions;

  constructor(
    declgenOptions: DeclgenCLIOptions,
    customResolveModuleNames?: (moduleName: string[], containingFile: string) => ts.ResolvedModuleFull[],
    compilerOptions?: ts.CompilerOptions
  ) {
    const { rootNames, options } = Declgen.parseDeclgenOptions(declgenOptions);

    this.rootFiles = rootNames;
    this.declgenOptions = declgenOptions;

    this.sourceFileMap = new Map<string, ts.SourceFile>();
    this.compilerOptions = Object.assign({}, options, {
      declaration: true,
      emitDeclarationOnly: true,
      outDir: declgenOptions.outDir,
      ...(declgenOptions.rootDir ? { rootDir: declgenOptions.rootDir } : {}),
      ...(compilerOptions ?? {})
    });
    // Prevent the noemit of the passed compilerOptions from being true
    this.compilerOptions.noEmit = false;
    this.hookedHost = Declgen.createHookedCompilerHost(this.sourceFileMap, this.compilerOptions, declgenOptions);
    if (customResolveModuleNames) {
      this.hookedHost.resolveModuleNames = customResolveModuleNames;
    }
  }

  run(): DeclgenResult {
    /**
     * First compilation with the hooked CompilerHost:
     * collect the SourceFiles after transformation to the hooked Map
     */
    let program = this.recompile();

    /**
     * If the rootFiles contains declaration files,
     * process the declaration files here.
     */
    this.processDeclarationFiles(program);

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

    return {
      emitResult: emitResult
    } as DeclgenResult;
  }

  private recompile(): ts.Program {
    return compile(this.rootFiles, this.compilerOptions, this.hookedHost);
  }

  private processDeclarationFiles(program: ts.Program): void {
    const typeChecker = program.getTypeChecker();

    this.rootFiles.forEach((fileName) => {
      const sourceFile = program.getSourceFile(fileName);
      if (sourceFile && sourceFile.isDeclarationFile) {

        const compilerOptions = program.getCompilerOptions();
        const outDir = compilerOptions.outDir || path.dirname(fileName);
        const rootDir = compilerOptions.rootDir || path.dirname(fileName);
        const relativePath = path.relative(rootDir, fileName);
        const fileNameWithoutExt = relativePath.replace(/\.d\.(ts|ets)$/, '');
        const dEtsFilePath = path.join(outDir, `${fileNameWithoutExt}${Extension.DETS}`);

        const result = this.transformDeclarationFiles(program, sourceFile, typeChecker);
        const printer = ts.createPrinter();
        const transformedCode = printer.printFile(result.transformed[0] as ts.SourceFile);

        if (Declgen.isFileInAllowedPath(this.declgenOptions, [sourceFile])) {
          if (!fs.existsSync(outDir)) {
            fs.mkdirSync(outDir, { recursive: true });
          }
          fs.writeFileSync(dEtsFilePath, transformedCode, { encoding: 'utf8' });
        }
      }
    });
  }

  private transformDeclarationFiles(program:ts.Program, sourceFile: ts.SourceFile, typeChecker: ts.TypeChecker,): ts.TransformationResult<ts.Node> { 
    const result = ts.transform(
      sourceFile,
      [
        (context: ts.TransformationContext): ts.Transformer<ts.SourceFile> => {
          const autofixer = new Autofixer(typeChecker, context);
          const visit = (node: ts.Node): ts.Node | undefined => {
            const fixedNode = autofixer.fixNode(node);

            if (fixedNode === undefined) {
              return node;
            } else if (Array.isArray(fixedNode)) {
              return context.factory.createBlock(fixedNode as readonly ts.Statement[]);
            } else {
              return ts.visitEachChild(fixedNode, visit, context);
            }
          };
          return (sourceFile: ts.SourceFile): ts.SourceFile => {
            return visit(sourceFile) as ts.SourceFile;
          };
        }
      ],
      program.getCompilerOptions()
    );

    return result;
  }

  private static createHookedCompilerHost(
    sourceFileMap: Map<string, ts.SourceFile>,
    compilerOptions: ts.CompilerOptions,
    declgenOptions: DeclgenCLIOptions
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
        if (!Declgen.isFileInAllowedPath(declgenOptions, sourceFiles)) {
          return;
        }
        const parsedPath = path.parse(fileName);
        fallbackWriteFile(
          /*
           * Since `.d` part of `.d.ts` extension is a part of the parsedPath.name,
           * use `Extension.Ets` for output file name generation.
           */
          path.join(parsedPath.dir, `${parsedPath.name}${Extension.ETS}`),
          text,
          writeByteOrderMark,
          onError,
          sourceFiles,
          data
        );
      }
    });
  }

  private static isFileInAllowedPath(
    declgenOptions: DeclgenCLIOptions,
    sourceFiles?: readonly ts.SourceFile[]
  ): boolean {
    // equal to includePaths = [*]
    if (!declgenOptions.includePaths?.length) {
      return true;
    }
    if (!sourceFiles?.length) {
      return false;
    }

    const filePath = path.resolve(sourceFiles[0].fileName);
    return declgenOptions.includePaths.some((allowedPath) => {
      return filePath.startsWith(path.resolve(allowedPath));
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

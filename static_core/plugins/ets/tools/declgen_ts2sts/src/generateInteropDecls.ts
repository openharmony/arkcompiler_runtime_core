/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import * as ts from 'typescript';
import { Logger } from './logger/Logger';
import { logMessages, SilentLogger } from './logger/SilentLogger';
import { Declgen, DeclgenOptions } from './Declgen';
import { ModuleNameResolver } from './compiler/Compiler';
import { getSourceFilesFromDir } from './compiler/Compiler';

export interface RunnerParms {
  inputFiles: string[];
  inputDirs: string[];
  outDir: string;
  rootDir?: string;
  customResolveModuleNames?: ModuleNameResolver;
  customCompilerOptions?: ts.CompilerOptions;
}

export function generateInteropDecls(config: RunnerParms): string[] {
  Logger.init(new SilentLogger());

  const intputFiles = collectInputFiles(config);

  const tsConfig: DeclgenOptions = {
    outDir: config.outDir,
    inputFiles: intputFiles,
    rootDir: config.rootDir,
    features: {
      enableInteropTypesFix: true,
      removeReservedKeywordIdentifier: true
    }
  };
  const declgen = new Declgen(tsConfig, config.customCompilerOptions, config.customResolveModuleNames);
  declgen.run().emit();
  return logMessages;
}

function collectInputFiles(opts: RunnerParms): string[] {
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

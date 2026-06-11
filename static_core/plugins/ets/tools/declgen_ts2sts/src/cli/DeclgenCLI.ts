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

import * as fs from 'node:fs';

import type { OptionValues } from 'commander';
import { Command } from 'commander';

import { CLI } from './CLI';

export interface DeclgenCLIOptions {
  outDir?: string;
  inputFiles: string[];
  tsconfig?: string;
  inputDirs: string[];
  rootDir?: string;
  /**
   * Only files in this directory will be generated into declaration files
   * default is [*]
   */
  includePaths?: string[];
  enableInteropTypesFix?: boolean;
  removeReservedKeywordIdentifier?: boolean;
  /**
   * Enable incremental builds backed by the declgen cache + tsc tsbuildinfo.
   */
  incremental?: boolean;
  /** Override the cache directory (default: `<outDir>/.declgen-cache/`). */
  cacheDir?: string;
  /** Force a full rebuild, ignoring any existing cache. */
  noCache?: boolean;
  /** When false, skip verifying previously-emitted outputs still exist (default: true). */
  verifyOutputs?: boolean;
}

export class DeclgenCLI extends CLI<DeclgenCLIOptions> {
  constructor() {
    super();
  }

  doRead(): string[] {
    void this;

    return process.argv;
  }

  doInit(): Command {
    void this;

    const cliParser = new Command();

    cliParser.name('declgen').description('Declarations generator for ArkTS.');
    cliParser.option('-o, --out <outDir>', 'Directory where to put generated declarations.');
    cliParser.option('-p, --project <tsconfigPath>', 'path to TS project config file');
    cliParser.option(
      '-d, --dir <projectDir>',
      'Directory with TS files to generate declrartions from.',
      (val, prev) => {
        return prev.concat(val);
      },
      [] as string[]
    );
    cliParser.option(
      '-f, --file <fileName>',
      'TS file to generate declarations from.',
      (val, prev) => {
        return prev.concat(val);
      },
      [] as string[]
    );
    cliParser.option(
      '-i, --include <includePath>',
      'Only files in these directories will be generated into declaration files. (default: [*])',
      (val, prev) => {
        return prev.concat(val);
      },
      [] as string[]
    );
    cliParser.option('--interop-types', 'Enable interop types fix.');
    cliParser.option('--remove-reserved-keyword-identifier', 'Remove reserved keyword identifier.');
    cliParser.option(
      '--incremental',
      'Enable incremental compilation. Reuses declgen cache + tsc .tsbuildinfo across runs to skip unchanged files.'
    );
    cliParser.option('--cache-dir <path>', 'Cache directory (default: <outDir>/.declgen-cache).');
    cliParser.option('--no-cache', 'Disable cache use for this run (equivalent to DECLGEN_NO_CACHE=1).');
    cliParser.option(
      '--no-verify-outputs',
      'Skip verifying previously-emitted .d.ets files still exist on disk. Speeds up startup at the cost of not detecting externally deleted/modified outputs.'
    );

    return cliParser;
  }

  doOptions(opts: OptionValues): DeclgenCLIOptions {
    void this;
    const options = {
      outDir: opts.out,
      inputFiles: opts.file,
      tsconfig: opts.project,
      inputDirs: opts.dir,
      includePaths: opts.include,
      enableInteropTypesFix: opts.interopTypes,
      removeReservedKeywordIdentifier: opts.removeReservedKeywordIdentifier,
      incremental: opts.incremental,
      cacheDir: opts.cacheDir,
      // commander's `--no-cache` flips `opts.cache` to false; map back.
      noCache: opts.cache === false,
      // commander's `--no-verify-outputs` flips `opts.verifyOutputs` to false.
      verifyOutputs: opts.verifyOutputs !== false
    };
    return options;
  }

  doValidate(opts: DeclgenCLIOptions): Error | undefined {
    void this;

    for (const entity of [...opts.inputDirs, ...opts.inputFiles]) {
      if (!fs.existsSync(entity)) {
        return new Error(`No entity <${entity}> exists!`);
      }
    }

    if (opts.tsconfig && !fs.existsSync(opts.tsconfig)) {
      return new Error(`Invalid tsconfig path! No file <${opts.tsconfig}> exists!`);
    }

    return undefined;
  }
}

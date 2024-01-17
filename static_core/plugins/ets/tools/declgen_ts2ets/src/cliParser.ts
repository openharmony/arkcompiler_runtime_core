/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

import * as fs from "fs";

import { Command } from 'commander';
import type { OptionValues } from 'commander';

function assertOptionIsDefined(opts: OptionValues, opt: string): void {
  if (opts[opt] === undefined) {
    throw new Error(`[ERROR] --${opt} option undefined`);
  }
}

function assertOptionIsValidPath(opts: OptionValues, opt: string): void {
  const opt_value = opts[opt];

  assertOptionIsDefined(opts, opt);

  if (!fs.existsSync(opts[opt])) {
    throw new Error(
      `[ERROR] --${opt} option invalid: ${opts[opt]} doesn't exist`
    );
  }
}

export function parseCliOpts(args: string[]): OptionValues {
  const program = new Command();
  program
    .option("--file <path>", "path to the file")
    .option("--out <path>", "path to the output directory")
    .parse(args);
  const cliOptions = program.opts();

  assertOptionIsValidPath(cliOptions, "file");
  assertOptionIsDefined(cliOptions, "out");

  return cliOptions;
}

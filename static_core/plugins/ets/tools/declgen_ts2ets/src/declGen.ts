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

import * as ts from "typescript";
import * as fs from "fs";

import { parseCliOpts } from "./cliParser";
import { transformAST } from "./ASTTransformer";

function createDir(path: string): void {
  if (!fs.existsSync(path)) {
    fs.mkdirSync(path, { recursive: true });
  }
}

function defaultCompilerOptions(): ts.CompilerOptions {
  return {
    target: ts.ScriptTarget.Latest,
    module: ts.ModuleKind.CommonJS,
    strict: true,
  };
}

function main(): number {
  let opts = parseCliOpts(process.argv);

  createDir(opts.out);

  const declOptions = defaultCompilerOptions();
  declOptions.declaration = true;
  declOptions.emitDeclarationOnly = true;
  declOptions.outDir = opts.out;

  const host = ts.createCompilerHost(defaultCompilerOptions());

  let prog = ts.createProgram([opts.file], declOptions, host);

  let file = prog.getSourceFile(opts.file);
  if (file === undefined) {
    throw new Error(`can't get AST of the file ${opts.file}`);
  }

  let res = prog.emit(file, undefined, undefined, true, {
    afterDeclarations: [
      (context: ts.TransformationContext) => {
        return transformAST(prog.getTypeChecker(), context);
      },
    ],
  });

  return 0;
}

main();

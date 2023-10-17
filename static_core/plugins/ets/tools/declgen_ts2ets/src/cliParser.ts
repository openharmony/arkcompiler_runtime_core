import * as fs from "fs";

import { Command, OptionValues } from "commander";

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

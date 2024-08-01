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

import * as fs from 'node:fs';
import * as path from 'node:path';
import * as ts from 'typescript';
import { exit } from 'node:process';

import { Logger } from '../utils/logger/Logger';
import { ConsoleLogger } from '../utils/logger/ConsoleLogger';

import { CLI } from './cli/CLI';
import type { TestRunnerCLIOptions } from './cli/TestRunnerCLI';
import { TestRunnerCLI } from './cli/TestRunnerCLI';

import type { CheckerResult } from './Declgen';
import { Declgen } from './Declgen';
import type { FaultID } from '../utils/lib/FaultId';
import { Extension } from './utils/Extension';

interface Test {
  suite: string;
  name: string;
  outDir: string;
  testSource: string;
  expectedReport: string;
  expectedOutput: string;
}

interface TestResult {
  test: Test;
  outputsMatch: boolean;
  reportsMatch: boolean;
}

interface TestNodeInfo {
  line: number;
  column: number;
  faultId: FaultID;
}

interface TestReport {
  nodes: TestNodeInfo[];
}

const copyright = `/*
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
 */\n`;

function main(): void {
  Logger.init(new ConsoleLogger());

  const parsedCliOptions = CLI.parseCLI(new TestRunnerCLI());

  const testSuites = parsedCliOptions.testSuites.length ?
    parsedCliOptions.testSuites :
    readTestSuites(parsedCliOptions.testDirPath);

  const failedTests: TestResult[] = [];

  for (const testSuite of testSuites) {
    failedTests.push(...runTestSuite(testSuite, parsedCliOptions));

    Logger.info(`Finished running <${testSuite}> test suite`);
  }

  if (failedTests.length) {
    Logger.info('Failed tests:');

    for (const test of failedTests) {
      serializeTestResult(test);
    }

    exit(-1);
  }
}

function serializeTestResult(failedTest: TestResult): void {
  Logger.info('');
  Logger.info(`Suite: ${failedTest.test.suite}`);
  Logger.info(`Test: ${failedTest.test.name}`);
  Logger.info(`Generated file: ${failedTest.outputsMatch ? '[SUCCESS]' : '[FAIL]'}`);
  Logger.info(`Generated report: ${failedTest.reportsMatch ? '[SUCCESS]' : '[FAIL]'}`);
}

function runTestSuite(testSuite: string, opts: TestRunnerCLIOptions): TestResult[] {
  Logger.info(`Running <${testSuite}> test suite`);

  const failedTests: TestResult[] = [];

  for (const test of collectTests(testSuite, opts)) {
    const res = runTest(test);

    if (!res.outputsMatch || !res.reportsMatch) {
      failedTests.push(res);
    }
  }

  return failedTests;
}

function runTest(test: Test): TestResult {
  Logger.info(`Running test ${test.name}`);

  const declgen = new Declgen({ outDir: test.outDir, inputFiles: [test.testSource], inputDirs: [] });

  const { emitResult, checkResult } = declgen.run();

  const actualReport = toTestNodeInfo(checkResult);

  saveTestReport(actualReport, test);

  return {
    test: test,
    outputsMatch: compareExpectedAndActualOutputs(test, emitResult),
    reportsMatch: compareExpectedAndActualReports(test, actualReport)
  };
}

function toTestNodeInfo(checkResult: CheckerResult): TestReport {
  void checkResult;

  return { nodes: [] } as TestReport;
}

function saveTestReport(actualReport: TestReport, test: Test): void {
  if (!fs.existsSync(test.outDir)) {
    fs.mkdirSync(test.outDir);
  }

  const report = JSON.stringify(actualReport, undefined, 4);

  fs.writeFileSync(path.join(test.outDir, test.name + ts.Extension.Json), report);
}

function compareExpectedAndActualOutputs(test: Test, emitResult: ts.EmitResult): boolean {
  void emitResult;

  const expected = fs.readFileSync(test.expectedOutput).toString().replace(copyright, '');
  const actualPath = path.join(test.outDir, `${test.name}${Extension.DSTS}`);
  const actual = fs.readFileSync(actualPath).toString();

  if (expected !== actual) {
    Logger.error(
      `Outputs for ${test.name} differ!
Expected (${test.expectedOutput}):
'''
${expected}
'''
Actual (${actualPath}):
'''
${actual}
'''`
    );
    return false;
  }

  return true;
}

function compareExpectedAndActualReports(test: Test, actualReport: TestReport): boolean {
  const expectedReport: TestReport = JSON.parse(fs.readFileSync(test.expectedReport).toString());

  if (!expectedReport?.nodes || !Array.isArray(expectedReport.nodes)) {
    throw new Error(`Invalid format of the <${test.expectedReport}> file! JSON does not satisfy 'TestReport' format!`);
  }

  if (expectedReport.nodes.length !== actualReport.nodes.length) {
    Logger.error(
      `Results differ! Expected count: ${expectedReport.nodes.length} vs actual count: ${actualReport.nodes.length}`
    );

    return false;
  }

  let result = true;
  for (let i = 0; i < actualReport.nodes.length; ++i) {
    const actual = actualReport.nodes[i];
    const expected = expectedReport.nodes[i];

    if (!validateTestNodeInfo(expected)) {
      throw new Error(
        `Invalid format of the <${test.expectedReport}> file!` +
          `Node ${JSON.stringify(expected)} does not satisfy 'TestNodeInfo' format!`
      );
    }

    if (!areTestNodesEqual(expected, actual)) {
      Logger.error(`Expected node:\n${expected}\nActual node:\n${actual}`);
      result = false;
    }
  }

  return result;
}

function areTestNodesEqual(lhs: TestNodeInfo, rhs: TestNodeInfo): boolean {
  const columnsEq = lhs.column === rhs.column;
  const linesEq = lhs.line === rhs.line;
  const faultIdsEq = lhs.faultId === rhs.faultId;

  return columnsEq && linesEq && faultIdsEq;
}

function validateTestNodeInfo(node: TestNodeInfo): boolean {
  if (node === undefined) {
    return false;
  }

  if (node.column === undefined) {
    return false;
  }

  if (node.line === undefined) {
    return false;
  }

  if (node.faultId === undefined) {
    return false;
  }

  return true;
}

function collectTests(testSuite: string, opts: TestRunnerCLIOptions): Test[] {
  const testSuitePath = path.join(opts.testDirPath, testSuite);
  const testSuiteOutPath = path.join(opts.testOutPath, testSuite);

  const dirContents = fs.readdirSync(testSuitePath).filter((e) => {
    return fs.statSync(path.join(testSuitePath, e)).isFile();
  });

  const basenames = new Set(
    dirContents.map((v) => {
      return v.split('.')[0];
    })
  );

  const tests: Test[] = [];

  for (const name of basenames) {
    const testSource = `${name}${ts.Extension.Ts}`;
    const expectedReport = `${name}${ts.Extension.Json}`;
    const expectedOutput = `${name}${Extension.DSTS}`;

    if (!dirContents.includes(testSource)) {
      throw new Error(`Test ${name} is missing it's source file <${testSource}>!`);
    }

    if (!dirContents.includes(expectedReport)) {
      throw new Error(`Test ${name} is missing it's expected report file <${expectedReport}>!`);
    }

    if (!dirContents.includes(expectedOutput)) {
      throw new Error(`Test ${name} is missing it's expected output file <${expectedOutput}>!`);
    }

    tests.push({
      suite: testSuite,
      name: name,
      outDir: path.join(testSuiteOutPath, name),
      testSource: path.join(testSuitePath, testSource),
      expectedOutput: path.join(testSuitePath, expectedOutput),
      expectedReport: path.join(testSuitePath, expectedReport)
    });
  }

  return tests;
}

function readTestSuites(testDirPath: string): string[] {
  const testSuites: string[] = [];

  for (const entity of fs.readdirSync(testDirPath)) {
    const entityPath = path.join(testDirPath, entity);
    if (fs.statSync(entityPath).isDirectory()) {
      testSuites.push(entity);
    }
  }

  return testSuites;
}

main();

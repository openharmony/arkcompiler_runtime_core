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

/**
 * Exit codes for CookbookExecutor.
 *   0 – test passed
 *   1 – test failed (output or report mismatch)
 */
export const enum ExitCode {
  PASS = 0,
  FAIL = 1,
  ERROR = 2
}

/**
 * Test file.
 */
export interface TestFile {
  file: string,
  content?: string
}

export interface MismatchTestResult {
  expected: TestFile,
  actual: TestFile
};

export interface MismatchReportResult {
  detail: string
}

/**
 * Structured result that the executor writes to stdout as JSON.
 */
export interface TestResult {
  suite: string;
  testCase: string;
  exitCode: ExitCode;
  outputMismatch: MismatchTestResult[];
  reportMismatch: MismatchReportResult[];
  message: string;
}

export interface TestCase {
  suite: string;
  name: string;
  inputFiles: string[];
  rootDir?: string;
  expectedOutput: string;
  expectedReport: string;
  outDir: string;
}

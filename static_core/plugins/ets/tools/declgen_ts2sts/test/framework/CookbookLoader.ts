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

import * as fs from 'node:fs';
import * as path from 'node:path';
import * as ts from 'typescript';

import { Extension } from '../../src/utils/Extension';

import { TestCase } from './CookbookTest';

const SUITE_EXTENSIONS: Record<string, string> = {
  ts_source: ts.Extension.Ts,
  dts_declarations: ts.Extension.Dts,
};

export class CookbookLoader {
  private readonly testDir: string;
  private readonly testOutDir: string;

  constructor(testDir: string, testOutDir: string) {
    this.testDir = testDir;
    this.testOutDir = testOutDir;
  }

  discoverSuites(): string[] {
    const suites: string[] = [];
    const knownSuites = new Set(['ts_source', 'dts_declarations', 'package_source']);

    for (const entry of fs.readdirSync(this.testDir)) {
      const fullPath = path.join(this.testDir, entry);
      if (fs.statSync(fullPath).isDirectory() && knownSuites.has(entry)) {
        suites.push(entry);
      }
    }

    return suites;
  }

  /**
   * Load all test cases for the given test suites.
   * @param suites - Array of suite names to load.
   * @returns Array of TestCase objects.
   */
  loadTestCases(suites: string[]): TestCase[] {
    const suitesToLoad = suites;
    const testCases: TestCase[] = [];

    for (const suite of suitesToLoad) {
      testCases.push(...this.loadSuite(suite));
    }

    return testCases;
  }

  /**
   * Load all test cases for a single test suite.
   */
  private loadSuite(suite: string): TestCase[] {
    const suitePath = path.join(this.testDir, suite);

    if (!fs.existsSync(suitePath)) {
      throw new Error(`Test suite directory not found: ${suitePath}`);
    }

    const jsonDir = path.join(this.testDir, 'json_storage');
    const detsDir = path.join(this.testDir, 'dets_output');

    const entries = fs.readdirSync(suitePath).filter((e) => {
      const fullPath = path.join(suitePath, e);
      return (
        (suite === 'package_source' && fs.statSync(fullPath).isDirectory()) || fs.statSync(fullPath).isFile()
      );
    });

    // Get unique base names (strip extensions)
    const baseNames = new Set(entries.map((v) => v.split('.')[0]));
    const testCases: TestCase[] = [];

    for (const name of baseNames) {
      const testCase = this.buildTestCase(suite, name, suitePath, jsonDir, detsDir);
      if (testCase) {
        testCases.push(testCase);
      }
    }

    return testCases;
  }

  /**
   * Build a single TestCase from its components.
   */
  private buildTestCase(
    suite: string,
    name: string,
    suitePath: string,
    jsonDir: string,
    detsDir: string
  ): TestCase | undefined {
    const ext = SUITE_EXTENSIONS[suite] ?? '';
    const sourceEntry = suite === 'package_source' ? name : `${name}${ext}`;
    const expectedReport = path.join(jsonDir, `${name}${ts.Extension.Json}`);
    const expectedOutput =
      suite === 'package_source'
        ? path.join(detsDir, name)
        : path.join(detsDir, `${name}${Extension.DETS}`);

    // Build input files
    let inputFiles: string[];
    let rootDir: string | undefined;

    if (suite === 'package_source') {
      rootDir = path.join(suitePath, name);
      const inputJsonPath = path.join(suitePath, name, 'input.json');
      if (!fs.existsSync(inputJsonPath)) {
        return undefined;
      }
      const inputJson = JSON.parse(fs.readFileSync(inputJsonPath, 'utf-8')) as string[];
      inputFiles = inputJson.map((f) => path.join(suitePath, name, f));
    } else {
      inputFiles = [path.join(suitePath, sourceEntry)];
    }

    return {
      suite,
      name,
      inputFiles,
      rootDir,
      expectedOutput,
      expectedReport,
      outDir: path.join(this.testOutDir, suite, name),
    };
  }
}

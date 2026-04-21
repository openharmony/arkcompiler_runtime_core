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

import {
  ExitCode,
  TestCase,
  TestResult
} from './CookbookTest';
import { ExecutionResult } from './CookbookExecutor';
import kleur from 'kleur';
import { diff } from 'jest-diff';

/**
 * Aggregated summary of all test executions.
 */
export interface TestSummary {
  total: number;
  passed: number;
  failed: number;
  errors: number;
  totalDuration: number;
  results: TestResult[];
}

/**
 * CookbookReporter formats and prints test results to the terminal.
 *
 * It prints:
 *  - Per-test pass / fail status with duration
 *  - Detail of failures (expected vs actual diff)
 *  - Final summary
 */
export class CookbookReporter {
  private results: ExecutionResult[] = [];
  private runnerTimeCost?: number;

  /**
   * Record a test result for later reporting.
   */
  addResult(result: ExecutionResult): void {
    this.results.push(result);
    this.printResultInline(result);
  }

  /**
   * Print a one-line result immediately as the test finishes (streaming output).
   */
  private printResultInline(result: ExecutionResult): void {
    const status = result.testResult.exitCode === ExitCode.PASS
      ? kleur.green(' PASS ')
      : kleur.red(' FAIL ');
    const duration = kleur.dim(`(${result.duration}ms)`);
    console.log(`  ${status}  ${result.testResult.suite}/${result.testResult.testCase} ${duration}`);
  }

  /**
   * Report runner time cost
   */
  reportRunnerTimeCost(timeCost: number): void {
    this.runnerTimeCost = timeCost;
  }

  /**
   * Build and return the aggregated summary.
   */
  buildSummary(): TestSummary {
    let passed = 0;
    let failed = 0;
    let errors = 0;
    let totalDuration = 0;
    let results: TestResult[] = [];

    for (const r of this.results) {
      results.push(r.testResult);
      totalDuration += r.duration;
      if (r.testResult.exitCode === ExitCode.PASS) {
        passed++;
      } else if (r.testResult.exitCode === ExitCode.ERROR) {
        errors++;
      } else {
        failed++;
      }
    }

    return {
      total: results.length,
      passed,
      failed,
      errors,
      totalDuration,
      results: results
    };
  }

  /**
   * Print the full report to stdout.
   */
  printReport(): TestSummary {
    const summary = this.buildSummary();

    console.log('');
    console.log(kleur.bold('═══════════════════════════════════════════════════'));
    console.log(kleur.bold('  Test Summary'));
    console.log(kleur.bold('═══════════════════════════════════════════════════'));

    // Print failures in detail
    const failures = summary.results.filter((r) => r.exitCode !== 0);
    if (failures.length > 0) {
      console.log('');
      console.log(kleur.red().bold('  Failed tests:'));
      for (const f of failures) {
        const tag = f.exitCode === ExitCode.ERROR
          ? kleur.yellow('[NOT FOUND]')
          : kleur.red('[MISMATCH]');
        console.log(`    ${tag} ${f.suite}/${f.testCase}`);
        if (f.exitCode === ExitCode.ERROR && f.message) {
          for (const line of f.message.split('\n')) {
            console.log(`      ${kleur.dim(line)}`);
          }
        } else if (f.exitCode === ExitCode.FAIL) {
          for (const mismatch of f.outputMismatch) {
            if (!mismatch.expected.content) {
              console.log(`      ${kleur.red('Expected output file not found:')} ${mismatch.expected.file}`);
              continue;
            }
            if (!mismatch.actual.content) {
              console.log(`      ${kleur.red('Actual output file not found:')} ${mismatch.actual.file}`);
              continue;
            }
            console.log(`      ${kleur.red('Output mismatch:')}`);
            const diffText = diff(mismatch.expected.content ?? '', mismatch.actual.content ?? '', {
              expand: false,
              aAnnotation: `[Expected] ${mismatch.expected.file}`,
              bAnnotation: `[ Actual ] ${mismatch.actual.file}`,
            });
            for (const line of (diffText ?? '').split('\n')) {
              console.log(`        ${line}`);
            }
          }
        }
      }
    }

    console.log('');
    console.log(`  Total:    ${kleur.bold(String(summary.total))}`);
    console.log(`  Passed:   ${kleur.green(String(summary.passed))}`);
    console.log(`  Failed:   ${kleur.red(String(summary.failed))}`);
    console.log(`  Errors:   ${kleur.yellow(String(summary.errors))}`);
    if (this.runnerTimeCost !== undefined) {
      console.log(`  Runner time cost: ${formatDuration(this.runnerTimeCost)}/${kleur.dim(formatDuration(summary.totalDuration))}`);
    } else {
      console.log(`  Duration: ${kleur.dim(formatDuration(summary.totalDuration))}`);
    }
    console.log('');

    if (summary.failed + summary.errors > 0) {
      console.log(kleur.red().bold('  SOME TESTS FAILED'));
    } else {
      console.log(kleur.green().bold('  ALL TESTS PASSED'));
    }

    console.log(kleur.bold('═══════════════════════════════════════════════════'));
    console.log('');

    return summary;
  }
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

function formatDuration(ms: number): string {
  if (ms < 1000) {
    return `${ms}ms`;
  }
  const seconds = (ms / 1000).toFixed(2);
  return `${seconds}s`;
}

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

import * as os from 'node:os';
import * as path from 'node:path';
import { fork, type ChildProcess } from 'node:child_process';
import { Command } from 'commander';

import { CookbookLoader } from './CookbookLoader';
import type { TestCase } from './CookbookTest';
import { CookbookReporter } from './CookbookReporter';
import type { TestResult } from './CookbookTest';
import { ExitCode } from './CookbookTest';
import type { ExecutionResult } from './CookbookExecutor';
import type { WorkerTask, WorkerMessage } from './CookbookWorker';

interface RunnerCLIOptions {
  testDir: string;
  testSuites: string[];
  testOutDir: string;
  /** Maximum number of parallel workers. Defaults to CPU count. */
  jobs: number;
}

function parseCLI(): RunnerCLIOptions {
  const program = new Command();

  program
    .name('cookbook-runner')
    .description('Parallel test runner for declgen cookbook tests.')
    .requiredOption('--test-dir <path>', 'Root directory containing test suites')
    .option(
      '-t, --test-suite <name>',
      'Test suite to run (repeatable). If omitted, all suites are run.',
      (val: string, prev: string[]) => prev.concat(val),
      [] as string[]
    )
    .requiredOption('--test-out-dir <path>', 'Directory where Declgen output is stored')
    .option(
      '-j, --jobs <count>',
      'Maximum parallel workers',
      String(Math.floor(os.cpus().length / 2))
    );

  program.parse(process.argv);

  const opts = program.opts<{
    testDir: string;
    testSuite: string[];
    testOutDir: string;
    jobs: string;
  }>();

  return {
    testDir: opts.testDir,
    testSuites: opts.testSuite,
    testOutDir: opts.testOutDir,
    jobs: parseInt(opts.jobs, 10) || Math.floor(os.cpus().length / 2),
  };
}

/**
 * Internal handle for a single pre-warmed worker process.
 */
interface WorkerHandle {
  child: ChildProcess;
  taskResolve?: (result: ExecutionResult) => void;
  taskReject?: (err: Error) => void;
}

/**
 * A pool of pre-forked worker processes.
 *
 * Workers are forked once during {@link warmup} and eagerly load all heavy
 * modules (TypeScript, Declgen, etc.).  Once warmed they accept tasks via
 * IPC so that no new process needs to be spawned per test case.
 */
class WorkerPool {
  private allWorkers: WorkerHandle[] = [];
  private idleWorkers: WorkerHandle[] = [];
  private pendingTasks: Array<{
    task: WorkerTask;
    resolve: (result: ExecutionResult) => void;
    reject: (err: Error) => void;
  }> = [];

  /**
   * Fork `count` workers and wait for every one to signal `ready`.
   * Can run concurrently with other startup work (e.g. test discovery).
   */
  async warmup(workerScript: string, count: number): Promise<void> {
    const readyPromises: Promise<void>[] = [];
    for (let i = 0; i < count; i++) {
      readyPromises.push(this.spawnWorker(workerScript));
    }
    await Promise.all(readyPromises);
    console.log(`  Worker pool : ${count} worker(s) warmed up and ready.`);
  }

  /**
   * Fork a single worker process and resolve when it sends the `ready` IPC
   * message.
   */
  private spawnWorker(script: string): Promise<void> {
    return new Promise<void>((readyResolve, readyReject) => {
      const child = fork(script, [], { silent: true });
      const handle: WorkerHandle = { child };

      child.on('message', (msg: WorkerMessage) => {
        switch (msg.type) {
          case 'ready':
            this.allWorkers.push(handle);
            this.idleWorkers.push(handle);
            readyResolve();
            break;

          case 'result':
            handle.taskResolve?.(msg.result);
            handle.taskResolve = undefined;
            handle.taskReject = undefined;
            this.returnWorker(handle);
            break;

          case 'error':
            handle.taskReject?.(new Error(msg.error));
            handle.taskResolve = undefined;
            handle.taskReject = undefined;
            this.returnWorker(handle);
            break;
        }
      });

      child.on('error', (err) => {
        readyReject(err);
        handle.taskReject?.(err);
      });
    });
  }

  /**
   * Return a worker to the idle pool and dispatch any pending tasks.
   */
  private returnWorker(handle: WorkerHandle): void {
    if (this.pendingTasks.length > 0) {
      const { task, resolve, reject } = this.pendingTasks.shift()!;
      handle.taskResolve = resolve;
      handle.taskReject = reject;
      handle.child.send(task);
    } else {
      this.idleWorkers.push(handle);
    }
  }

  /**
   * Send a task to an idle worker, or enqueue it until one becomes available.
   */
  dispatch(task: WorkerTask): Promise<ExecutionResult> {
    return new Promise<ExecutionResult>((resolve, reject) => {
      const idle = this.idleWorkers.shift();
      if (idle) {
        idle.taskResolve = resolve;
        idle.taskReject = reject;
        idle.child.send(task);
      } else {
        this.pendingTasks.push({ task, resolve, reject });
      }
    });
  }

  /**
   * Terminate all worker processes.
   */
  shutdown(): void {
    for (const w of this.allWorkers) {
      w.child.kill();
    }
    this.allWorkers = [];
    this.idleWorkers = [];
  }
}

/**
 * Dispatch all test cases through the pre-warmed worker pool.
 *
 * Bounded parallelism is implicitly enforced by the pool size – only as many
 * tasks run concurrently as there are idle workers; the rest are queued.
 */
async function runParallel(
  testCases: TestCase[],
  pool: WorkerPool,
  testDir: string,
  testOutDir: string,
  reporter: CookbookReporter
): Promise<void> {
  const promises = testCases.map((testCase) => {
    const task: WorkerTask = {
      type: 'task',
      testDir,
      suite: testCase.suite,
      testCase: testCase.name,
      testOutDir,
    };

    return pool.dispatch(task).then(
      (execResult) => {
        reporter.addResult(execResult);
      },
      (err: Error) => {
        reporter.addResult({
          testResult: {
            suite: testCase.suite,
            testCase: testCase.name,
            exitCode: 2 as ExitCode,
            outputMismatch: [],
            reportMismatch: [],
            message: `Worker error: ${err.message}`,
          },
          duration: 0,
        });
      }
    );
  });

  await Promise.all(promises);
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------

async function main(): Promise<void> {
  const opts = parseCLI();

  console.log(`Cookbook Test Runner`);
  console.log(`  Test directory : ${opts.testDir}`);
  console.log(`  Output directory: ${opts.testOutDir}`);
  console.log(`  Parallel jobs  : ${opts.jobs}`);
  console.log('');

  // Resolve the compiled worker script path.
  // The worker is compiled alongside the runner under the same build output tree.
  const workerScript = path.join(__dirname, 'CookbookWorker.js');

  // Pre-warm the worker pool.  Workers eagerly load TypeScript, Declgen and
  // other heavy dependencies so they are ready to execute tests immediately.
  // This runs concurrently with test discovery below.
  const pool = new WorkerPool();
  const warmupPromise = pool.warmup(workerScript, opts.jobs);

  // Discover tests (runs concurrently with worker warmup)
  const loader = new CookbookLoader(opts.testDir, opts.testOutDir);
  const testCases = loader.loadTestCases(opts.testSuites);

  // Ensure workers are ready before dispatching any tasks
  await warmupPromise;

  if (testCases.length === 0) {
    pool.shutdown();
    console.log('No test cases found.');
    process.exit(0);
  }

  console.log(`Found ${testCases.length} test case(s) across suite(s): [${
    [...new Set(testCases.map((t) => t.suite))].join(', ')
  }]`);
  console.log('');

  const reporter = new CookbookReporter();

  // Run all tests through the pre-warmed worker pool
  const startTime = performance.now();
  await runParallel(testCases, pool, opts.testDir, opts.testOutDir, reporter);
  const totalTime = (performance.now() - startTime);
  reporter.reportRunnerTimeCost(totalTime);

  // Terminate worker processes
  pool.shutdown();

  // Print final report
  const summary = reporter.printReport();

  // Exit with non-zero if any test failed
  if (summary.failed + summary.errors > 0) {
    process.exit(1);
  }
}

main().catch((err) => {
  console.error('CookbookRunner fatal error:', err);
  process.exit(1);
});

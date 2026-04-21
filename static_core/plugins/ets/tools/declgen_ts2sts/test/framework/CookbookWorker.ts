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
 * Pre-warmed worker process for CookbookRunner.
 *
 * On startup, this script eagerly loads heavy modules (TypeScript, Declgen, etc.)
 * and initialises the logger. Once ready it signals the parent process via IPC
 * and then waits for task messages. Each task is executed in-process and the
 * result is sent back via IPC, avoiding the cost of forking a new Node.js
 * process for every test case.
 */

import { execute, type ExecutionResult } from './CookbookExecutor';
// ---------------------------------------------------------------------------
// IPC message types (shared with CookbookRunner)
// ---------------------------------------------------------------------------

/** Message sent from the parent to a worker to request test execution. */
export interface WorkerTask {
  type: 'task';
  testDir: string;
  suite: string;
  testCase: string;
  testOutDir: string;
}

/** Message sent from the worker to the parent when it is ready. */
export interface WorkerReady {
  type: 'ready';
}

/** Message sent from the worker to the parent with a test result. */
export interface WorkerResultMsg {
  type: 'result';
  result: ExecutionResult;
}

/** Message sent from the worker to the parent when execution fails. */
export interface WorkerError {
  type: 'error';
  error: string;
}

/** Union of all messages a worker can send to the parent. */
export type WorkerMessage = WorkerReady | WorkerResultMsg | WorkerError;

// Signal readiness to the parent process.
process.send?.({ type: 'ready' } as WorkerReady);

// ---------------------------------------------------------------------------
// Task loop – listen for tasks from the parent via IPC
// ---------------------------------------------------------------------------

process.on('message', async (msg: WorkerTask) => {
  if (msg.type !== 'task') {
    return;
  }

  try {
    const result = await execute(msg.testDir, msg.suite, msg.testCase, msg.testOutDir);
    process.send?.({ type: 'result', result } as WorkerResultMsg);
  } catch (err: unknown) {
    const errorMessage = err instanceof Error ? err.message : String(err);
    process.send?.({ type: 'error', error: errorMessage } as WorkerError);
  }
});

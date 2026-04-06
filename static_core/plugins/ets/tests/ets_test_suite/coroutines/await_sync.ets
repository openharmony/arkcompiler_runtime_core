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

async function asyncResolve42(): Promise<int> {
  return 42;
}

async function asyncReject(): Promise<int> {
  throw new Error("rejected");
}

function testAwaitSyncAlreadyResolved(): void {
  let p = Promise.resolve<int>(100);
  let val = p.awaitSync();
  arktest.assertEQ(val, 100, "awaitSync on already-resolved promise should return 100");
}

function testAwaitSyncAsyncResolution(): void {
  let p = asyncResolve42();
  let val = p.awaitSync();
  arktest.assertEQ(val, 42, "awaitSync on async-resolved promise should return 42");
}

function testAwaitSyncRejection(): void {
  let p = asyncReject();
  try {
      p.awaitSync();
      arktest.assertTrue(false, "awaitSync on rejected promise should throw");
  } catch (err) {
      arktest.assertTrue(err.toString().contains("rejected", 0),
          "awaitSync rejection error should contain 'rejected'");
  }
}

function syncSleep(ms: int): void {
  let start = Date.now();
  while (Date.now() - start < ms) {
    // do nothing
  }
}

function testAwaitSyncOtherThread(): void {
  let resolver: ((val: Any) => void) | null = null;
  let p = new Promise<Any>((resolve) => {
        resolver = resolve;
    });
  let lParams: LaunchParams = {
      workerGroupId: WorkerGroup.generateGroupId(WorkerDomain.GENERAL),
      abortFlag: false
  }; 
  launch(lParams) {
    syncSleep(100);
    resolver!(100);
  }
  let val = p.awaitSync() as int;
  arktest.assertEQ(val, 100, "awaitSync on other thread should return 100");
}

function main(): int {
  let testSuite = new arktest.ArkTestsuite("coroutines.await_sync");
  testSuite.addTest("testAwaitSyncAlreadyResolved", testAwaitSyncAlreadyResolved);
  testSuite.addTest("testAwaitSyncAsyncResolution", testAwaitSyncAsyncResolution);
  testSuite.addTest("testAwaitSyncRejection", testAwaitSyncRejection);
  testSuite.addTest("testAwaitSyncOtherThread", testAwaitSyncOtherThread);
  let res = testSuite.run();
  return res;
}

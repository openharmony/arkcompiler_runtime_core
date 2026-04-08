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

/*---
desc: Two EAWorkers can execute jobs independently
---*/

async function startTest() {
  let rwLock = new concurrency.stackless.AsyncReadWriteLock()
  let readLock = rwLock.readLock()
  let writeLock = rwLock.writeLock()

  let readRun = false
  let writeRun = false
  let i = 0

  const ea1 = new EAWorker()
  ea1.start()

  const ea2 = new EAWorker()
  ea2.start()

  let job1 = ea1.run<undefined>(() => {
    waitForCompletion(async () => {
      await readLock.lock()
      readRun = true
      i++
      arktest.assertEQ(1, i)
      i--
      readLock.unlock()
    })
  })

  let job2 = ea2.run<undefined>(() => {
    waitForCompletion(async () => {
      await writeLock.lock()
      writeRun = true
      i++
      arktest.assertEQ(1, i)
      i--
      Coroutine.Schedule()
      writeLock.unlock()
    })
  })

  job1.Await()
  job2.Await()

  arktest.assertTrue(readRun)
  arktest.assertTrue(writeRun)

  ea1.quit()
  ea2.quit()
}

function main() {
  waitForCompletion(startTest)
}
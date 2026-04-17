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
desc: Suspendable job is scheduled before other job on the same worker thread
tags: [no-warmup]
---*/

async function ready(): Promise<int> {
  return 0
}

async function suspendable(order: int[]): Promise<int> {
  await ready()
  order.push(1)
  return 0
}

async function startTest(): Promise<void> {
  let order: int[] = []

  const ea = new EAWorker()
  ea.start()

  await ea.run<undefined>(async () => {
    let high: Promise<int> = suspendable(order)

    let low: Promise<undefined> = launch<undefined>() {
      order.push(2)
    }

    await high
    await low
  })

  arktest.assertEQ(order.length, 2)
  arktest.assertEQ(order[0], 1)
  arktest.assertEQ(order[1], 2)

  ea.quit()
}

async function main(): Promise<void> {
  await startTest()
}

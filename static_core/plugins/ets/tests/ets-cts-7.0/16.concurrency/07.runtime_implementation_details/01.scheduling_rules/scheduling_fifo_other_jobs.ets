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
desc: Other jobs with the same priority are scheduled in FIFO order
tags: [no-warmup]
---*/

async function startTest(): Promise<void> {
  let order: int[] = []

  const ea = new EAWorker()
  ea.start()

  await ea.run<undefined>(async () => {
    let j1: Promise<undefined> = launch<undefined>() {
      order.push(1)
    }

    let j2: Promise<undefined> = launch<undefined>() {
      order.push(2)
    }

    await j1
    await j2
  })

  arktest.assertEQ(order.length, 2)
  arktest.assertEQ(order[0], 1)
  arktest.assertEQ(order[1], 2)

  ea.quit()
}

async function main(): Promise<void> {
  await startTest()
}

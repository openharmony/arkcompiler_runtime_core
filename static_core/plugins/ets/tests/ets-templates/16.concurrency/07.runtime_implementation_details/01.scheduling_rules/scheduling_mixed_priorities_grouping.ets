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
desc: Higher priority jobs are scheduled before lower priority jobs, FIFO is preserved within each priority
tags: [no-warmup]
---*/

async function resolved(): Promise<int> {
  return 0
}

async function startTest(): Promise<void> {
  let order: int[] = []

  const ea = new EAWorker()
  ea.start()

  await ea.run<undefined>(async () => {
    let low1: Promise<undefined> = launch<undefined>() {
      order.push(3)
    }

    let high1: Promise<int> = resolved().then((v: int): int => {
      order.push(1)
      return v
    })

    let low2: Promise<undefined> = launch<undefined>() {
      order.push(4)
    }

    let high2: Promise<int> = resolved().then((v: int): int => {
      order.push(2)
      return v
    })

    await high1
    await high2
    await low1
    await low2
  })

  arktest.assertEQ(order.length, 4)
  arktest.assertEQ(order[0], 1)
  arktest.assertEQ(order[1], 2)
  arktest.assertEQ(order[2], 3)
  arktest.assertEQ(order[3], 4)

  ea.quit()
}

async function main(): Promise<void> {
  await startTest()
}

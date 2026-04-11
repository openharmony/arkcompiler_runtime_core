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
---*/

async function resolved(): Promise<int> {
  return 0
}

async function startTest(): Promise<void> {
  let order: int[] = []

  const ea = new EAWorker()
  ea.start()

  ea.run<undefined>(() => {
    let low1 = launch<undefined>() {
      order.push(3)
    }

    let high1 = resolved().then((v: int): int => {
      order.push(1)
      return v
    })

    let low2 = launch<undefined>() {
      order.push(4)
    }

    let high2 = resolved().then((v: int): int => {
      order.push(2)
      return v
    })

    low1.Await()
    low2.Await()
  }).Await()

  arktest.assertTrue(
    order.length == 4 &&
    order[0] == 1 &&
    order[1] == 2 &&
    order[2] == 3 &&
    order[3] == 4
  )

  ea.quit()
}

async function main(): Promise<void> {
  await startTest()
}

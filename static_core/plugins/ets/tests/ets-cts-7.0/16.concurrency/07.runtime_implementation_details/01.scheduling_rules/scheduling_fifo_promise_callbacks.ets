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
desc: Promise callbacks with the same priority are scheduled in FIFO order
---*/

async function resolved(): Promise<int> {
  return 0
}

async function startTest(): Promise<void> {
  let order: int[] = []

  let p = resolved()

  let c1 = p.then((v: int): int => {
    order.push(1)
    return v
  })

  let c2 = p.then((v: int): int => {
    order.push(2)
    return v
  })

  await c1
  await c2

  arktest.assertTrue(order[0] == 1 && order[1] == 2)
}

async function main(): Promise<void> {
  await startTest()
}

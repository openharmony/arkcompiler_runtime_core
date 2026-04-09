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
desc: Suspendable jobs with the same priority are scheduled in FIFO order
---*/

async function ready(): Promise<int> {
  return 0
}

async function s1(order: int[]): Promise<int> {
  await ready()
  order.push(1)
  return 1
}

async function s2(order: int[]): Promise<int> {
  await ready()
  order.push(2)
  return 2
}

async function startTest(): Promise<void> {
  let order: int[] = []

  let p1 = s1(order)
  let p2 = s2(order)

  await p1
  await p2

  arktest.assertTrue(order.length == 2 && order[0] == 1 && order[1] == 2)
}

async function main(): Promise<void> {
  await startTest()
}
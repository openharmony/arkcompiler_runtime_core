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
desc: Asynchronous function can await another asynchronous function
expected_out:
    runtime:
        - Test for asynchronous function-1 is passed.
        - Test for asynchronous lambda is passed.
        - Test for asynchronous function-2 is passed.    
---*/

async function g(): Promise<int> {
  console.log("Test for asynchronous function-1 is passed."); //To ensure the async function has completed
  return 1
}

async function startTest() {
  let result = await (async (): Promise<int> => {
    let x = await g()
    console.log("Test for asynchronous lambda is passed."); //To ensure the async lambda has completed
    return x + 41
  })()

  arktest.assertEQ(42, result)
  console.log("Test for asynchronous function-2 is passed."); //To ensure the async function has completed
}

async function main(): Promise<void> {
  await startTest()
}

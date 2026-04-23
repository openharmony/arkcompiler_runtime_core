
/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
desc: Start a coroutine using an async function.
expected_out:
    runtime:
        - Test for coroutine using an async function is passed.
---*/

async function cof(a: int): Promise<int> {
  return a * 2;
}

function main(): void {
  let cof_result = cof(10);
  cof_result.then((result: int) => {
    arktest.assertEQ(result, 20);
    console.log("Test for coroutine using an async function is passed."); //To ensure the async function has completed
  });
}

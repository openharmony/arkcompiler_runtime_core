
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
desc: Start a coroutine with a complex lambda expression.
expected_out:
    runtime:
        - Test for coroutine with a complex lambda expression is passed.
---*/

async function main(): Promise<void> {
  const complexLambda = async (n: number): Promise<int> => {
    let sum: int = 0;
    for (let i = 1; i <= n; i++) {
      sum += i;
    }
    return Promise.resolve(sum);
  };
  await ( async ()=>{
    const result: int = await complexLambda(3);
    arktest.assertEQ(result, 6);
    console.log("Test for coroutine with a complex lambda expression is passed."); //To ensure the async lambda has completed
  })()
}

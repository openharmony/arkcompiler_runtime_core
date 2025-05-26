/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
desc: Test cases for various usage scenarios of asynchronous function.
      Concurrent execution of multiple asynchronous function.
---*/

async function methodReturnString(): Promise<string> {
  return "Hello";
}

async function methodReturnNumber(): Promise<number> {
  return 42.0;
}

async function methodConcurrentExecution(): Promise<Array<Promise<String | number>>> {
  const result = await Promise.all( new Array<Promise<string | number>>(
    methodReturnString(),
    methodReturnNumber(),
  ));
  return result as Array<Promise<String|number>>;
}

async function startTest() {
  const result = await methodConcurrentExecution();
  const resTemp = new Array<number | string>("Hello", 42);
  assertEQ(result === resTemp, true);
}

function main(): void {
  startTest();
}
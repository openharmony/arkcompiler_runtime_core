/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
desc: Call Error with try...recovery.
assert: In most case errors are caused by Virtial Machine or by standard libraries. In case it is necessary to cause an error explicitly, use “Error Statements”.
---*/

class TestError extends Error {}

function main(): void {
  let i: int = 1;
  
  try {
    throw new TestError();
  }
  catch (p: TestError) {
    i = 5;
  }

  assert i == 5;
}

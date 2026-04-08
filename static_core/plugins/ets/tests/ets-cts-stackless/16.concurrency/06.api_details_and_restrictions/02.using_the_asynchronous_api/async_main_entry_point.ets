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
desc: >-
  ArkTS supports the async entry point function. The main function itself
  can be declared as async, allowing direct use of await without
  waitForCompletion wrapper.
---*/

async function asyncCompute(x: int): Promise<int> {
    return x * 5;
}

async function main(): Promise<void> {
    let result = await asyncCompute(8);
    arktest.assertEQ(result, 40, "async main should support direct await");
}

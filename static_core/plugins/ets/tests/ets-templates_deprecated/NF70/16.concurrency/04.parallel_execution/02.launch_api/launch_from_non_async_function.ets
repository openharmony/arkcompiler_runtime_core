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
desc: launch is allowed in non-async functions
---*/

function runFromNonAsync(): int {
  return launch<int>() {
    return 5
  }.Await()
}

async function startTest() {
  let result = runFromNonAsync()
  arktest.assertEQ(5, result)
}

function main() {
  waitForCompletion(startTest)
}
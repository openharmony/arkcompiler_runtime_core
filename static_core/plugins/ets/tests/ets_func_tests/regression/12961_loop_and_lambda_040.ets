/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
desc: Verification of issue panda#12961
tags: []
---*/


function test () {
  let k : int  = 0;

  let f1 = (k:int) => { return ++k }

  do {
    k++;
  } while (k < 10)

  let f2 = (k:int) => { return ++k }


  let result = f1(k)
  result = f2(result)
  assertEQ(result,  12)

}

function main() {
    test();
}

/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
desc: If the result of the evaluation of e is equal to null or undefined, then NullPointerError is thrown.
---*/

function main(): void {
  let i: Int|undefined = 5;
  let ii: Int|undefined = undefined;
  try {
    (1 ? ii : i)!;
    assertTrue( false, "evaluation of e seems to be e" )
  } catch(npe: NullPointerError) {
    assertTrue( true, "NPE caught - correct")
  } catch(e) {
    assertTrue( false, "Some other Error thrown, not correct")
  }
}

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
desc: Logical complement operator
name: 07.expressions/20.unary_expressions/08.logical_complement/logical_complement_3
params: 1 => false
---*/

function main(): void {
  let a = "";
  assertEQ(!a, true, "Empty string is false");
  a = "non empty string"
  assertEQ(!a, false,"Non Empty string is true");
}

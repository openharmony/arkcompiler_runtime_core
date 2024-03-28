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
desc: lambda captured local not assigned variable.
assert: Any local variable used but not declared in a lambda body must be assigned before the lambda body, or a compile-time error occurs.
tags: [negative]
---*/

type func = (y: int) => int;

function main(): void {
  let i: int;

  let bar: func = (y: int): int => { return y + i; }

  assert bar(5) == 12;
}

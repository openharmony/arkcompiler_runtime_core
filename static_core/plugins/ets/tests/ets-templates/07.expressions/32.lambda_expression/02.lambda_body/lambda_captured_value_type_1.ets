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
desc: lambda captured local variable and invoke in other function.
---*/

type func = (y: int) => int;

function lambdaInit(): func {
  let i: int = 7;

  return (y: int): int => { return y + i; }
}

function main(): void {
  let bar: func = lambdaInit();
  assert bar(5) == 12;
}

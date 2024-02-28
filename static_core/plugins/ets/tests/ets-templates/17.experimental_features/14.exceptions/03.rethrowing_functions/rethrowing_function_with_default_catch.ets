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
desc: Rethrowing function declaration with default exception.
assert: A function that that accepts a throwing function as a parameter can be marked with the rethrows keyword. Such function is called a rethrowing function.
tags: [compile-only, negative]
---*/

type func = (): void;

function foo(bar: func throws): void rethrows {
  bar();
}

function canThrow (): void {
  throw new Exception();
}

function main(): void {
  let y: int = 1;
  try() {
    foo(canThrow);
  }
  catch() {
    y = 5;
  }
  assert y == 5;
}

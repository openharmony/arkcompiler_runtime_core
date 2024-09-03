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
desc: Declaring throwing lambda without 'throws' keyword.
assert: To indicate that a function (in general sense, meaning a function, method, constructor or lambda) can throw an error, use the throws keyword at the end of the signature. A function marked with throws is called a throwing function. A function type can also be marked as ‘throws’.
tags: [compile-only, negative]
---*/

class TestException extends Exception {}

type func = (x: int) => int;

function bar(): void {
  let foo: func = (x: int): int => { throw new TestException(); return 5; };

  let i = foo(5);
}

function main(): void {
  bar();
}

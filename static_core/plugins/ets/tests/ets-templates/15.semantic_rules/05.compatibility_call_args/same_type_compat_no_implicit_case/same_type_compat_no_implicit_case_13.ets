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
desc: T1 is compatible with T2 if implicitly convertible. Class does not implement interface.
tags: [compile-only, negative]
---*/

interface I {
  i: RegExp;
}

class A {
  i: RegExp;
}

function checkTypeIdentity<T>(a: T, b: T): boolean {
    return typeof a === typeof b;
}

function main(): void {
  const val1: RegExp = /test/;
  const val2: RegExp = /example/;
  const diffTypeVal: any = "This can be any type";

  const result1 = checkTypeIdentity<RegExp>(val1, val2);

  const result2 = checkTypeIdentity(val1, diffTypeVal);

  assertEQ(result1, true);
  assertEQ(result2, false);
}
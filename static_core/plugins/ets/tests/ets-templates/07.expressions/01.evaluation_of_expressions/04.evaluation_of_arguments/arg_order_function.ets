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
desc: Argument list evaluation order (functions)
---*/

let a: boolean = false;
let b: boolean = false;
let c: boolean = false;

function ta(): int {
  assertTrue( !a );
  assertTrue( !b );
  assertTrue( !c );
  a = true;
  return 0;
}

function tb(): int {
  assertTrue( a );
  assertTrue( !b );
  assertTrue( !c );
  b = true;
  return 1;
}

function tc(): int {
  assertTrue( a );
  assertTrue( b );
  assertTrue( !c );
  c = true;
  return 2;
}

function td(): int {
  assertTrue( a );
  assertTrue( b );
  assertTrue( c );
  return 3;
}


function test(a: int, b: int, c: int, d: int): void {
  assertTrue( a == 0 && b == 1 && c == 2 && d == 3 );
}

function main(): void {
  a = b = c = false;  // reset due to jit mode warmup

  test(ta(), tb(), tc(), td());
}

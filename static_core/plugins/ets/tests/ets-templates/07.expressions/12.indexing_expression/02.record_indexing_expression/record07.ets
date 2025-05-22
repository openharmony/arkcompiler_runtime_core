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
desc: Record index expression - key is of other types.
---*/

type Keys = 'key1' | 'key2' | 'key3';

let x2: Record<number, string> = {
  1: "hello",
  2: "buy",
};

function foo(n: number): string | undefined {
  return x2[n];
}

function main(): void {
  let x = foo(3);
  assertEQ(x, undefined);
}
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
desc: Optional chaining declaration.
---*/

class A {
    public x: int = 5;
}

function foo(a?: A|null): Int|undefined{
  return a?.x
}

function getX(): Int|undefined {
    let a: A|null = new A();
    return foo(a);
}

function main(): void {
  assertEQ( getX(), 5 )
}

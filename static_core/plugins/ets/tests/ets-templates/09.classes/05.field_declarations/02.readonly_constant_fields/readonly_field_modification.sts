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
desc: Readonly field modification.
assert: A variable with the readonly modifier may only be initialized once, its value could not be modified after initialization.
tags: [negative, compile-only]
---*/

class A {
  public readonly x: int = 1;
}

function main(): void {
    let instance: A = new A();
    instance.x = 5;
}

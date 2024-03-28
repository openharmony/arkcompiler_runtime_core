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
desc: lambda declaration with some parameters.
---*/

class A {}
final class B extends A {}

type func = (y: int, obj: Object, a: A) => int;

function main(): void {
  let bar: func = (y: int, obj: Object, a: A): int => { return 5; }

  assert bar(5, new Int(5), new B()) == 5;
}

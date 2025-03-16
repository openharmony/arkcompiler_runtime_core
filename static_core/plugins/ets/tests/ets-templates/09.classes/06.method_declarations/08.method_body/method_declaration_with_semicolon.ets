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
desc: Method declaraion with simply semicolon.
assert: A method body is either a block of code that implements the method or simply a semicolon, indicating the lack of an implementation. It is a compile-time error if a method declaration is neither abstract nor native and has a semicolon for its body.
tags: [compile-only, negative]
---*/

class A {
  run(): void;
}

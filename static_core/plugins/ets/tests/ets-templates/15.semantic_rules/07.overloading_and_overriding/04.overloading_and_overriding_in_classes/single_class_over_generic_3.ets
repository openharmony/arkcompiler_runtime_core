/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
desc: >-
    Two instance methods, two static methods with the same name or, two constructors are defined in the same class
    member. If signatures are overload-equivalent, then a compile-time error occurs. Otherwise, overloading is used.
tags: [compile-only, negative]
---*/

class Base<T> {
  constructor(i: Base<T>) {}
  constructor(i: Base<Number>) {}
}
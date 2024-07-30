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
desc: Direct implement one interface two times.
assert: It is a compile-time error if the same interface is mentioned as a direct superinterface more than once in a single implements clause. This is true even if the interface is named in different ways.
tags: [compile-only, negative]
---*/

interface A { }

interface B extends A { }

class C implements A, B { }

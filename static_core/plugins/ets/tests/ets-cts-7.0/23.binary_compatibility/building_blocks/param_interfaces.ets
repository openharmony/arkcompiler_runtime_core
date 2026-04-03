/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
desc: Graph of parameter interface. This is a collection of empty interfaces with complex subtyping hierarchies
tags:
- compile-only
- not-a-test
---*/

export interface PI1 {}
export interface PI2 {}
export interface PI3 extends PI1, PI2 {}
export interface PI4 extends PI2 {}
export interface PI5 extends PI3, PI4 {}

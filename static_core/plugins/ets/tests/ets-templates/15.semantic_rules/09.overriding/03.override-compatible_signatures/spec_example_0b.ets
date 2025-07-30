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
desc: Derived class cannot have non-compatible constraints of type parameters
tags: [compile-only, negative]
---*/

class Base {}
class Derived extends Base {}

class A2<ContravariantTypeParameter extends Derived> {}
class B2<ContravariantTypeParameter extends Base> extends A2<ContravariantTypeParameter> {}

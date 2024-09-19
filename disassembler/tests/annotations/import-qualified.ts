/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

import * as Namespace1 from "./a"

@#Namespace1.Anno()
class A {}

@#Namespace1.Anno
class B {}

@#Namespace1.Anno1({a: 1, b: "string"})
class C {}

@#Namespace1.Namespace2.Namespace3.Anno2()
class D {}

@#Namespace1.Namespace2.Namespace3.Anno2
class E {}

@#Namespace1.Namespace2.Namespace3.Anno3({a: 1, b: "string"})
class F {}
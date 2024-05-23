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
desc: 10.6.1 Interface Method Overload Signatures
name: 10.interfaces/6.interface_method_declarations/1.interface_method_overload_signatures/method_overload7
tags: [compile-only]
---*/

interface Interface1 {
    method1<A>(name: string | undefined = undefined): void {}
    method1<B>(name?: string): void {}
}

function main(): void {}

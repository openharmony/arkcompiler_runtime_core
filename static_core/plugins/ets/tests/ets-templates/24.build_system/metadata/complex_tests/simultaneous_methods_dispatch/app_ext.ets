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
desc: Package @panda/app - extension module (simultaneous with app_main)
tags: [not-a-test]
name: app_ext
---*/

import { Holder, TextFormatter } from '@panda/types/lib_types'
import { FancyTag } from '@panda/types/lib_impls'
import { describe, formatWith } from '@panda/utils/lib_utils'

// --- 15.12: Generic class with different type parameters (type erasure) ---
export function testGenericHolder(): string {
    let h1 = new Holder<int>(42)
    let h2 = new Holder<string>("hi")
    return h1.value.toString() + "," + h2.value
}

// --- 15.11.10: Default interface method overridden (FancyTag.fullLabel) ---
export function testDefaultMethodOverridden(): string {
    return new FancyTag().fullLabel()
}

// --- 15.10: Method overloading (TextFormatter.format with number and string) ---
export function testMethodOverload(): string {
    let f = new TextFormatter()
    return f.format(42.0) + "," + f.format("hi")
}

// --- 15.10: Function overloading (describe with number and string) ---
export function testFunctionOverload(): string {
    return describe(3.14) + "," + describe("text")
}

// --- 15.10: Cross-package overloaded function via lib_utils ---
export function testCrossPackageOverload(): string {
    let f = new TextFormatter()
    return formatWith(f, 7.0) + "," + formatWith(f, "ok")
}

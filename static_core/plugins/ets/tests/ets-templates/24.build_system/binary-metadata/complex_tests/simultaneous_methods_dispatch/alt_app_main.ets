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
desc: >-
    Alt app main module
tags: [not-a-test, compile-only]
files: [lib_types.ets, lib_impls.ets]
name: alt_app_main
arktsconfig: arktsconfig-alt-app.json
es2panda_options: ['emit-metadata', 'simultaneous', 'read-metadata']
---*/

import { Counter } from '@panda/types/lib_types'
import { Dog, HtmlRenderer, SimpleTag } from '@panda/types/lib_impls'

// --- 15.11: Override of concrete method ---
export function testOverrideSpeak(): string {
    return new Dog("Rex").speak()
}

// --- 15.11: Abstract method override ---
export function testAbstractOverride(): string {
    return new HtmlRenderer().render("hi")
}

// --- 15.11.10: Default interface method kept ---
export function testDefaultMethodKept(): string {
    return new SimpleTag().fullLabel()
}

// --- 15.11.10: Static methods ---
export function testStaticMethod(): string {
    Counter.reset()
    Counter.increment()
    Counter.increment()
    Counter.increment()
    return Counter.getCount().toString()
}

// --- 15.9: Inherited method call triggers assembler crash under --simultaneous ---
export function testInheritedMethod(): string {
    return new Dog("Rex").getName()
}

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
desc: Package @panda/app - main module (simultaneous, depends on types + utils)
tags: [not-a-test, compile-only]
files: [lib_utils.ets]
name: app_main
arktsconfig: arktsconfig-app.json
es2panda_options: ['emit-metadata', 'simultaneous', 'read-metadata']
---*/

import { Counter } from '@panda/types/lib_types'
import { Dog, HtmlRenderer, SimpleTag } from '@panda/types/lib_impls'

// --- 15.11: Direct call to overridden method (Dog.speak) ---
export function testOverrideSpeak(): string {
    return new Dog("Rex").speak()
}

// --- 15.11: Abstract method override (HtmlRenderer.render) ---
export function testAbstractOverride(): string {
    return new HtmlRenderer().render("hi")
}

// --- 15.11.10: Default interface method kept (SimpleTag.fullLabel) ---
export function testDefaultMethodKept(): string {
    return new SimpleTag().fullLabel()
}

// --- 15.11.6: Super call — super.speak() resolves to Animal.speak at compile-time ---
export function testSuperCall(): string {
    return new Dog("Rex").speakBoth()
}

// --- 15.11.10: Static methods — no dynamic dispatch (Counter) ---
export function testStaticMethod(): string {
    Counter.reset()
    Counter.increment()
    Counter.increment()
    Counter.increment()
    return Counter.getCount().toString()
}

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
desc: Package @panda/utils - dispatch helper functions (depends on @panda/types)
tags: [not-a-test, compile-only]
files: [lib_types.ets]
name: lib_utils
arktsconfig: arktsconfig-utils.json
es2panda_options: ['emit-metadata', 'read-metadata']
---*/

import { Animal, Renderer, Labeled, Holder, Vehicle, TextFormatter } from '@panda/types/lib_types'

// --- 15.11.10: Dynamic dispatch through base class reference ---
export function animalSpeak(a: Animal): string {
    return a.speak()
}

// --- 15.11: Dispatch through abstract class reference ---
export function renderContent(r: Renderer, content: string): string {
    return r.render(content)
}

// --- 15.11.10: Dispatch through interface reference (default method) ---
export function labeledFull(l: Labeled): string {
    return l.fullLabel()
}

// --- 15.12: Generic dispatch (type parameter erased at runtime) ---
export function holderDescribe<T>(h: Holder<T>): string {
    return (h.value as Object).toString()
}

// --- 15.9: Dispatch through base Vehicle reference ---
export function vehicleDescribe(v: Vehicle): string {
    return v.describe()
}

export function vehicleHonk(v: Vehicle): string {
    return v.honk()
}

// --- 15.10: Overloaded top-level functions ---
export function describe(val: number): string {
    return "desc_n:" + val.toString()
}

export function describe(val: string): string {
    return "desc_s:" + val
}

// --- 15.10: TextFormatter method overload forwarding ---
export function formatWith(f: TextFormatter, val: number): string {
    return f.format(val)
}

export function formatWith(f: TextFormatter, val: string): string {
    return f.format(val)
}

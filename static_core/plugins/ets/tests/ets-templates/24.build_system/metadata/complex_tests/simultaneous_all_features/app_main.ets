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
es2panda_options: ['emit-declaration', 'simultaneous']
---*/

import { GenBox, Config, Color, Wrapper, Registry, Priority, colorName, makeConfig, priorityLabel } from '@panda/types/lib_types'
import { Circle } from '@panda/types/lib_impls'
import { greet, makeAdder } from '@panda/utils/lib_utils'

// --- Generics + map ---
export function testGenBox(): string {
    let b = new GenBox<string>("test")
    let mapped = b.map<int>((s: string): int => { return s.length })
    return mapped.value.toString()
}

// --- Readonly fields via factory ---
export function testConfig(): string {
    let cfg = makeConfig(42)
    return cfg.id.toString() + ":" + cfg.label
}

// --- Enum across packages ---
export function testColor(): string {
    return colorName(Color.Green)
}

// --- Optional parameter default ---
export function testGreet(): string {
    return greet("World")
}

// --- Closure from factory ---
export function testAdder(): string {
    let add5 = makeAdder(5)
    return add5(3).toString()
}

// --- Nullable + nullish coalescing ---
export function testWrapper(): string {
    let w1 = new Wrapper(null)
    let w2 = new Wrapper("found")
    return w1.getOrDefault("empty") + "," + w2.getOrDefault("empty")
}

// --- Type inference ---
export function testInference(): string {
    let x = 100
    let s = "inferred"
    return s + x.toString()
}

// --- Abstract + interface (describe) ---
export function testCircleDescribe(): string {
    return new Circle(1.0).describe()
}

// --- Interface default method across packages ---
export function testDefaultMethod(): string {
    return new Circle(1.0).summary()
}

// --- Static fields and methods across packages ---
export function testStaticRegistry(): string {
    Registry.reset()
    let a = new Registry("alpha")
    let b = new Registry("beta")
    return Registry.getCount().toString() + ":" + a.getName() + "," + b.getName()
}

// --- Computed enum values across packages ---
export function testComputedEnum(): string {
    return Priority.Medium.toString() + ":" + priorityLabel(Priority.High)
}

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
desc: Package @panda/types - implementations (simultaneous with lib_types)
tags: [not-a-test]
name: lib_impls
---*/

import { Animal, Renderer, Labeled, Vehicle } from './lib_types'

// --- 15.11: Override concrete method in subclass ---
export class Dog extends Animal {
    constructor(name: string) {
        super(name)
    }
    override speak(): string {
        return this.getName() + ":woof"
    }
    // --- 15.11.6: Super call — compile-time resolved to Animal.speak ---
    speakBoth(): string {
        return super.speak() + "," + this.speak()
    }
}

// --- 15.11: Another override for polymorphic dispatch ---
export class Cat extends Animal {
    constructor(name: string) {
        super(name)
    }
    override speak(): string {
        return this.getName() + ":meow"
    }
}

// --- 15.11: Override abstract method ---
export class HtmlRenderer extends Renderer {
    override render(content: string): string {
        return this.prefix() + "<" + content + ">"
    }
}

// --- 15.11.10: Implements interface, keeps default fullLabel() ---
export class SimpleTag implements Labeled {
    getLabel(): string {
        return "simple"
    }
}

// --- 15.11.10: Implements interface, overrides default fullLabel() ---
export class FancyTag implements Labeled {
    getLabel(): string {
        return "fancy"
    }
    fullLabel(): string {
        return "*" + this.getLabel() + "*"
    }
}

// --- 15.9: Subclass overrides one method, inherits others ---
export class Car extends Vehicle {
    constructor(brand: string) {
        super(brand)
    }
    override describe(): string {
        return "car:" + this.getBrand()
    }
}


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
desc: Test for binary compatibility. Inserting class with no member to the type hierarchy.
tags:
- compile-only
- not-a-test
files: ['../../param_interfaces/param_interfaces.ets']
---*/

import * as PI from '../../param_interfaces/param_interfaces'

export interface I1 {
    m(p: PI.PI5): string {
        return "I1"
    }
}

export interface I2 {
    m(p: PI.PI3): string
}

export class C1 implements I1, I2 {
    m(p: PI.PI3): string {
        return "C1"
    }
}

export interface I4 extends I2 {}

export interface I3 extends I4 {
    m(p: PI.PI4): string {
        return "I3"
    }
}

export class C2 extends C1 implements I3 {}

export class C extends C2 {} // Inserted class

export interface I7 {}

export interface I6 extends I3, I7 {}

export interface I5 extends I6 {
    m(p: PI.PI1): string {
        return "I5"
    }
}

export class C3 extends C implements I5 {}

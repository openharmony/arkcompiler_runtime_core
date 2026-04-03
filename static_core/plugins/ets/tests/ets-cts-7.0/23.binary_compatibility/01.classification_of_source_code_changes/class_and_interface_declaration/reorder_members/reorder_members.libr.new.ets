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
desc: Test for binary compatibility. Reordering of members such as fields, methods.
tags:
- compile-only
- not-a-test
files: ['../../param_interfaces/param_interfaces.ets']
---*/

import * as PI from '../../param_interfaces/param_interfaces'

export interface I1 {
    m(p: PI.PI6): string {
        return "m(PI6) from I1"
    }
    m(p: PI.PI4): string
    m(p: PI.PI3): string
}

export class C1 implements I1 {
    m(p: PI.PI4): string {
        return "m(PI4) from C1"
    }
    m(p: PI.PI3): string {
        return "m(PI3) from C1"
    }
}

export class C2 {
    foo(b: boolean): string {
        return "foo(boolean) from C2"
    }
    foo(n: number): string {
        return "foo(number) from C2"
    }
}

export class C3 {
    public pi6: PI.PI6 = {};

    public foo2(): string {
        return "foo2() from C3"
    }

    public foo1(): string {
        return "foo1() from C3"
    }

    pi4: PI.PI4 = {};
    pi3: PI.PI3 = {};
}

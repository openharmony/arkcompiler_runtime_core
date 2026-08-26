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

/*
 * Baseline for the field-storage checks. `a` and `b` are the same width, so
 * their offsets are assigned in declaration order; `r` is a reference, whose
 * type id alone does not identify what it points at.
 */

import { RuntimeDebug } from 'std/debug';

class A { public who(): string { return 'A'; } }
class B { public who(): string { return 'B'; } }

class Holder {
    public a: int = 1;
    public b: int = 2;
    public r: A = new A();
    public tag(): string { return 'V1'; }
}

function show(h: Holder, when: string): void {
    console.println(when + ' a=' + h.a + ' b=' + h.b + ' r=' + h.r.who() + ' tag=' + h.tag());
}

/*
 * The native side is bound by the smoke driver (tests/ani/hotreload_smoke): bare paths,
 * the target resolved across the live panda files.
 */
class HotReloadBridge {
    public static native hotReloadNative(target: string, patch: string): int;

    public run(a0: string, a1: string, a2: string): int {
    let h = new Holder();
    // Values the DECLARED initialisers never produce, so a swapped offset is unmistakable
    h.a = 11;
    h.b = 22;
    show(h, 'before');
    let rc = HotReloadBridge.hotReloadNative(a0, a1);
    console.println('rc=' + rc);
    show(h, 'after ');
    return 0;
}
}

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
 * Real class shapes, version 2 --- the patch for `inherit_base`.
 *
 * Every body that `inherit_base` reads is changed here, and nothing structural is: same classes, same
 * interface, same method names, prototypes and access flags. So a correct reload must make all four
 * observations flip to V2, and a reload that leaves any table or cached pointer behind shows up as
 * exactly one of them staying on V1.
 *
 * argv: <targetPath> <patchPath>
 */

interface Shape {
    area(): int;
}

class Base {
    // Inherited by `Derived` and NOT overridden -> exercises the inherited vtable slot
    public who(): string {
        return 'BASE-V2';
    }

    // Satisfies `Shape.area()` for `Derived` -> exercises the inherited ITable / IMT entry
    public area(): int {
        return 42;
    }
}

class Derived extends Base implements Shape {
    public own(): string {
        return 'OWN-V2';
    }
}

/*
 * A class whose static initializer has already failed, i.e. one that is ERRONEOUS by the time the
 * reload runs. It can never be initialized again no matter what its body is replaced with, so it is
 * dropped from the transaction --- the point being that the rest of the module still reloads.
 * Failing the whole transaction instead would mean one `<cctor>` that threw once, possibly inside a
 * swallowed `try` like the one in `main`, disables hot reload for the whole app until it restarts.
 */
class Broken {
    public static v: int = Broken.blow();

    static blow(): int {
        throw new Error('this cctor fails on purpose');
    }

    public static tag(): string {
        return 'BROKEN-V2';
    }
}

/*
 * The interface call must cross a function boundary. Inside `main` the compiler can see that the
 * `Shape` reference was just assigned a `Derived`, narrows the static type, and emits an ordinary
 * virtual call --- which goes through the vtable and never touches the ITable. Behind a parameter
 * of interface type it has to emit a real interface call, which resolves through IMT and ITable.
 */
function callArea(s: Shape): int {
    return s.area();
}

function reload(target: string, patch: string): int {
    return HotReloadBridge.hotReloadNative(target, patch);
}

/*
 * The native side is bound by the smoke driver (tests/ani/hotreload_smoke): bare paths,
 * the target resolved across the live panda files.
 */
class HotReloadBridge {
    public static native hotReloadNative(target: string, patch: string): int;

    public run(a0: string, a1: string, a2: string): int {
    let d = new Derived();
    let asBase: Base = d;
    let asShape: Shape = d;

    // Poison `Broken`: touching it runs the failing `<cctor>` and leaves the class ERRONEOUS
    try {
        console.println('brk=' + Broken.v);
    } catch (e) {
        console.println('brk=erroneous');
    }

    // Captured BEFORE the reload, used after it: this is the handle that goes stale
    let cached = Class.of(d).getInstanceMethod('own')!;

    console.println('before who=' + asBase.who() + ' area=' + callArea(asShape) + ' own=' + d.own() +
                    ' refl=' + cached.invoke(d));

    let rc = reload(a0, a1);
    console.println('rc=' + rc);

    console.println('after  who=' + asBase.who() + ' area=' + callArea(asShape) + ' own=' + d.own() +
                    ' refl=' + cached.invoke(d));
    return 0;
}
}

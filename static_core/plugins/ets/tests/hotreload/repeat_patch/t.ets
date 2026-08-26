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
 * Many reloads in a row, version 2 --- identical to version 1 apart from the tag, so that
 * alternating between the two files always has something to swap.
 *
 * `gen_three` proves that reload N+1 works after reload N. This one is about what that COSTS: every
 * generation stays registered with the class linker for the rest of the process, so anything the
 * commit does per loaded panda file, per obsolete class or per redirect entry turns into work that
 * grows with the number of reloads already done.
 *
 * That is not hypothetical --- sweeping every file's `PandaCache` unconditionally made each reload
 * proportional to the reloads before it, and 800 reloads took 1.9 s and 42 MB instead of 0.09 s and
 * 3.8 MB. A per-reload regression is invisible at the two reloads `gen_three` does, so the count
 * here is high enough for a quadratic term to show up as a timeout.
 *
 * The bodies alternate V1 / V2 so that every iteration is a real swap: each reload's target is the
 * generation the previous one installed, which is exactly the shape an IDE produces.
 *
 * argv: <abcA> <abcB>
 */

class Repeated {
    public tag(): string {
        return 'V2';
    }
}

/*
 * The native side is bound by the smoke driver (tests/ani/hotreload_smoke): bare paths,
 * the target resolved across the live panda files.
 */
class HotReloadBridge {
    public static native hotReloadNative(target: string, patch: string): int;

    public run(a0: string, a1: string, a2: string): int {
    let r = new Repeated();
    let n = 0;
    // High enough that a per-reload cost proportional to the generation count blows the timeout
    while (n < 400) {
        // a0 is the path every generation is published under, so it stays the target forever
        let rc = HotReloadBridge.hotReloadNative(a0, (n % 2 == 0) ? a1 : a0);
        if (rc != 0) {
            console.println('stopped at ' + n + ' rc=' + rc);
            return 1;
        }
        n = n + 1;
    }
    console.println('reloads=' + n);
    // ...and the object created before the first reload is still usable after the last one
    console.println('tag=' + r.tag());
    return 0;
}
}

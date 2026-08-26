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
 * Successive reloads of the same target, generation 3 (second patch).
 *
 * The point of this case: the caller passes ONE stable `targetPath` -- the string it originally
 * loaded the abc with -- and keeps passing it for every reload, which is exactly what an IDE does
 * when the developer presses the button twice. That only works if each new generation takes over
 * the target's identity, and if target resolution distinguishes the current file from older files
 * that still own LIVE skipped classes. Every superseded generation remains registered under the
 * same name.
 *
 * An interface makes both generations partial: it stays on the original panda file while
 * `Counter` moves to the next one. The second reload must still resolve the current generation,
 * collect the carried-over interface again, and report it as skipped again.
 *
 * argv: <targetPath> <patch2Path> <patch3Path>
 */

interface Marker { ping(): int; }

class Counter implements Marker {
    public ping(): int { return 7; }
    public n: int = 0;

    public bump(): int {
        this.n = this.n + 1;
        return this.n;
    }

    public tag(): string {
        return 'V3';
    }
}

/*
 * The native side is bound by the smoke driver (tests/ani/hotreload_smoke): bare paths,
 * the target resolved across the live panda files.
 */
class HotReloadBridge {
    public static native hotReloadNative(target: string, patch: string): int;

    public run(a0: string, a1: string, a2: string): int {
    let c = new Counter();
    c.bump();
    console.println('gen0=' + c.tag() + ' n=' + c.n);

    let rc1 = HotReloadBridge.hotReloadNative(a0, a1);
    c.bump();
    console.println('rc1=' + rc1 + ' gen1=' + c.tag() + ' n=' + c.n);

    // The SAME target string again -- not the path of the patch installed a moment ago
    let rc2 = HotReloadBridge.hotReloadNative(a0, a2);
    c.bump();
    console.println('rc2=' + rc2 + ' gen2=' + c.tag() + ' n=' + c.n);
    return 0;
}
}

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
 * Incremental patch: `Greeter` (changed) and every other class that is already loaded when the
 * reload happens (`Calc`, the module globals). `LazyHolder` and `Lazy` are NOT loaded at that
 * point and are absent from the patch --- exactly the hole an incremental compilation leaves.
 *
 * The reload must swap `Greeter` (tag=V2) and still resolve what the patch omits: `LazyHolder`
 * and `Lazy` are used only AFTER the reload and must fall through the patch to the retained
 * original file (lazy=L1).
 *
 * Regression: the publication used to REPLACE the target's slot in `abcFiles`, which stranded
 * every never-loaded class of the target.
 */

class Greeter {
    public n: int = 0;

    public bump(): int {
        this.n = this.n + 1;
        return this.n;
    }

    // the only change relative to the baseline
    public tag(): string {
        return 'V2';
    }
}

class Calc {
    public static base: int = 100;

    public static calc(): int {
        return 1;
    }
}

function report(g: Greeter, phase: string): void {
    console.println(phase + ' tag=' + g.tag() + ' n=' + g.n + ' calc=' + Calc.calc() + ' base=' + Calc.base);
}

function doReload(target: string, patch: string): int {
    return HotReloadBridge.hotReloadNative(target, patch);
}

/*
 * The native side is bound by the smoke driver (tests/ani/hotreload_smoke): bare paths,
 * the target resolved across the live panda files.
 */
class HotReloadBridge {
    public static native hotReloadNative(target: string, patch: string): int;

    public run(a0: string, a1: string, a2: string): int {
    return 0;
}
}

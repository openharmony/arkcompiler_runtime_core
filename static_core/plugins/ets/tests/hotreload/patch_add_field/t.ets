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
 * Hotreload smoke test, negative case: adds a field, must be refused.
 *
 * The method bodies here are the patched ones ON PURPOSE. If the transaction ever applied part of the
 * swap before detecting the structural change, the `after` and `lazy` lines would show V2/42/L2 --
 * with baseline bodies the "nothing was modified" assertion would hold no matter what the code did.
 *
 * The file name MUST be `t.ets` in every version: es2panda derives the module name from the
 * source basename, so all versions produce identical record descriptors (`t.Greeter`, `t.Calc`,
 * `t.LazyHolder`, `t.Lazy`, `t.ETSGLOBAL`), which is what makes the abc files comparable.
 *
 * argv: <targetPath> <patchPath>
 */

class Greeter {
    public n: int = 0;
    public extra: int = 0; // structural change: must be rejected

    public bump(): int {
        this.n = this.n + 1;
        return this.n;
    }

    // the patch changes the returned literal -> proves a method body was really swapped
    public tag(): string {
        return 'V2';
    }
}

class Calc {
    public static base: int = 100;

    // the patch changes the arithmetic -> proves static dispatch also goes to the new body
    public static calc(): int {
        return 42;
    }
}

/*
 * `Lazy` is reachable ONLY through `LazyHolder.get()`, i.e. two levels away from `main`.
 *
 * That distance matters: with the default `--verification-mode=ahead-of-time`, verifying a class
 * loads the classes its own methods reference, but only ONE level deep. So `LazyHolder` is already
 * loaded when the reload happens (and is swapped like any other class), while `Lazy` is not loaded
 * yet. Loading it afterwards therefore goes through the lazy path
 * (`EtsClassLinkerContext::EnumeratePandaFilesImpl` -> `EtsAbcFile::GetPandaFile()`), which only
 * yields the patch if the transaction published the new file into the `AbcRuntimeLinker` slot.
 */
class LazyHolder {
    public get(): string {
        return new Lazy().tag();
    }
}

class Lazy {
    public tag(): string {
        return 'L2';
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
    let g = new Greeter();
    g.bump();
    g.bump();
    // mutate a static field so that "state survives" is not just the cctor default
    Calc.base = 777;

    report(g, 'before');
    let rc = doReload(a0, a1);
    console.println('rc=' + rc);
    report(g, 'after ');
    // first use of `LazyHolder.get()` in the whole program, so `Lazy` is loaded only now
    console.println('lazy=' + new LazyHolder().get());
    return 0;
}
}

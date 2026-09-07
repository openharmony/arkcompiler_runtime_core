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
 * Coldreload smoke test, base program (the abc that `ark` runs).
 *
 * The file name MUST be `t.ets` in every directory: es2panda derives the module
 * name from the source basename, so the program and every patch produce identical
 * record descriptors (`t.Greeter`, `t.Calc`, ...), which is what makes a class of
 * the patch replace the same class of the original file.
 *
 * Cold reload is a startup-time mechanism: main() must call coldReload BEFORE any
 * of the classes under test is loaded. Two things keep that order:
 *
 *  - the linker comes from `Class.current()` -- the module's own class -- so no
 *    instance has to be created before the call;
 *  - `Driver` is the only class main references directly, so it is the only one
 *    the ahead-of-time verification of main can load (it goes one level deep).
 *    `Driver` is VERIFIED only when it is initialized (`new Driver()`), and its
 *    methods resolve `Greeter`/`Sub`/`Calc` -- so those classes load from the
 *    patch iff `new Driver()` happens after coldReload, which is what main does.
 *    `LazyHolder`/`Lazy` are two levels away and load at the first `lazy()` call.
 *
 * argv: <patchPath> <patchPath2|''> <pre|''>
 *   patchPath   abc prepended by the first coldReload call ('' -> INVALID_INPUT_ARG)
 *   patchPath2  second patch prepended by a second coldReload call ('' -> no second call)
 *   'pre'       run Driver once BEFORE coldReload, so the classes load from the
 *               original file and stay unreachable for the patch (pins that cold
 *               reload is a startup-time mechanism, not a retroactive swap)
 */

class Greeter {
    public n: int = 0;

    public bump(): int {
        this.n = this.n + 1;
        return this.n;
    }

    // every patch changes the returned literal
    public tag(): string {
        return 'V1';
    }
}

/*
 * `Sub` stays empty in the base program. The `addmethod` patch declares `tag()` on
 * it -- a method ADDITION -- and the old call site `s.tag()` must dispatch into the
 * added override.
 */
class Sub extends Greeter {
}

class Calc {
    public static base: int = 100;

    public static calc(): int {
        return 1;
    }
}

class LazyHolder {
    public get(): string {
        return new Lazy().tag();
    }
}

class Lazy {
    public tag(): string {
        return 'L1';
    }
}

class Driver {
    public run(): string {
        let g = new Greeter();
        g.bump();
        g.bump();
        let s = new Sub();
        return 'tag=' + g.tag() + ' sub=' + s.tag() + ' n=' + g.n + ' calc=' + Calc.calc() + ' base=' + Calc.base;
    }

    public lazy(): string {
        return new LazyHolder().get();
    }
}

/*
 * The observation entry points the driver calls: `pre` optionally loads Driver BEFORE the
 * reload (the startup-contract violation case); `observe` always runs AFTER it, so the
 * classes under test resolve through the (possibly patched) search order.
 */
class TestRunner {
    public static pre(): void {
        let early = new Driver();
        console.println('pre ' + early.run() + ' lazy=' + early.lazy());
    }

    public static observe(): int {
        let driver = new Driver();
        console.println('run ' + driver.run());
        console.println('lazy=' + driver.lazy());
        return 0;
    }
}

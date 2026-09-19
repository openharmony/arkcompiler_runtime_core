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
 *
 * Hotreload vs concurrent lazy loading, program version (running when the reloads happen).
 *
 * Two real worker threads lazily load the target file's classes while the main coroutine drives a
 * storm of reloads of the same file. The loads are PACED: `main` signals every transaction
 * attempt through a shared array cell right before it, each worker waits for that signal and then
 * lets a bounded number of ticks pass, so its load lands in the middle of the transaction ---
 * past the patch-open phase a load racing the signal would always win, before the commit ---
 * instead of relying on timing luck.
 *
 * The observable contract: a reload that reports success must not silently miss a class another
 * thread published after its snapshot, or subsequent lookups keep returning the old class. A
 * reload that raced such a load must refuse (CLASS_INITIALIZING while it is still initializing,
 * CONCURRENT_LOADING once it is published) and change nothing.
 *
 * Because every later successful reload sweeps up anything an earlier one missed, the case is
 * decided by the LAST attempt that reports success: its own paced loads are the ones that must
 * not be lost. After the final success no further reload runs, so a class lost by it stays stale
 * and the closing check sees it.
 *
 * argv: <targetPath> <patchPath>
 */

import { launch } from 'std/concurrency'

const WORKERS: int = 2;
const PER_WORKER: int = 25;
const RELOADS: int = 15;
const RETRY_LIMIT: int = 10;
// Hang guard only: a lost flag write must turn into a skipped signal, not a hang
const GUARD: int = 5000000;
// Ticks between the signal and the load, see the worker
const DELAY: int = 400000;
// `hotreload::Error` ordinals, keep in sync with runtime/hotreload/hotreload.h
const ERR_CLASS_INITIALIZING: int = 26;
const ERR_CONCURRENT_LOADING: int = 27;

class Shared {
    // one slot per worker: distinct slots, so "has it started" needs no atomics
    public static started: FixedArray<int> = new FixedArray<int>(WORKERS);
    // signal cell, written by `main` right before every attempt: an array cell is the
    // cross-thread channel the mt case has always relied on
    public static seq: FixedArray<int> = new FixedArray<int>(1);
    // per-worker spin accumulators: the stores keep the pacing loop observable
    public static tick: FixedArray<int> = new FixedArray<int>(WORKERS);
    public static loader: WLoader = new WLoader();
}

class W0 {
    public tag(): string {
        return 'C2';
    }
}

class W1 {
    public tag(): string {
        return 'C2';
    }
}

class W2 {
    public tag(): string {
        return 'C2';
    }
}

class W3 {
    public tag(): string {
        return 'C2';
    }
}

class W4 {
    public tag(): string {
        return 'C2';
    }
}

class W5 {
    public tag(): string {
        return 'C2';
    }
}

class W6 {
    public tag(): string {
        return 'C2';
    }
}

class W7 {
    public tag(): string {
        return 'C2';
    }
}

class W8 {
    public tag(): string {
        return 'C2';
    }
}

class W9 {
    public tag(): string {
        return 'C2';
    }
}

class W10 {
    public tag(): string {
        return 'C2';
    }
}

class W11 {
    public tag(): string {
        return 'C2';
    }
}

class W12 {
    public tag(): string {
        return 'C2';
    }
}

class W13 {
    public tag(): string {
        return 'C2';
    }
}

class W14 {
    public tag(): string {
        return 'C2';
    }
}

class W15 {
    public tag(): string {
        return 'C2';
    }
}

class W16 {
    public tag(): string {
        return 'C2';
    }
}

class W17 {
    public tag(): string {
        return 'C2';
    }
}

class W18 {
    public tag(): string {
        return 'C2';
    }
}

class W19 {
    public tag(): string {
        return 'C2';
    }
}

class W20 {
    public tag(): string {
        return 'C2';
    }
}

class W21 {
    public tag(): string {
        return 'C2';
    }
}

class W22 {
    public tag(): string {
        return 'C2';
    }
}

class W23 {
    public tag(): string {
        return 'C2';
    }
}

class W24 {
    public tag(): string {
        return 'C2';
    }
}

class W25 {
    public tag(): string {
        return 'C2';
    }
}

class W26 {
    public tag(): string {
        return 'C2';
    }
}

class W27 {
    public tag(): string {
        return 'C2';
    }
}

class W28 {
    public tag(): string {
        return 'C2';
    }
}

class W29 {
    public tag(): string {
        return 'C2';
    }
}

class W30 {
    public tag(): string {
        return 'C2';
    }
}

class W31 {
    public tag(): string {
        return 'C2';
    }
}

class W32 {
    public tag(): string {
        return 'C2';
    }
}

class W33 {
    public tag(): string {
        return 'C2';
    }
}

class W34 {
    public tag(): string {
        return 'C2';
    }
}

class W35 {
    public tag(): string {
        return 'C2';
    }
}

class W36 {
    public tag(): string {
        return 'C2';
    }
}

class W37 {
    public tag(): string {
        return 'C2';
    }
}

class W38 {
    public tag(): string {
        return 'C2';
    }
}

class W39 {
    public tag(): string {
        return 'C2';
    }
}

class W40 {
    public tag(): string {
        return 'C2';
    }
}

class W41 {
    public tag(): string {
        return 'C2';
    }
}

class W42 {
    public tag(): string {
        return 'C2';
    }
}

class W43 {
    public tag(): string {
        return 'C2';
    }
}

class W44 {
    public tag(): string {
        return 'C2';
    }
}

class W45 {
    public tag(): string {
        return 'C2';
    }
}

class W46 {
    public tag(): string {
        return 'C2';
    }
}

class W47 {
    public tag(): string {
        return 'C2';
    }
}

class W48 {
    public tag(): string {
        return 'C2';
    }
}

class W49 {
    public tag(): string {
        return 'C2';
    }
}


class WLoader {
    /*
     * Each `W` class is referenced ONLY by its own `loadK` method, so the ahead-of-time
     * verification of one `loadK` (at its first call) loads exactly one class: the dispatch body
     * below references methods, not the `W` classes, and `main`/`worker` reference only `WLoader`.
     */
    public get(k: int): string {
        if (k == 0) {
            return this.load0();
        }
        if (k == 1) {
            return this.load1();
        }
        if (k == 2) {
            return this.load2();
        }
        if (k == 3) {
            return this.load3();
        }
        if (k == 4) {
            return this.load4();
        }
        if (k == 5) {
            return this.load5();
        }
        if (k == 6) {
            return this.load6();
        }
        if (k == 7) {
            return this.load7();
        }
        if (k == 8) {
            return this.load8();
        }
        if (k == 9) {
            return this.load9();
        }
        if (k == 10) {
            return this.load10();
        }
        if (k == 11) {
            return this.load11();
        }
        if (k == 12) {
            return this.load12();
        }
        if (k == 13) {
            return this.load13();
        }
        if (k == 14) {
            return this.load14();
        }
        if (k == 15) {
            return this.load15();
        }
        if (k == 16) {
            return this.load16();
        }
        if (k == 17) {
            return this.load17();
        }
        if (k == 18) {
            return this.load18();
        }
        if (k == 19) {
            return this.load19();
        }
        if (k == 20) {
            return this.load20();
        }
        if (k == 21) {
            return this.load21();
        }
        if (k == 22) {
            return this.load22();
        }
        if (k == 23) {
            return this.load23();
        }
        if (k == 24) {
            return this.load24();
        }
        if (k == 25) {
            return this.load25();
        }
        if (k == 26) {
            return this.load26();
        }
        if (k == 27) {
            return this.load27();
        }
        if (k == 28) {
            return this.load28();
        }
        if (k == 29) {
            return this.load29();
        }
        if (k == 30) {
            return this.load30();
        }
        if (k == 31) {
            return this.load31();
        }
        if (k == 32) {
            return this.load32();
        }
        if (k == 33) {
            return this.load33();
        }
        if (k == 34) {
            return this.load34();
        }
        if (k == 35) {
            return this.load35();
        }
        if (k == 36) {
            return this.load36();
        }
        if (k == 37) {
            return this.load37();
        }
        if (k == 38) {
            return this.load38();
        }
        if (k == 39) {
            return this.load39();
        }
        if (k == 40) {
            return this.load40();
        }
        if (k == 41) {
            return this.load41();
        }
        if (k == 42) {
            return this.load42();
        }
        if (k == 43) {
            return this.load43();
        }
        if (k == 44) {
            return this.load44();
        }
        if (k == 45) {
            return this.load45();
        }
        if (k == 46) {
            return this.load46();
        }
        if (k == 47) {
            return this.load47();
        }
        if (k == 48) {
            return this.load48();
        }
        if (k == 49) {
            return this.load49();
        }
        return 'none';
    }

    public load0(): string {
        return new W0().tag();
    }

    public load1(): string {
        return new W1().tag();
    }

    public load2(): string {
        return new W2().tag();
    }

    public load3(): string {
        return new W3().tag();
    }

    public load4(): string {
        return new W4().tag();
    }

    public load5(): string {
        return new W5().tag();
    }

    public load6(): string {
        return new W6().tag();
    }

    public load7(): string {
        return new W7().tag();
    }

    public load8(): string {
        return new W8().tag();
    }

    public load9(): string {
        return new W9().tag();
    }

    public load10(): string {
        return new W10().tag();
    }

    public load11(): string {
        return new W11().tag();
    }

    public load12(): string {
        return new W12().tag();
    }

    public load13(): string {
        return new W13().tag();
    }

    public load14(): string {
        return new W14().tag();
    }

    public load15(): string {
        return new W15().tag();
    }

    public load16(): string {
        return new W16().tag();
    }

    public load17(): string {
        return new W17().tag();
    }

    public load18(): string {
        return new W18().tag();
    }

    public load19(): string {
        return new W19().tag();
    }

    public load20(): string {
        return new W20().tag();
    }

    public load21(): string {
        return new W21().tag();
    }

    public load22(): string {
        return new W22().tag();
    }

    public load23(): string {
        return new W23().tag();
    }

    public load24(): string {
        return new W24().tag();
    }

    public load25(): string {
        return new W25().tag();
    }

    public load26(): string {
        return new W26().tag();
    }

    public load27(): string {
        return new W27().tag();
    }

    public load28(): string {
        return new W28().tag();
    }

    public load29(): string {
        return new W29().tag();
    }

    public load30(): string {
        return new W30().tag();
    }

    public load31(): string {
        return new W31().tag();
    }

    public load32(): string {
        return new W32().tag();
    }

    public load33(): string {
        return new W33().tag();
    }

    public load34(): string {
        return new W34().tag();
    }

    public load35(): string {
        return new W35().tag();
    }

    public load36(): string {
        return new W36().tag();
    }

    public load37(): string {
        return new W37().tag();
    }

    public load38(): string {
        return new W38().tag();
    }

    public load39(): string {
        return new W39().tag();
    }

    public load40(): string {
        return new W40().tag();
    }

    public load41(): string {
        return new W41().tag();
    }

    public load42(): string {
        return new W42().tag();
    }

    public load43(): string {
        return new W43().tag();
    }

    public load44(): string {
        return new W44().tag();
    }

    public load45(): string {
        return new W45().tag();
    }

    public load46(): string {
        return new W46().tag();
    }

    public load47(): string {
        return new W47().tag();
    }

    public load48(): string {
        return new W48().tag();
    }

    public load49(): string {
        return new W49().tag();
    }
}

function worker(id: int): int {
    Shared.started[id] = 1;
    const base: int = id * PER_WORKER;
    for (let i = 0; i < PER_WORKER; i++) {
        // Wait for the signal of attempt i + 1, then let the ticks pass so the load lands in
        // the middle of that transaction
        let guard: int = 0;
        while (Shared.seq[0] < i + 1 && guard < GUARD) {
            guard = guard + 1;
        }
        let delay: int = 0;
        while (delay < DELAY) {
            delay = delay + 1;
            Shared.tick[id] = Shared.tick[id] + 1;
        }
        Shared.loader.get(base + i);
    }
    return id;
}

function startedCount(): int {
    let n = 0;
    for (let i = 0; i < WORKERS; i++) {
        n = n + Shared.started[i];
    }
    return n;
}

/*
 * The native side is bound by the smoke driver (tests/ani/hotreload_smoke): bare paths,
 * the target resolved across the live panda files.
 */
class HotReloadBridge {
    public static native hotReloadNative(target: string, patch: string): int;

    public run(a0: string, a1: string, a2: string): int {
    let jobs: Job<int>[] = [];
    for (let i = 0; i < WORKERS; i++) {
        jobs.push(launch<int, (id: int) => int>(worker, i));
    }
    let waited = 0;
    while (startedCount() < WORKERS && waited < GUARD) {
        waited = waited + 1;
    }

    // The storm, then bounded retries. Every attempt signals the workers first, so the loads
    // keep racing even the retries. Each attempt either commits (0), is refused because a paced
    // class was still initializing (26), or is refused because one was published after the
    // snapshot (27); any other value is a bug.
    let ok = 0;
    let refused = 0;
    let rc: int = -1;
    const attempts: int = RELOADS + RETRY_LIMIT;
    for (let a = 0; a < attempts; a++) {
        Shared.seq[0] = a + 1;
        rc = HotReloadBridge.hotReloadNative(a0, a1);
        if (rc == 0) {
            ok = ok + 1;
        } else if (rc == ERR_CLASS_INITIALIZING || rc == ERR_CONCURRENT_LOADING) {
            refused = refused + 1;
        } else {
            console.println('unexpected rc=' + rc);
            return 1;
        }
        // A success ends the loop only once the retries have begun: the storm must run to the
        // end so the workers' pacing covers it, and the final success is then decided by the
        // retries.
        if (a >= RELOADS && rc == 0) {
            break;
        }
    }
    if (rc != 0) {
        console.println('no successful reload');
        return 1;
    }

    // The workers finish their remaining (unsignalled) loads from the published file; no reload
    // runs after the final success, so a class it silently missed stays on the old generation.
    for (let i = 0; i < jobs.length; i++) {
        jobs[i].Await();
    }

    // EVERY class of the target file must now report the new generation: those the final success
    // collected were swapped, those loaded after it read the published file. A class published
    // during the final success's own window is the one this case exists for: with the quiesce
    // gate that attempt was refused and a later one swept the class; without it the attempt
    // reported success, the class stayed old, and it shows up right here.
    for (let k = 0; k < WORKERS * PER_WORKER; k++) {
        if (Shared.loader.get(k) != 'C2') {
            console.println('stale=' + k);
            return 1;
        }
    }
    console.println('ok=' + ok + ' refused=' + refused);
    console.println('all=C2');
    return 0;
}
}

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
 * Hotreload multi-coroutine smoke test, version 1.
 *
 * What it is for: every other case reloads while the VM has exactly one mutator, so the
 * stop-the-world part of the transaction never has to wait for anybody. Here N coroutines are
 * spinning in managed code when the reload happens, so the commit phase really has to suspend them
 * at a safepoint and resume them afterwards.
 *
 * Results are per worker, not shared: each one returns the last value IT observed, and `main`
 * checks every returned value. A single shared "last seen" variable would be satisfied by one
 * worker alone and would additionally be a data race.
 *
 * Run with `--coroutine-workers-count=N` to choose how many real worker threads exist.
 *
 * argv: <targetPath> <patchPath> <mode>, where mode is
 *   "wait"   -- reload only once every worker is spinning (needs real worker threads)
 *   "nowait" -- reload immediately, i.e. while the coroutines exist but have not started
 */

import { launch } from 'std/concurrency'

const WORKERS: int = 4;
// Hang guard only, deliberately far more iterations than a worker can execute before the
// transaction commits. A worker leaves as soon as `main` reports the reload is over; if it were
// able to exhaust this loop first it would report the old body for a reason that has nothing to do
// with the swap, which is exactly how an earlier version of this case fooled itself.
const SPINS: int = 2000000000;
// Bound on the "wait until the workers are running" spin, so a scheduling hiccup can never turn
// this test into a hang. Only used when argv asks to wait.
const READY_WAIT_LIMIT: int = 2000000000;

class Calc {
    // the patch changes the arithmetic -> proves the swap reached the running coroutines
    public static calc(): int {
        return 1;
    }
}

class Shared {
    // one slot per worker: distinct slots, so "has it started" needs no atomics
    public static started: FixedArray<int> = new FixedArray<int>(WORKERS);
    // set by `main` once the transaction has returned
    public static reloadDone: int = 0;
}

/*
 * Each worker must still be spinning when the transaction commits, and must make at least one more
 * call afterwards -- otherwise it could finish early and report the old value for a reason that has
 * nothing to do with the swap. So it keeps calling until `main` says the reload is over, then makes
 * one final observation and returns it.
 *
 * The flag is a plain field, so this relies on the write eventually becoming visible; `SPINS` is
 * the backstop that turns a lost update into a failed assertion rather than a hang.
 *
 * The returned value carries TWO independent observations, which is the whole point of the bias:
 *
 *   the tens digit  -- `Calc.calc()`, i.e. did the CALL this body makes reach the new code
 *   the hundreds    -- a literal in THIS function's own body, i.e. is the frame itself running the
 *                      new `worker`
 *
 * They separate on purpose. A coroutine already executing when the world stopped keeps its frame
 * and finishes on the old body -- documented behaviour, not a defect -- so it reports 42. A
 * coroutine that had not started yet dispatches through the function value taken at `launch` time;
 * the value's synthesised class is reloaded with everything else and its call to `worker` resolves
 * after the commit, so it must report 142. It used to report 42: the resolution hit a stale entry
 * in the patch file's own cache, written while the temporary classes were prepared against the old
 * generation. In the first version of this case `worker` was byte-identical in both versions, so
 * the only thing it ever measured was the callee.
 */
function worker(id: int): int {
    Shared.started[id] = 1;
    let last = 0;
    for (let i = 0; i < SPINS; i++) {
        last = Calc.calc();
        if (Shared.reloadDone == 1) {
            return Calc.calc() + 0;
        }
    }
    return last + 0;
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

    /*
     * With real worker threads, wait until every worker is inside its loop: otherwise the reload
     * could win the race and the case would silently degrade into the single-mutator one it is
     * meant to go beyond. With a single worker the coroutines are fibers queued behind this one and
     * cannot start at all until it yields, so waiting would only burn the budget -- there the point
     * of the case is precisely that they have NOT started.
     */
    if (a2 == 'wait') {
        let waited = 0;
        while (startedCount() < WORKERS && waited < READY_WAIT_LIMIT) {
            waited = waited + 1;
        }
    }
    console.println('started=' + startedCount());

    let rc = HotReloadBridge.hotReloadNative(a0, a1);
    Shared.reloadDone = 1;
    console.println('rc=' + rc);

    // Every worker ran past the reload, so each of them must have ended on the new body
    let onNewBody = 0;
    let onNewEntry = 0;
    let done = 0;
    for (let i = 0; i < jobs.length; i++) {
        let seen = jobs[i].Await();
        console.println('  w' + i + '=' + seen);
        done = done + 1;
        if (seen % 100 == 42) {
            onNewBody = onNewBody + 1;
        }
        if (seen >= 100) {
            onNewEntry = onNewEntry + 1;
        }
    }
    console.println('done=' + done);
    console.println('onNewBody=' + onNewBody);
    console.println('onNewEntry=' + onNewEntry);
    return 0;
}
}

/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

const { 
    MODULE_PATH,
    ARK_ETS_STDLIB_PATH,
    ARK_ETS_INTEROP_JS_GTEST_ABC_PATH,
    VMB_BENCH_UNIT_ITERATIONS,
    ETS_VM_OPTS
} = process.env;

const DEFAULT_ITERATIONS = 100000;

globalThis.require = require;

class Benchmark {
    #ark = require(MODULE_PATH + '/ets_interop_js_napi.node');
    #benchmark;
    #passedTime = BigInt(0);
    #binaries = [];
    constructor(...binaries) {
        this.#binaries = binaries;
        this.#__init();
    }

    #__init() {
        const isOk = this.#ark.createRuntime(Object.assign({
            'boot-panda-files': [ARK_ETS_STDLIB_PATH, ...this.#binaries].join(':'),
            'panda-files': this.#binaries.join(':')
       }, JSON.parse(ETS_VM_OPTS)));

        if (!isOk) {
            throw new Error('Failed to initialize');
        }
    }

    #prepare() {
        const TestClass = this.#ark.getClass("L$STATE" + "_" + "$METHOD" + "/$STATE;");
        this.#benchmark = new TestClass();
        this.#benchmark.setup();
    }

    #loopFromJS(iterations = DEFAULT_ITERATIONS) {
        if (!Number.isFinite(iterations)) {
            throw new Error(`Iteration count should be a valid integer, $${iterations} given instead`);
        }
        for (let i = 0; i < iterations ?? this.iterations; i++) {
            const startTimeNsec = process.hrtime.bigint();
            this.#benchmark.$METHOD();
            const endTimeNSec = process.hrtime.bigint();
            this.#passedTime += (endTimeNSec - startTimeNsec);
        }
        return this.#passedTime;
    }

    #loopFromArk(iterations = DEFAULT_ITERATIONS) {
        if (!Number.isFinite(iterations)) {
            throw new Error(`Iteration count should be a valid integer, $${iterations} given instead`);
        }
        this.#benchmark.runsLeft = iterations;
        return Number(this.#benchmark.$METHOD());
    }

    run(iterations = DEFAULT_ITERATIONS) {
        this.#prepare();
        const isSelfTimed = this.#benchmark.totalTime !== undefined;
        const testRunner = isSelfTimed ? this.#loopFromArk.bind(this) : this.#loopFromJS.bind(this);
        const timing = testRunner(iterations);
        console.log("Benchmark result: $NAME " + timing);
    }
}

const benchmark = new Benchmark(ARK_ETS_INTEROP_JS_GTEST_ABC_PATH);

const iterations = (VMB_BENCH_UNIT_ITERATIONS && Number.isFinite(Number(VMB_BENCH_UNIT_ITERATIONS))) 
    ? Number(VMB_BENCH_UNIT_ITERATIONS)
    : DEFAULT_ITERATIONS;

benchmark.run(iterations);
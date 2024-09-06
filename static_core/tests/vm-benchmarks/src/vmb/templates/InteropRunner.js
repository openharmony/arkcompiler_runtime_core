/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
    VMB_BENCH_UNIT_NAME,
    VMB_BENCH_UNIT_ITERATIONS
} = process.env;

const DEFAULT_ITERATIONS = 100000;

globalThis.require = require;

const capitalize = (str = '') => {
    return str.split('_')
        .filter(Boolean)
        .map((substring, i) =>
            substring[0].toUpperCase() + substring.substring(1))
        .join('');
};

class Benchmark {
    #ark = require(MODULE_PATH + '/ets_interop_js_napi.node');
    #benchName = capitalize(VMB_BENCH_UNIT_NAME);
    #benchmark;
    #passedTime = BigInt(0);
    #binaries = [];
    constructor(...binaries) {
        this.#binaries = binaries;
        this.#__init();
    }

    #__init() {
        const isOk = this.#ark.createRuntime({
            'boot-panda-files': [ARK_ETS_STDLIB_PATH, ...this.#binaries].join(':'),
            'panda-files': this.#binaries.join(':'),
            'gc-trigger-type': 'heap-trigger',
            'compiler-enable-jit': 'false',
            'run-gc-in-place': 'true',
            'log-components': 'ets_interop_js',
            'load-runtimes': 'ets'
        });

        if (!isOk) {
            throw new Error('Failed to initialize');
        }
    }

    #prepare() {
        const TestClass = this.#ark.getClass(`L${this.#benchName};`);
        this.#benchmark = new TestClass();
        this.#benchmark.setup();
    }

    #loopFromJS(iterations = DEFAULT_ITERATIONS) {
        if (!Number.isFinite(iterations)) {
            throw new Error(`Iteration count should be a valid integer, ${iterations} given instead`);
        }
        for (let i = 0; i < iterations ?? this.iterations; i++) {
            const startTimeNsec = process.hrtime.bigint();
            this.#benchmark.test();
            const endTimeNSec = process.hrtime.bigint();
            this.#passedTime += (endTimeNSec - startTimeNsec);
        }
        return this.#passedTime;
    }

    #loopFromArk(iterations = DEFAULT_ITERATIONS) {
        if (!Number.isFinite(iterations)) {
            throw new Error(`Iteration count should be a valid integer, ${iterations} given instead`);
        }
        this.#benchmark.runsLeft = iterations;
        return Number(this.#benchmark.test());
    }

    run(iterations = DEFAULT_ITERATIONS) {
        this.#prepare();
        const isSelfTimed = this.#benchmark.totalTime !== undefined;
        const testRunner = isSelfTimed ? this.#loopFromArk.bind(this) : this.#loopFromJS.bind(this);
        const timing = testRunner(iterations);
        console.log(`Benchmark result: ${this.#benchName.toLowerCase()} ` + timing);
    }
}

const benchmark = new Benchmark(ARK_ETS_INTEROP_JS_GTEST_ABC_PATH);

const iterations = (VMB_BENCH_UNIT_ITERATIONS && Number.isFinite(Number(VMB_BENCH_UNIT_ITERATIONS))) 
    ? Number(VMB_BENCH_UNIT_ITERATIONS)
    : DEFAULT_ITERATIONS;

benchmark.run(iterations);
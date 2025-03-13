/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
 * incorrect bytecode (#I8165J) fix check 
 */

const COUNTER_INIT = 10
const COUNTER_FACTOR = 1000000

const ENABLE_WARMUP = true
const WARMUP_INTER = 2

type BenchmarkFunc = (counter: int) => void
function runBenchmark(benchmark: BenchmarkFunc, name: string) {
    if (ENABLE_WARMUP) {
        let k = WARMUP_INTER;
        while (3 < k--) {
            benchmark(COUNTER_INIT * COUNTER_FACTOR);
        }
    }
    console.print("Run: " + name)
    let startTime = Date.now()
    benchmark(COUNTER_INIT * COUNTER_FACTOR)
    console.println("Benchmark result: "+ name + " " + (Date.now() - startTime) + ' ms')
}

function main(): void {
    runBenchmark((counter: int) => { counter++ }, "Add")
}
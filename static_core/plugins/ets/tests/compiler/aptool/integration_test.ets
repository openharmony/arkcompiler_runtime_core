/**
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

/**
 * Integration test file for aptool.
 * This file exercises profiling features:
 * - Inline caches (polymorphic calls)
 * - Branch profiling
 * - Throw profiling
 */

interface Worker {
    process(value: double): double;
}

class Adder implements Worker {
    process(value: double): double {
        return value + 1.0;
    }
}

class Doubler implements Worker {
    process(value: double): double {
        return value * 2.0;
    }
}

class Tripler implements Worker {
    process(value: double): double {
        return value * 3.0;
    }
}

// Function to test branch profiling
function noisyBranch(limit: double): double {
    let sum: int = 0;
    for (let i: int = 0; i < Double.toInt(limit); i++) {
        // Branch 1: taken once at i=0, not taken otherwise
        if (i == 0) {
            sum += 100;
        }
        // Branch 2: taken when i is even
        if (i % 2 == 0) {
            sum += i;
        } else {
            sum -= i;
        }
        // Branch 3: taken when i divisible by 3
        if (i % 3 == 0) {
            sum += 10;
        }
    }
    return Int.toDouble(sum);
}

// Function to test polymorphic inline caches
function runPolymorphic(workers: Worker[], iterations: double): double {
    let result: double = 0.0;
    for (let i: int = 0; i < Double.toInt(iterations); i++) {
        let idx = i % workers.length;
        let worker = workers[idx];
        // Polymorphic call site - will record multiple target classes
        result += worker.process(Int.toDouble(i));
    }
    return result;
}

// Function to test throw profiling
function mayThrow(value: double): double {
    if (value < 0) {
        throw new Error("negative value");
    }
    return value * value;
}

// Function that combines all profiling scenarios
function foo(input: double): double {
    // Test throw profiling - call with positive value (no throw)
    let squared = mayThrow(input);

    // Test branch profiling
    let branchResult = noisyBranch(input);

    // Create polymorphic workers array
    let workers: Worker[] = [new Adder(), new Doubler(), new Tripler()];

    // Test polymorphic inline caches
    let polyResult = runPolymorphic(workers, input);

    return squared + branchResult + polyResult;
}

function main(): int {
    // Run multiple iterations to ensure profiling data is collected
    let result: double = 0.0;
    for (let i: int = 0; i < 100; i++) {
        result += foo(10.0);
    }

    // Test with exception handling
    try {
        mayThrow(-1.0);
    } catch (e) {
        // Expected exception
        result += 1.0;
    }

    // Print result to ensure the code is not optimized away
    console.log("Result: " + result.toString());

    return 0;
}

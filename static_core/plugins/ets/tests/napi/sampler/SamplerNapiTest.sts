/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

function main(): int {
    loadLibrary("sampler_etsnapi_lib");
    
    let res: int = 0;
    res = CallNativeSlowFunction(20000000);
    if (res != 20000000) {
        // throw new Exception("ERROR: CallNativeSlowFunction worked wrong");
        return 1;
    }

    res = CallNativeFastFunction(100000);
    if (res != 100000) {
        // throw new Exception("ERROR: CallNativeFastFunction worked wrong");
        return 2;
    }

    res = CallNativeNAPISlowFunction(500000);
    if (res != 500000) {
        // throw new Exception("ERROR: CallNativeNAPISlowFunction worked wrong");
        return 3;
    }

    res = CallNativeNAPIFastFunction(100000);
    if (res != 100000) {
        // throw new Exception("ERROR: CallNativeNAPIFastFunction worked wrong");
        return 4;
    }

    return 0;
}

function release_main(): int {
    loadLibrary("sampler_etsnapi_lib");
    
    let res: int = 0;
    res = CallNativeSlowFunction(2000000000);
    // res = 2 billion
    if (res != 2000000000) {
        // throw new Exception("ERROR: CallNativeSlowFunction worked wrong");
        return 1;
    }

    res = CallNativeFastFunction(5000000);
    if (res != 5000000) {
        // throw new Exception("ERROR: CallNativeFastFunction worked wrong");
        return 2;
    }

    res = CallNativeNAPISlowFunction(5000000);
    if (res != 5000000) {
        // throw new Exception("ERROR: CallNativeNAPISlowFunction worked wrong");
        return 3;
    }

    res = CallNativeNAPIFastFunction(1000000);
    if (res != 1000000) {
        // throw new Exception("ERROR: CallNativeNAPIFastFunction worked wrong");
        return 4;
    }

    return 0;
}

function CallNativeSlowFunction(iterations: int): int {
    return NativeSlowFunction(iterations);
}

function CallNativeFastFunction(iterations: int): int {
    let ret: int = 0;
    for (let i: int = 0; i < iterations; i++) {
        ret += NativeFastFunction();
    }
    return ret;
}

function CallNativeNAPISlowFunction(iterations: int): int {
    return NativeNAPISlowFunction(iterations);
}

function SlowETSFunction(iterations: int): int {
    let ret: int = 0;
    for (let i: int = 0; i < iterations; i++) {
        ret++;
    }
    for (let i: int = 0; i < iterations; i++) {
        ret = ret ^ i;
    }
    for (let i: int = 0; i < iterations; i++) {
        ret = ret ^ i;
    }
    return ret;
}

function CallNativeNAPIFastFunction(iterations: int): int {
    return NativeNAPIFastFunction(iterations);
}

function FastETSFunction(): int {
    let ret: int = 1;
    return ret;
}

native function NativeSlowFunction(iterations: int): int;
native function NativeFastFunction(): int;
native function NativeNAPISlowFunction(iterations: int): int;
native function NativeNAPIFastFunction(iterations: int): int;

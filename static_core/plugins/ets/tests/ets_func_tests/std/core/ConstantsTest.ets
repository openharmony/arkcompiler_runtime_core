/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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


function test(result: int, message: String ): int {
    if(result == 0) {
      return 0;      
    }
    console.println("FAILED: " + message);
    return 1;
}

function main(): int {
    let failures = 0;

    failures += test(testShort(), "Try to set min/max value to short");
    failures += test(testByte(), "Try to set min/max value to byte");
    failures += test(testInt(), "Try to set min/max value to int");
    failures += test(testLong(), "Try to set min/max value to long");
    failures += test(testFloat(), "Try to set min/max value to float");
    failures += test(testDouble(), "Try to set min/max value to double");

    return test(failures, "All tests run");
}

function testShort() {
    try {
        let res: short = Short.MAX_VALUE;
        res = Short.MIN_VALUE;
    } catch (e) {
        return 1
    }
    return 0;
}

function testByte() {
    try {
        let res: byte = Byte.MAX_VALUE;
        res = Byte.MIN_VALUE;
    } catch (e) {
        return 1
    }
    return 0;
}

function testInt() {
    try {
        let res: int = Int.MAX_VALUE;
        res = Int.MIN_VALUE;
    } catch (e) {
        return 1
    }
    return 0;
}

function testLong() {
    try {
        let res: long = Long.MAX_VALUE;
        res = Long.MIN_VALUE;
    } catch (e) {
        return 1
    }
    return 0;
}

function testFloat() {
    try {
        let res: float = Float.MAX_VALUE;
        res = Float.MIN_VALUE;
    } catch (e) {
        return 1
    }
    return 0;
}

function testDouble() {
    try {
        let res: double = Double.MAX_VALUE;
        res = Double.MIN_VALUE;
    } catch (e) {
        return 1
    }
    return 0;
}

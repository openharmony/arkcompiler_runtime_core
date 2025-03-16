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


// ArkTS implementations of hash functions

const hello16 = "hello world hello world hello world hello world "
              + "hello world hello world hello world hello world "
              + "hello world hello world hello world hello world "
              + "hello world hello world hello world hello world";
const brefStr = "Short String";

function crc32(s: String): int {
    // TODO: unsigned types are not ready yet
    let crc: int = 0xFFFFFFFF;
    let poly: int = 0xEDB88320;
    for (let i: int = 0; i < s.length; i++) {
        let c: int = s.charAt(i) as int;
        let tmp: int = (crc ^ c) & 0xFF;
        for (let j: int = 0; j < 8; j++) {
            if ((tmp & 1) == 1) {
                tmp = (tmp >>> 1) ^ poly;
            } else {
                tmp = (tmp >>> 1);
            }
        }
        crc = (crc >>> 8) ^ tmp;
    }
    return ~crc;
}

function test(result: boolean, name: String): int {
    if (result) {
        console.println("PASSED: " + name);
        return 0;
    }
    console.println("FAILED: " + name);
    return 1;
}

function main(): int {
    let failures: int = 0;
    failures += test(
        0xebe93d8a == crc32(brefStr),
        "CRC32 short string");
    failures += test(
        0x890f6765 == crc32(hello16),
        "CRC32 long string");
    if (failures == 0) {
        console.println("PASSED: All tests run");
    } else {
        console.println("FAILED: All tests run");
    }
    return failures;
}

/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

import {util} from "@ohos.util";

const success: int = 0;
const fail: int = 1;

function check(result: int, message: string): int {
    if (result == 0) {
        return success;
    }
    console.println("\nFAILED: " + message);
    return fail;
}

function main(): int {
    let failures: int = 0
    failures += check(testGenerateRandomUUID(), "Test testGenerateRandomUUID() for RFC4122 compliance")
    failures += check(testIsUniqueUUID(), "Test generated UUID's by generateRandomUUID() are unique")
    failures += check(testGetNonCachedUUID(), "Test get non-cached UUID")
    failures += check(testGetCachedUUID(), "Test get cached UUID")
    return failures
}

function testGenerateRandomUUID(): int {
    let id = util.generateRandomUUID()
    assertEQ(id.length, 36)
    let delimiterIdx = new Array<int>(8, 13, 18, 23)
    const hexDigits = "0123456789abcdef"
    for (let i = 0; i < 36; ++i) {
       if (delimiterIdx.includes(i)) {
           assertEQ(id[i] as string, "-")
       } else {
           assertTrue(hexDigits.includes(id[i] as string))
       }
    }

    const versionByte = 14
    assertEQ(id[versionByte], c'4', "Version byte has to be 0x4")
    const withReservedBits = 19
    assertTrue(id[withReservedBits] >= c'8' && id[withReservedBits] <= c'b',
        id[withReservedBits] + " are not valid reserved bits from range 0x8..0xb!")
    return success
}

function testIsUniqueUUID(): int {
    let idSet: Set<string> = new Set<string>()
    for (let i = 0; i < 1000; ++i) {
        let id = util.generateRandomUUID()
        assertFalse(idSet.has(id))
        idSet.add(id)
    }
    return success
}

function testGetNonCachedUUID(): int {
    let idSet: Set<string> = new Set<string>()
    for (let i = 0; i < 1000; ++i) {
        let id = util.generateRandomUUID(false)
        assertFalse(idSet.has(id))
        idSet.add(id)
    }
    return success
}

function testGetCachedUUID(): int {
    let idSet: Set<string> = new Set<string>()
    for (let i = 0; i < 1000; ++i) {
        let id = util.generateRandomUUID(true)
        if (i > 0) {
            assertTrue(idSet.has(id))
        }
        idSet.add(id)
        assertEQ(idSet.size, 1)
    }
    return success
}

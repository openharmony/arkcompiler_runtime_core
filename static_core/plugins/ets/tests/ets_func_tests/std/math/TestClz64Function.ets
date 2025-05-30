/*
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

function test_clz64_long(): void {
    let result : long = 0xFFFFFFFFFFFFFFFF
    assertEQ(clz64(result), 0);
}

function test_clz64_int(): void {
    let result : int = 0x00000000FFFFFFFF
    let result1 : int = -1
    assertEQ(clz64(result), 32);
    assertEQ(clz64(result1), 32);
}

function test_clz64_short(): void {
    let result : short = 32767
    let result1 : short = -1
    assertEQ(clz64(result), 49);
    assertEQ(clz64(result1), 48);
}

function test_clz64_byte(): void {
    let result : byte = -1
    let result1 : byte = 255 as byte
    let result2 : byte = 127
    assertEQ(clz64(result), 56);
    assertEQ(clz64(result1), 56);
    assertEQ(clz64(result2), 57);
}

function test_clz64_char(): void {
    let result : char = c'0'
    assertEQ(clz64(result), 58);
}

function test_clz64_hex(): void {
    let result = 0xFFFFFFFFFFFFFFFF
    let result1 = 0x00000000FFFFFFFF
    assertEQ(clz64(result), 0);
    assertEQ(clz64(result1), 32);
}

function test_clz64_oct(): void {
    let result = 0o37777777777
    let result1 = 0o177777
    let result2 = 0o1777777777777777777777
    assertEQ(clz64(result), 32);
    assertEQ(clz64(result1), 48);
    assertEQ(clz64(result2), 0);
}

function test_clz64_bin(): void {
    let result = 0b1111111111111111111111111111111111111111111111111111111111111111
    let result1 = 0b0000000000000000000000000000000011111111111111111111111111111111
    assertEQ(clz64(result), 0);
    assertEQ(clz64(result1), 32);
}

function main(): void {
    test_clz64_long();
    test_clz64_int();
    test_clz64_short();
    test_clz64_byte();
    test_clz64_char();
    test_clz64_hex();
    test_clz64_oct();
    test_clz64_bin();
}

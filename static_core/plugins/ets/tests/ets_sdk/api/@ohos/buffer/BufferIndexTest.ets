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

import buffer from "@ohos.buffer";

function main(): int {
    const suite = new ArkTestsuite("Buffer indexer methods")

    suite.addTest("Test get via indexer", testIndexerGet)
    suite.addTest("Test set via indexer", testIndexerSet)
    suite.addTest("Test combined get and set via indexer", testIndexerGetSet)
    suite.addTest("Test indexer out of bounds", testIndexerOutOfBounds)

    return suite.run()
}

function testIndexerGet() {
    const buf = buffer.from([5, 10, 15, 20])
    assertEQ(buf[0], 5)
    assertEQ(buf[2], 15)
}

function testIndexerSet() {
    const buf = buffer.alloc(4)
    buf[1] = 100
    buf[3] = 255
    assertEQ(buf[1], 100)
    assertEQ(buf[3], 255)
}

function testIndexerGetSet() {
    const buf = buffer.from("abcd", "utf8")
    buf[0] = 0x78  // 'x'
    buf[3] = 0x79  // 'y'
    assertEQ(buf[0], 0x78)
    assertEQ(buf[3], 0x79)
    assertEQ(buf.toString(), "xbcy")
}

function testIndexerOutOfBounds() {
    const buf = buffer.from([1, 2, 3])
    assertEQ(buf[-1], undefined)
    assertEQ(buf[5], undefined)
    buf[10] = 99
    assertEQ(buf.length, 3)
    assertEQ(buf.toString(), "\u0001\u0002\u0003")
}

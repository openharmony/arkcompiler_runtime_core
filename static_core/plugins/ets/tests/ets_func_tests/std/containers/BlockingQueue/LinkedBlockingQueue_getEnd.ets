/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
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
    let LinkedBlockingQueueTestsuite = new ArkTestsuite("LinkedBlockingQueueGetEndTest")
    LinkedBlockingQueueTestsuite.addTest("getEndFill", getEndFillTest)
    LinkedBlockingQueueTestsuite.addTest("getEndMoreType", getEndMoreTypeTest)
    LinkedBlockingQueueTestsuite.addTest("getEndEmpty", getEndEmptyTest)
    LinkedBlockingQueueTestsuite.addTest("getEndOne", getEndOneTest)
    LinkedBlockingQueueTestsuite.addTest("getEndCapacity", getEndCapacityTest)
    return LinkedBlockingQueueTestsuite.run()
}

function getEndFillTest() {
    let testBQ = new LinkedBlockingQueue<int>(10)
    for (let i = 0; i < testBQ.capacity; i++) {
        testBQ.add(i)
    }
    assertEQ(testBQ.size, testBQ.capacity)
    assertEQ(testBQ.getEnd(), testBQ.capacity - 1)
}

function getEndMoreTypeTest() {
    let testBQ = new LinkedBlockingQueue<int | null | undefined | string>(10)
    testBQ.add(1)
    assertEQ(testBQ.getEnd(), 1)
    testBQ.add(null)
    assertEQ(testBQ.getEnd(), null)
    testBQ.add(undefined)
    assertEQ(testBQ.getEnd(), undefined)
    testBQ.add("\n")
    assertEQ(testBQ.getEnd(), "\n")
    testBQ.add("\t")
    assertEQ(testBQ.getEnd(), "\t")
    assertEQ(testBQ.size, 5)
}

function getEndEmptyTest() {
    let testBQ = new LinkedBlockingQueue<int>(10)
    assertTrue(testBQ.isEmpty())
    assertEQ(testBQ.getEnd(), null)
}

function getEndOneTest() {
    let testBQ = new LinkedBlockingQueue<int>(1)
    testBQ.add(1)
    assertEQ(testBQ.size, 1)
    assertEQ(testBQ.getEnd(), 1)
}

function getEndCapacityTest() {
    let testBQ = new LinkedBlockingQueue<int>(10)
    testBQ.add(0)
    assertEQ(testBQ.getEnd(), 0)
    assertEQ(testBQ.size, 1)
    for (let i = 1; i < testBQ.capacity - 1; i++) {
        assertEQ(testBQ.size, i)
        testBQ.add(i)
        assertEQ(testBQ.getEnd(), i)
    }
    assertEQ(testBQ.getEnd(), testBQ.capacity - 2)
    assertEQ(testBQ.size, testBQ.capacity - 1)
    testBQ.add(testBQ.capacity - 1)
    assertEQ(testBQ.getEnd(), testBQ.capacity - 1)
    assertEQ(testBQ.size, testBQ.capacity)
}

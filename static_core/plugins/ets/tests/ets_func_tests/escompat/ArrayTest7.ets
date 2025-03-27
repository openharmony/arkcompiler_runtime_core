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

function main() throws {
    testArraySplice()
}

function testArraySplice() {
    {
        const arr = new Array<number>(1, 2, 3, 4, 5);
        const removed = arr.splice(1, 2);
        assertEQ(arr.length, 3)
        assertEQ(arr[0], 1)
        assertEQ(arr[1], 4)
        assertEQ(arr[2], 5)
        assertEQ(removed.length, 2)
        assertEQ(removed[0], 2)
        assertEQ(removed[1], 3)
    }
    {
        const arr = new Array<number>(1, 2, 3);
        const removed = arr.splice(1, 0, 4, 5);
        assertEQ(arr.length, 5)
        assertEQ(arr[0], 1)
        assertEQ(arr[1], 4)
        assertEQ(arr[2], 5)
        assertEQ(arr[3], 2)
        assertEQ(arr[4], 3)
        assertEQ(removed.length, 0)
    }
    {
        const arr = new Array<number>(1, 2, 3);
        const removed = arr.splice(1, undefined, 4, 5);
        assertEQ(arr.length, 5)
        assertEQ(arr[0], 1)
        assertEQ(arr[1], 4)
        assertEQ(arr[2], 5)
        assertEQ(arr[3], 2)
        assertEQ(arr[4], 3)
        assertEQ(removed.length, 0)
    }
    {
        const arr = new Array<number>(1, 2, 3, 4);
        const removed = arr.splice(1, 2, 5, 6);
        assertEQ(arr.length, 4)
        assertEQ(arr[0], 1)
        assertEQ(arr[1], 5)
        assertEQ(arr[2], 6)
        assertEQ(arr[3], 4)
        assertEQ(removed.length, 2)
        assertEQ(removed[0], 2)
        assertEQ(removed[1], 3)
    }
    {
        const arr = new Array<number>();
        const removed = arr.splice(0, 0, 1, 2);
        assertEQ(arr.length, 2)
        assertEQ(arr[0], 1)
        assertEQ(arr[1], 2)
        assertEQ(removed.length, 0)
    }
    {
        const arr = new Array<number>(1, 2, 3);
        const removed = arr.splice(0, 3);
        assertEQ(arr.length, 0)
        assertEQ(removed.length, 3)
        assertEQ(removed[0], 1)
        assertEQ(removed[1], 2)
        assertEQ(removed[2], 3)
    }
    {
        const arr = new Array<number>(1, 2, 3, 4, 5);
        const removed = arr.splice(2);
        assertEQ(arr.length, 2)
        assertEQ(arr[0], 1)
        assertEQ(arr[1], 2)
        assertEQ(removed.length, 3)
        assertEQ(removed[0], 3)
        assertEQ(removed[1], 4)
        assertEQ(removed[2], 5)
    }
    {
        const arr = new Array<number>(1, 2, 3);
        const removed = arr.splice(1, undefined);
        assertEQ(arr.length, 3)
        assertEQ(arr[0], 1)
        assertEQ(arr[1], 2)
        assertEQ(arr[2], 3)
        assertEQ(removed.length, 0)
    }
}

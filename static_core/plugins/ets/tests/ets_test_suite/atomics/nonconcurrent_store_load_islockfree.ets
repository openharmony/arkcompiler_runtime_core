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


function main() {
    let buf = new ArrayBuffer(4);
    let arr = new Int8Array(buf, 0, 1);

    Atomics.store(arr, 0, 10);
    assertEQ(Atomics.load(arr, 0), 10)

    // isLockFree testing
    assertTrue(Atomics.isLockFree(1) && Atomics.isLockFree(2))
    assertTrue(Atomics.isLockFree(4) && Atomics.isLockFree(8))
    assertTrue(!Atomics.isLockFree(3) && !Atomics.isLockFree(5))
    assertTrue(!Atomics.isLockFree(6) && !Atomics.isLockFree(7))
}

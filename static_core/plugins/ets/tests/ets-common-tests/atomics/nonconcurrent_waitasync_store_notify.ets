/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

let buf: ArrayBuffer;
let arr: Int32Array;

function main() {
    let c = new Console()

    buf = new ArrayBuffer(4);
    arr = new Int32Array(buf, 0, 1);

    let promise: Promise<string> = Atomics.waitAsync(arr, 0, new BigInt(0));
    Atomics.store(arr, 0, 5);
    let count = Atomics.notify(arr, 0);
    let res: string = await promise;
    assertTrue(res == "ok" || res == "not-equal")
}

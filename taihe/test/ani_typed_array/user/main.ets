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
import * as bar from "@generated/bar";
import * as foo from "@generated/foo";

loadLibrary("ani_typed_array");

function main() {
    let ab = new ArrayBuffer(8);
    ab.set(0, 0x01 as byte);
    ab.set(1, 0x23 as byte);
    ab.set(2, 0x45 as byte);
    ab.set(3, 0x67 as byte);
    ab.set(4, 0x89 as byte);
    ab.set(5, 0xab as byte);
    ab.set(6, 0xcd as byte);
    ab.set(7, 0xef as byte);
    let ua = new Uint16Array(ab, 2, 2);

    foo.inArrayBuffer(ab);
    let x = foo.outArrayBuffer();
    console.log(x);

    bar.inUint16Array(ua);
    let y = bar.outUint16Array();
    console.log(y);
    console.log(y.byteLength);
    console.log(y.byteOffset);
    console.log(y.buffer);
}

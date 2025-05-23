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

export function createArrayBuffer(length: int): ArrayBuffer {
    let buff = new ArrayBuffer(length);
    return buff;
}

export function createSlicedArrayBuffer(buff: ArrayBuffer, begin: int, end: int): ArrayBuffer {
    let sliced = buff.slice(begin, end);
    return sliced;
}

export function getLength(buff: ArrayBuffer): int {
    return buff.byteLength as int;
}

export function getByte(buff: ArrayBuffer, idx: int): int {
    return buff.at(idx) as int;
}

export function setByte(buff: ArrayBuffer, idx: int, val: byte): int {
    buff.set(idx, val);
    return 0;
}

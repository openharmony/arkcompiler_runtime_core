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

function GetArrayBuffer(maxByteLength?: int) : ArrayBuffer
{
    let arrayBuffer = new ArrayBuffer(10, maxByteLength);
    let vals : byte[] = [1, 2, 3];
    arrayBuffer.set(0, vals[0]);
    arrayBuffer.set(1, vals[1]);
    arrayBuffer.set(2, vals[2]);
    return arrayBuffer;
}

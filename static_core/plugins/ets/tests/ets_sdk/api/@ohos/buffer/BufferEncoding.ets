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

import buffer from "@ohos.buffer"

type ExpectedBufferEncoding =
    'ascii' | 'utf8' | 'utf-8' | 'utf16le' | 'ucs2' | 'ucs-2' |
    'base64' |'base64url' | 'latin1' | 'binary' | 'hex';


function main(): int {

    const validEncTyp =
        ['ascii', 'utf8', 'utf-8', 'utf16le', 'ucs2', 'ucs-2', 'base64', 'base64url', 'latin1', 'binary', 'hex']

    for (let i = 0; i < validEncTyp.length; ++i) {
        let a = validEncTyp[i] as buffer.BufferEncoding
        let b: ExpectedBufferEncoding = a
        a = b
        b = a
    }

    return 0
}

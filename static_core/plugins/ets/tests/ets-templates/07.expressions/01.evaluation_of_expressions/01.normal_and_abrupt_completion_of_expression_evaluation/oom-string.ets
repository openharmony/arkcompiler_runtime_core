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

/*---
desc: OutOfMemoryError is thrown by string allocation
ark_options: ['--heap-size-limit=8388608', '--init-heap-size-limit=8388608', '--gc-type=stw']
timeout: 60
tags: [no-warmup]
---*/

const STR_ARRAY_SIZE = 250

function main() {
    let s: string = "lolo"
    try {
        for (let i = 1; i < STR_ARRAY_SIZE; i++) {
            s = s + s
        }
    } catch (e) {
        arktest.assertTrue(e instanceof OutOfMemoryError, 'OutOfMemoryError was expected');
        return
    }
    arktest.assertTrue(false, 'OutOfMemoryError was expected');
}

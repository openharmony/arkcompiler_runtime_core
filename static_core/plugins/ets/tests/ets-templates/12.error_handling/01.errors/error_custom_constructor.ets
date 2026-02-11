/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
desc: Define custom error class with constructor that wraps another error.
---*/

class UnknownError extends Error {
    error: Error
    constructor (error: Error) {
        super()
        this.error = error
    }
}

function main(): void {
    let originalError = new Error("original cause")
    let wrapped = new UnknownError(originalError)

    arktest.assertTrue(wrapped instanceof Error)
    arktest.assertTrue(wrapped instanceof UnknownError)
    arktest.assertEQ(wrapped.error, originalError)
    arktest.assertEQ(wrapped.error.message, "original cause")
}

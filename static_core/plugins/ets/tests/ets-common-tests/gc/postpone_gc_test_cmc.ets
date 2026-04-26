/**
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

// Both postponeGCStart() and postponeGCEnd() must throw UnsupportedOperationError.

function main(): void {
    let startErrorCaught: boolean = false;
    try {
        GC.postponeGCStart();
    } catch (e) {
        if (e instanceof UnsupportedOperationError) {
            startErrorCaught = true;
        } else {
            arktest.assertTrue(false, 'Unexpected error type from postponeGCStart');
        }
    }
    arktest.assertTrue(startErrorCaught, 'CMC GC must throw UnsupportedOperationError for postponeGCStart');

    let endErrorCaught: boolean = false;
    try {
        GC.postponeGCEnd();
    } catch (e) {
        if (e instanceof UnsupportedOperationError) {
            endErrorCaught = true;
        } else {
            arktest.assertTrue(false, 'Unexpected error type from postponeGCEnd');
        }
    }
    arktest.assertTrue(endErrorCaught, 'CMC GC must throw UnsupportedOperationError for postponeGCEnd');
}

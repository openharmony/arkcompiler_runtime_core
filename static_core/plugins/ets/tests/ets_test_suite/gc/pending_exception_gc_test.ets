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

/**
 * Test case when we have moving GC along with pending exception.
 * Check that exception is correctly processed
 * (in case of using objects they are wrapped with handles)
 */

function main(): void throws {
    // create object of exception class
    let e: Exception = new Exception("test");

    // adding a couple Full GC to be run without waiting for its completion
    // in order to have GC running during exception processing
    GC.startGC(GC.Cause.FULL);
    GC.startGC(GC.Cause.FULL);
    throw e;
}

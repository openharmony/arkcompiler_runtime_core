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

import { jsNumber } from './test_import';

/**
 * @State
 * @Tags interop, bu_d2d
 */
function benchD2d() {

    /**
     * @Setup
     */
    this.setup = function () {
        // place for code to use in benchmark setup
        return;
    };

    /**
     * @Benchmark
     */
    this.test1 = function() {
        testFunction();
        return;
    };

    /**
     * @Benchmark
     */
    this.test2 = function() {
        return testFunction();
    };

    function testFunction() {
        return jsNumber;
    };

    return;
}

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

import { UnlimitedArguments } from './test_import';

/**
 * @State
 * @Tags interop, bu_d2d
 */
function classInitByRestD2d() {
    function createArgArray(count) {
        const arr = [];

        for (let i = 0; i < count; i++) {
            arr.push(i);
        }
        return arr;
    };

    function init(arg) {
        new UnlimitedArguments(arg);
        return;
    };

    /**
     * @Benchmark
     */
    this.test = function() {
        init(createArgArray(0));

        init(createArgArray(5));

        init(createArgArray(10));

        init(createArgArray(20));

        init(createArgArray(50));

        init(createArgArray(100));

        init(createArgArray(200));

        return;
    };

    return;
}

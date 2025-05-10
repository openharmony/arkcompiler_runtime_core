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

/**
 * @State
 * @Tags interop, bu_d2s
 */
function stringifyObjectD2s() {
    this.obj = null;

    /**
     * @Setup
     */
    this.setup = function () {
        let stsVm = initEtsVm();

        const Person = stsVm.getClass('Lbench_stringify_object_j2a/StringifyObjectD2s;');

        this.obj = new Person();

        return 0;
    };

    /**
     * @Benchmark
     * @returns {Char}
     */
    this.test = function() {
        return JSON.stringify(this.obj);
    };

    return;
}

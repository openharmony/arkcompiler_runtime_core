
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

/**
 * @State
 * @Import { UnlimitedArguments } from '../bu_d2s/libClassVarArg.ets'
 * @Tags interop, s2s
 */
class classInitByRest {

    /**
     * @Param 5, 200
     */
    size: int;
    arr: Array<Int> = [];

    createArgArray(count: int): void {
        this.arr = new Array<Int>();
        for (let i: Int = 0; i < count; i++) {
            this.arr.push(i);
        }
    }

    /**
     * @Setup
     */
    setup(): void {
        this.createArgArray(this.size);
    }

    /**
     * @Benchmark
     */
    test(): void {
        // TypeError: Type 'Array<Int>' is not compatible with rest parameter type 'Double' at index 1
        // TypeError: No matching construct signature for libClassVarArg.UnlimitedArguments(Array<Int>)
        new UnlimitedArguments(this.arr);
    }

}
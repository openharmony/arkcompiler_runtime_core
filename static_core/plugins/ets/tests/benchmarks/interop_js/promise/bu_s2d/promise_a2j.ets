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
 * @Import { promise } from './libImport.js'
 * @Tags interop, promise, s2d
 */
class testPromise {

    totalTime: number;
    runsLeft: number = 10;
    start: number = Chrono.nanoNow();

    /**
     * @Benchmark
     */
    test(): void {
        this.test_function();
    }

    test_function(): Promise<number> {
        return promise().then<number>((value: number): Promise<number> => {
            this.totalTime += Chrono.nanoNow() - this.start;
            this.runsLeft = this.runsLeft - 1;
            return this.test_function();
        }).catch((err: Object): number => {
            // Test failed
            return this.totalTime;
        });
    }
}
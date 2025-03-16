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

import { promise } from 'import';

/**
 * @State
 * @Tags interop, bu_a2j
 */
class PromiseA2j {
    /**
     * @Setup
     */
    public setup(): void {
        console.log('Starting...');
    }

    /**
     * @Benchmark
     */
    test(): number {
        test_function();
        return 1;
    }

    test(): Promise<number> {
        return promise().then<number>((value: number): Promise<number> => {
            this.totalTime += Chrono.nanoNow() - start;

            this.runsLeft = this.runsLeft - 1;

            return this.test();
        }).catch((err: Object): number => {
            console.log("Test failed.", err);
            return this.totalTime;
        });
    }
}
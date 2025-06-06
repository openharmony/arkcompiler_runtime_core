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

import { sumFunction } from 'test_import';

/**
 * @State
 * @Tags interop, bu_a2j
 */
class ReduceCallbackA2j {
    zero: number = 0;
    min: number = 0;
    max: number = 9999999;
    arrayLength: number = 100;
    testArray: Array<number> = new Array<number>();
    sumArray: number;

    generateRandomNumber(min: number, max: number): number {
        return Math.floor(Math.random() * (max - min + 1)) + min;
    }

    generateRandomArray(length: number, min: number, max: number, arr: Array<number>): void {
        for (let i = 0; i < length; i++) {
            const randomNumber = Math.floor(Math.random() * (max - min + 1)) + min;
            arr.push(randomNumber);
        }
    }

    /**
     * @Setup
     */
    public setup(): void {
        console.log('Starting');
        this.generateRandomArray(this.arrayLength, this.min, this.max, this.testArray);
    }

    /**
     * @Benchmark
     */
    test(): number {
        this.sumArray = this.testArray.reduce<number>((acc: number, curr: number): number => {
            return sumFunction(acc, curr);
        }, this.zero);
        return this.sumArray;
    }
}

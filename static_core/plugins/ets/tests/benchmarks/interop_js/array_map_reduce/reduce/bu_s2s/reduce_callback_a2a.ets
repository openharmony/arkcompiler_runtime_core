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
 * @Import { sumFunction } from '../bu_d2s/libImport.ets'
 * @Tags interop, s2s
 */
class reduceCallback {

    zero: number = 0;
    min: number = 0;
    max: number = 9999999;
    arrayLength: number = 100;
    testArray: Array<number> = new Array<number>();
    sumArray: number;

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
        this.generateRandomArray(this.arrayLength, this.min, this.max, this.testArray);
    }

    /**
     * @Benchmark
     */
    test(): number {
        this.sumArray = this.testArray.reduce<number>(sumFunction, this.zero);
        return this.sumArray;
    }
}

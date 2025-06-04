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

import { hexStringToNumberToString } from 'test_import';

/**
 * @State
 * @Tags interop, bu_a2j
 */
class ConversionHexA2j {
    hexString: string = "";
    result: string = '';
    seed: number = 123;

    generateNumber(seed: number): number {
        const modulus = Math.pow(2, 32);
        const a = 1664525;
        const c = 1013904223;
      
        seed = (a * seed + c) % modulus;
        
        return Math.floor((seed / modulus) * 100); 
      }

    /**
     * @Setup
     */
    public setup(): void {
        console.log('Starting');
        const data: Number = this.generateNumber(this.seed);
        this.hexString = data.toString(16);
    }

    /**
     * @Benchmark
     */
    test(): string {
        this.result = hexStringToNumberToString(this.hexString)
        return this.result;
    }
}

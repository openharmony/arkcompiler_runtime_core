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

import { addPrefix } from 'test_import';

/**
 * @State
 * @Tags interop, bu_a2j
 */
class MapCallbackA2j {
    arrayLength: number = 100;
    stringLength: number = 25;
    testArray: Array<string> = new Array<string>();
    newTestArray: Array<string> = new Array<string>();

    generateRandomString(length: number): string {
        const characters = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz';
        let result = '';
        const charactersLength = characters.length;
    
        for (let i = 0; i < length; i++) {
            const randomIndex = Math.floor(Math.random() * charactersLength);
            result += characters[randomIndex];
        }
    
        return result;
    }

    generateRandomArray(length: number, stringLength: number, arr: Array<string>): void {
        for (let i = 0; i < length; i++) {
            let randomString = this.generateRandomString(stringLength);
            arr.push(randomString);
        }
    }

    /**
     * @Setup
     */
    public setup(): void {
        console.log('Starting');
        this.generateRandomArray(this.arrayLength, this.stringLength, this.testArray);
    }

    /**
     * @Benchmark
     */
    test(): number {
        this.newTestArray = this.testArray.map<string>((text: string): string => addPrefix(text));
        return 1;
    }
}

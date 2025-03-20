/**
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

export class CustomArrayIterTest {
    private items: number[];

    private counter: number = 0;

    constructor(items: number[]) {
        this.items = items;
    }

    public getCountedLength(): number
    {
        return this.counter;
    }

    [Symbol.iterator]() {
        let idx = 0;
        const items = this.items;
        let customArrayIterTest = this;

        return {
            next(): IteratorResult<number> {
                if (idx < items.length) {
                    customArrayIterTest.counter++;
                    return { value: items[idx++], done: false };
                } else {
                    return { value: undefined, done: true };
                }
            }
        }
    };
}

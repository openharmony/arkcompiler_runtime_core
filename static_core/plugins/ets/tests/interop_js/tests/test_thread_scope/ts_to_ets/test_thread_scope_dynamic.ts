/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

export let testObject = {};
export let testStr = 'test';
export let testStr50 = new Array(50).fill('a').join('');
export let testStr100 = new Array(100).fill('a').join('');
export let testStr500 = new Array(500).fill('a').join('');
export let testStr1000 = new Array(1000).fill('a').join('');
export let testStr5000 = new Array(5000).fill('a').join('');
export let testStr10000 = new Array(10000).fill('a').join('');
export let testStr50000 = new Array(50000).fill('a').join('');
export let testNumber = 11.22;

export class Foo {
    constructor() {
        this.objectProperty = testObject;
        this.stringProperty = testStr;
        this.string50 = testStr50;
        this.string100 = testStr100;
        this.string500 = testStr500;
        this.string1000 = testStr1000;
        this.string5000 = testStr5000;
        this.string10000 = testStr10000;
        this.string50000 = testStr50000;
        this.numberProperty = testNumber;
    }

    public objectProperty: Object;
    public stringProperty: string;
    public string50: string;
    public string100: string;
    public string500: string;
    public string1000: string;
    public string5000: string;
    public string10000: string;
    public string50000: string;
    public numberProperty: number;
}

export let fooInstance = new Foo();

export let fooFunc = (f: Foo): Foo => {
    return f;
};
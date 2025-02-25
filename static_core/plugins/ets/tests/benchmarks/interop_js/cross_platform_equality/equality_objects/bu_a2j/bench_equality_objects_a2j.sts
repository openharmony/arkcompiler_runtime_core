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

import {
    jsNum,
    jsObj,
    jsObjCopy,
    jsObjOther,
} from 'import';

/**
 * @State
 * @Tags interop, bu_a2j
 */
class EqualityObjectsA2j {
    noEqual: string = '==';
    equal: string = '!=';

    /**
     * @Setup
     */
    public setup(): void {
        console.log('Starting');
    }

    /**
     * @Benchmark
     */
    test(): number {
        this.comparisonObject(jsObj, jsObj, this.equal, 'object ');

        this.comparisonObject(jsObj, jsObjCopy, this.equal, 'object by link ');

        this.comparisonObject(jsObj, jsObjOther, this.noEqual, 'different objects ');

        return 1;
    }

    comparisonObject(valueA: JSValue, valueB: JSValue, compare: string, target: string): void {
        if (compare == this.equal && valueA !== valueB) throw Error();
        if (compare == this.noEqual && valueA === valueB) throw Error();
    }

    comparison<T>(iterations: number, valueA: T, valueB: T, compare: string, target: string): void {
        if (compare == this.equal && valueA != valueB) throw Error();
        if (compare == this.noEqual && valueA == valueB) throw Error();
    }
}

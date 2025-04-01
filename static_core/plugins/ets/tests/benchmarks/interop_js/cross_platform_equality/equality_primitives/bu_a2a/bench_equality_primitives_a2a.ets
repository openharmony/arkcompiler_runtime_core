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

class ClassTest {
    a: int;

    constructor(a: int) {
        this.a = a;
    }
}

const etsBigInt = 1n;
const etsBool = true;
const etsNull = null;
const etsUndefined = undefined;

/**
 * @State
 * @Tags interop, bu_a2a
 */
class EqualityPrimitivesA2a {
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
        // NOTE issue (17741) - enable below after fix import bigInt
        if (false) {
            this.comparison(etsBigInt, etsBigInt, this.equal, 'bigInt ');
        }

        this.comparison(etsBool, etsBool, this.equal, 'boolean ');

        this.comparison(etsNull, etsNull, this.equal, 'null ');

        // NOTE: (19193) enable below after fix comparison
        if (false) {
            this.comparison(etsUndefined, etsUndefined, this.equal, 'undefined ');
        }

        return 1;
    }

    comparison<T>(valueA: T, valueB: T, compare: string, target: string): void {
        if (compare == this.equal && valueA != valueB) throw Error();
        if (compare == this.noEqual && valueA == valueB) throw Error();
    }
}

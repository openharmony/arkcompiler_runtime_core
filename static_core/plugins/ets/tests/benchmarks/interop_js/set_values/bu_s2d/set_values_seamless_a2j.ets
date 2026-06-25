/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
 * @Import {jsNumber,jsByte,jsShort,jsInt,jsLong,jsFloat,jsDouble} from '../../shared/js/libValues.js'
 * @Import {jsBigint,jsBoolean,jsNull,jsUndefined,jsString,jsArray} from '../../shared/js/libValues.js'
 * @Import {jsNumberMap,jsStringMap,jsValueHolder} from "../../shared/js/libValues.js";
 * @Seamless '../../shared/js/libValues.d.ets'
 * @Tags interop, s2d, seamless
 */
class setValueSeamless {

    holder = jsValueHolder;
    array = jsArray;
    numberMap = jsNumberMap;
    stringMap = jsStringMap;

    /**
     * @Benchmark
     */
    setNumber(): void {
        this.holder.jsNumber = 42;
    };

    /**
     * @Benchmark
     */
    setByte(): void {
        this.holder.jsByte = 42;
    };

    /**
     * @Benchmark
     */
    setShort(): void {
        this.holder.jsShort = 42;
    };

    /**
     * @Benchmark
     */
    setInt(): void {
        this.holder.jsInt = 42;
    };

    /**
     * @Benchmark
     */
    setLong(): void {
        this.holder.jsLong = 42;
    };

    /**
     * @Benchmark
     */
    setFloat(): void {
        this.holder.jsFloat = 42.0;
    };

    /**
     * @Benchmark
     */
    setDouble(): void {
        this.holder.jsDouble = 42.0;
    };

    /**
     * @Benchmark
     */
    setBigInt(): void {
        this.holder.jsBigint = 9876543210987654321098765432109876543210n;
    };

    /**
     * @Benchmark
     */
    setBool(): void {
        this.holder.jsBoolean = false;
    };

    /**
     * @Benchmark
     */
    setUndefined(): void {
        this.holder.jsUndefined = undefined;
    };

    /**
     * @Benchmark
     */
    setNull(): void {
        this.holder.jsNull = null;
    };

    /**
     * @Benchmark
     */
    setString(): void {
        this.holder.jsString = "Set Test";
    };

    /**
     * @Benchmark
     */
    setArrayElement(): void {
        this.array[1] = 42;
    };

    /**
     * @Benchmark
     */
    setNumberMapElementExisting(): void {
        this.numberMap[0] = 24;
    };

    /**
     * @Benchmark
     */
    setNumberMapElementNew(): void {
        this.numberMap[3] = 42;
    };

    /**
     * @Benchmark
     */
    setStringMapElementExisting(): void {
        this.stringMap['two'] = 24;
    };

    /**
     * @Benchmark
     */
    setStringMapElementNew(): void {
        this.stringMap['four'] = 42;
    };

}

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

class ValueHolderStatic {
    private prop: number = 42;

    jsMethodArg0(): number {
        return this.prop;
    };
    jsMethodArg1(arg1: number): number {
        return this.prop * arg1;
    };
    jsMethodArg2(arg1: number, arg2: number): number {
        return arg1 * arg2;
    };
    jsMethodArg0V(): void {
        this.prop = 32;
    };
    jsMethodArg1V(arg1: number): void {
        this.prop = this.prop * arg1;
    };
    jsMethodArg2V(arg1: number, arg2: number): void {
        this.prop = arg1 * arg2;
    };
}


/**
 * @State
 * @Import {retHolder} from "../../shared/js/libFunctions.js";
 * @Seamless '../../shared/js/libFunctions.d.ets'
 * @Tags interop, s2s, seamless, static_any
 */
class CallObjectMethodsStatic {

    private holder = retHolder(new ValueHolderStatic());

    /**
     * @Benchmark
     */
    callZero(): Object {
        return this.holder.jsMethodArg0();
    };

    /**
     * @Benchmark
     */
    callShort(): Object {
        return this.holder.jsMethodArg1(24);
    };

    /**
     * @Benchmark
     */
    callRange(): Object {
        return this.holder.jsMethodArg2(42, 24);
    };

    /**
     * @Benchmark
     */
    callZeroVoid(): void {
        this.holder.jsMethodArg0V();
    };

    /**
     * @Benchmark
     */
    callShortVoid(): void {
        this.holder.jsMethodArg1V(24);
    };

    /**
     * @Benchmark
     */
    callRangeVoid(): void {
        this.holder.jsMethodArg2V(42, 24);
    };
}

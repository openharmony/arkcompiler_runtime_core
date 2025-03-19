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
 * @Tags interop, bu_a2a
 */
class GetNumericA2a {
    valueHolder: ValueHolder = new ValueHolder();

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
        const numVal = this.valueHolder.etsNumber;
        const byteVal = this.valueHolder.etsByte;
        const shortVal = this.valueHolder.etsShort;
        const intVal = this.valueHolder.etsInt;
        const longVal = this.valueHolder.etsLong;
        const floatVal = this.valueHolder.etsFloat;
        const doubleVal = this.valueHolder.etsDouble;
        return numVal;
    }
}

export class ValueHolder {
    etsNumber: number = 1;
    etsByte: byte = 123;
    etsShort: short = 123;
    etsInt: int = 123;
    etsLong: long = 123;
    etsFloat: float = 12.34;
    etsDouble: double = 12.34;
}

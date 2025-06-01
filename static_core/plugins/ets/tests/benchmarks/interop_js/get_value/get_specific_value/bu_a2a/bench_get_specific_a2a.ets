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
export class GetSpecificA2a {
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
    test(): boolean {
        // NOTE (oignatenko) as pointed in review comments in PR 2636 split numeric types and string/char
        //  to different benchmarks (#1901)
        const boolVal = this.valueHolder.etsBoolean;
        const bigintVal = this.valueHolder.etsBigint;
        const undefinedVal = this.valueHolder.etsUndefined;
        const nullVal = this.valueHolder.etsNull;
        return boolVal;
    }
}

export class ValueHolder {
    etsBoolean: boolean = true;
    etsBigint: bigint = 1234567890123456789012345678901234567890n;
    etsUndefined: undefined = undefined;
    etsNull: null = null;
}

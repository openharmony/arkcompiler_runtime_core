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
    jsBigint,
    jsBoolean,
    jsNull,
    jsUndefined
} from 'test_import';

/**
 * @State
 * @Tags interop, bu_a2j
 */
export class GetSpecificA2j {
    /**
     * @Setup
     */
    public setup(): void {
        console.log('Starting');
    }

    /**
     * @Benchmark
     */
    test(): object {
        const etsBoolean = jsBoolean;
        const etsBigint = jsBigint;
        const etsNull = jsNull;
        const etsUndefined = jsUndefined;
        return etsBoolean;
    }
}

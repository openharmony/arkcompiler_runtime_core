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

package error.test;

import {
    foo0,
    foo1
} from "org_error_js"

export function bar0(): void {
    try {
        foo0()
    } catch (e: Error) {
        let errObj: ESValue = (e as ESError).getValue()
        console.log("e ets: " + errObj.toNumber())
        throw e
    }
}

export function bar1(): void {
    foo0()
}

export function bar2(): void {
    try {
        foo1()
    } catch (originalError) {
        throw new Error('current error', { cause: originalError })
    }
}
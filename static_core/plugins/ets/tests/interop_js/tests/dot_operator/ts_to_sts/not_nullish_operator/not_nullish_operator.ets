/**
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
    tsString,
    notNullishObj
} from 'main_js';


class PublicGetterClass {
    _value = tsString;
    test: null | undefined//NullishType
    get value(): string {
        return this._value;
    }
}
// NOTE (alexanderpolenov) issue(18449) enable this after fix non null operator
// function checkNotNullishOperator(): boolean {
//     notNullishObj.name = tsString;

//     return notNullishObj.name! as string == tsString as string
// }

// NOTE (alexanderpolenov) issue(18449) enable this after fix non null operator
// function checkNotNullishOperatorError(): boolean {
//     try {
//         notNullishObj.secondName!;

//         return false;
//     } catch (e) {
//         return true;
//     }
// }

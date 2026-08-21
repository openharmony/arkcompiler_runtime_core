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

/*---
desc: >-
    Some of the changes that can make some compiled code not loadable:
    Changing Effective Type of a public or protected method or field.
files: ['./effective_type_class_field_protected.libr.ets']
---*/

import { B } from './effective_type_class_field_protected.libr.ets'

class C extends B {
    check() {
        arktest.assertEQ(this.fldString, 'abc')
        arktest.assertEQ(this.fldTuple.length, 2)
        arktest.assertEQ(this.fldFixedArray[0], null)
        arktest.assertEQ(this.fldValueArray[0], 11.1)
        arktest.assertEQ(this.fldArray.length, 2)
    }
}

function main(): void {
    new C().check()
}

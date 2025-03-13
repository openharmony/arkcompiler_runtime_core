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

 /*---

 name: issue-15142_10.ets
 tags: []
 ---*/

 interface GetAndSet {
    get value(): int
    set value(value: int);
 }


 class Provider implements GetAndSet {
    value: int = 3;
 }

class Extender extends Provider {
    set value(val: int) {
        super.value = val;
    }
}

 function main(): void {
    let test = new Extender();
    assertEQ( test.value, 3)
    test.value = 4;
    assertEQ( test.value, 4)
 }

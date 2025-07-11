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

const gEmpty: String = '';
const gHello: String = 'Hello';
const gHelloDup: String = 'HelloHelloHelloHelloHelloHelloHelloHello';
const gNature: String = '自然界';
const gNatureDup: String = '自然界自然界自然界自然界自然界自然界自然界自然界';

function main(): int {
    arktest.assertEQ(gEmpty.repeat(0), '')
    arktest.assertEQ(gEmpty.repeat(1), '')
    arktest.assertEQ(gNature.repeat(0), '')
    arktest.assertEQ(gHello.repeat(0), '')
    for (let i: int = 1; i < 8; ++i) {
        let hello: String = gHello.repeat(i);
        let nature: String = gNature.repeat(i);
        if (hello != gHelloDup.substring(0, 5 * i)) {
            return 10 + i;
        }
        if (nature != gNatureDup.substring(0, 3 * i)) {
            return 20 + i;
        }
    }

    // try repeat() with a negative index
    let errors: int = 0;
    try {
        let z = gEmpty.repeat(-1);
    } catch (e) {
        errors = 1;
    }

    if (errors != 1) {
        return 1;
    }

    return 0;
}

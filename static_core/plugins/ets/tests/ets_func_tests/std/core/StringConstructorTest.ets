/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

class TestObject extends Object {
    public toString(): String {
        return 'abc'
    }
}

function isNotEqual(a: String, b: String) {
    if (a == b) {
        return 0
    }
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.log('Incorrect String constructor!')
    return 1
}

function main(): int {
    let fails: int = 0
    fails += isNotEqual(String(new TestObject()), 'abc')
    fails += isNotEqual(String(null), 'null')
    fails += isNotEqual(String(undefined), 'undefined')
    fails += isNotEqual(String(), '')
    fails += isNotEqual(String('aaba'), 'aaba')
    fails += isNotEqual(String('aaba'), String('aaba'))
    let e = new RangeError('aaaa')
    fails += isNotEqual(String(e), e.toString())
    return fails
}
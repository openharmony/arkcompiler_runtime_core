/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

type KeyType = 'day' | 'month' | 'year';

let myRecord: Record<KeyType, number | string> = {
    'day': 199999999,
    'month': 'two',
    'year': 3
};

function testNewRecordValueFromDynamic() {
    arktest.assertTrue(myRecord['day'] === 'one');
    arktest.assertTrue(myRecord['month'] === 2);
    arktest.assertTrue(myRecord['year'] === 'three');
}

// Test case 1: Record with mixed value types
type MixedTypeKey = 'num' | 'str' | 'bool' | 'nullVal';

let mixedTypeRecord: Record<MixedTypeKey, number | string | boolean | null> = {
    'num': 42,
    'str': 'hello',
    'bool': true,
    'nullVal': null
};

// Test case 2: Record with falsy values
type FalsyKey = 'zero' | 'false' | 'emptyStr';

let falsyRecord: Record<FalsyKey, number | boolean | string> = {
    'zero': 0,
    'false': false,
    'emptyStr': ''
};

// Test case 3: Record with large numbers
type LargeNumKey = 'intMax' | 'intMin' | 'largeNum';

let largeNumberRecord: Record<LargeNumKey, number> = {
    'intMax': 2147483647,
    'intMin': -2147483648,
    'largeNum': 9007199254740991
};

// Test case 4: Record with sequential access
type SequenceKey = 'a' | 'b' | 'c' | 'd' | 'e';

let sequenceRecord: Record<SequenceKey, number> = {
    'a': 1,
    'b': 2,
    'c': 3,
    'd': 4,
    'e': 5
};

// Test case 5: Record with modification
type ModifyKey = 'x' | 'y' | 'z';

let modifyRecord: Record<ModifyKey, number> = {
    'x': 10,
    'y': 20,
    'z': 30
};

// Test case 6: Record with special string values
type SpecialStrKey = 'empty' | 'space' | 'special';

let specialStringRecord: Record<SpecialStrKey, string> = {
    'empty': '',
    'space': 'hello world',
    'special': 'a@b#c$'
};

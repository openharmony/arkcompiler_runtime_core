/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

const DUMP: boolean = false;
const HITS_EXPECTED: int = 40;
let golden: String = 'abc\u03a3\u03a8\u03a90123456789!';
let strlenPlus1: int = golden.getLength() + 1;
let hits: int = 0;

let stringTable1: String[] = [
    'abcΣΨΩ0123456789!',
    'bcΣΨΩ0123456789!',
    'cΣΨΩ0123456789!',
    'ΣΨΩ0123456789!',
    'ΨΩ0123456789!',
    'Ω0123456789!',
    '0123456789!',
    '123456789!',
    '23456789!',
    '3456789!',
    '456789!',
    '56789!',
    '6789!',
    '789!',
    '89!',
    '9!',
    '!',
    ''
];

let stringTable2: String[] = [
    '',
    'a',
    'ab',
    'abc',
    'abcΣ',
    'abcΣΨ',
    'abcΣΨΩ',
    'abcΣΨΩ0',
    'abcΣΨΩ01',
    'abcΣΨΩ012',
    'abcΣΨΩ0123',
    'abcΣΨΩ01234',
    'abcΣΨΩ012345',
    'abcΣΨΩ0123456',
    'abcΣΨΩ01234567',
    'abcΣΨΩ012345678',
    'abcΣΨΩ0123456789',
    'abcΣΨΩ0123456789!'
];


function main(): int {

    for (let i: int = 0; i <= golden.getLength(); ++i) {
        let subs: String = golden.substring(i, golden.getLength());
        arktest.assertLT(i, stringTable1.length);
        if (subs == stringTable1[i]) {
            ++hits;
        }
        if (DUMP) {
			console.println(subs);
		}
    }
    for (let i: int = 0; i <= golden.getLength(); ++i) {
        arktest.assertLT(i, stringTable2.length);
        let subs: String = golden.substring(0, i);
        if (subs == stringTable2[i]) {
            ++hits;
        }
        if (DUMP) {
			console.println(subs);
		}
    }

    arktest.assertLT(golden.getLength(), strlenPlus1);
    // If begin_index < 0, then it is assumed to be equal to zero.
    if (golden.substring(-1, 1) == 'a') {
        ++hits;
    } else if (DUMP) {
        console.println('substring failed if begin_index < 0');
    }
    // If begin_index > str_len, then it is assumed to be equal to str_len.
    // If begin_index > end_index, then these are swapped.
    if (golden.substring(strlenPlus1, 1) == 'bcΣΨΩ0123456789!') {
        ++hits;
    } else if (DUMP) {
        console.println('substring failed if (begin_index > str_len || begin_index > end_index');
    }
    // If end_index < 0, then it is assumed to be equal to zero.
    if (golden.substring(0, -1) == '') {
        ++hits;
    } else if (DUMP) {
        console.println('substring failed if end_index < 0');
    }
    // If end_index > str_len, then it is assumed to be equal to str_len.
    if (golden.substring(0, strlenPlus1) == golden) {
        ++hits;
    } else if (DUMP) {
        console.println('substring failed if end_index > str_len');
    }

    if (hits != HITS_EXPECTED) {
        if (DUMP) {
            console.println('hits: ' + hits + ' (expected ' + HITS_EXPECTED + ')');
        }
        console.println('FAIL');
        return 1;
    }
    return 0;
}

/*
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

let what: String = '';

function finder(s: String) {
    return s.compareTo(what) == 0;
}

function testLatin() {
    let strs: String[] = [];
    let n = 5000;
    for (let i = 0; i < n; i += 10) {
        strs.push(`${i}ww`);
        what = `${i}ww`;
        let found = strs.find(finder);
        arktest.assertEQ(found, what, 'The string should be found');
        if (strs.length > 3) {
            strs.unshift();
        }
    }
}

function testCornerCases() {
    let tuples: [String, String, int][] = [
        ['zя', 'zz', 1],
        ['zя', 'z', 1],
        ['zzя', 'zzz', 1],
        ['zzя', 'zz', 1],
        ['zzzя', 'zzzz', 1],
        ['zzzя', 'zzz', 1],
        ['zzzzя', 'zzzzz', 1],
        ['zzzzя', 'zzzz', 1],
        ['\uFEFFa', 'aa', 1],
        ['aaa', 'aaa', 0],
    ];
    for (let i = 0; i < tuples.length; i++) {
        let tuple = tuples[i];
        let a = tuple[0];
        let b = tuple[1];
        let e = tuple[2];
        let r = a.compareTo(b);
        if (e > 0) {
            arktest.assertLT(0, r);
        } else if (e < 0) {
            arktest.assertLT(r, 0);
        } else {
            arktest.assertEQ(r, 0);
        }
    }
}

let prefix = '';

function mixed_finder(s: String) {
    let r = s.compareTo(what);
    if (s.startsWith(prefix)) {
        arktest.assertLT(r, 0);
    }
    return r == 0;
}

function testMixed() {
    let strs = new Array<String>;
    let n = 5000;
    for (let i = 0; i < n; i += 10) {
        prefix = `${i}`;
        strs.push(`${i}w`);
        what = `${i}я`;
        let found = strs.find(mixed_finder);
        arktest.assertEQ(found, undefined, 'The string should not be found');
        if (strs.length > 3) {
            strs.unshift();
        }
    }

    testCornerCases();
}

function testUtf16() {
    let strs = new Array<String>;
    let n = 5000;
    for (let i = 0; i < n; i += 10) {
        strs.push(`${i}я`);
        what = `${i}я`;
        let found = strs.find(finder);
        arktest.assertEQ(found, what, 'The string should be found');
        if (strs.length > 3) {
            strs.unshift();
        }
    }
}

function test_tree() {
    let basestring: String = "12345678901234"
    let basestring2 = "abcdefghijklmn"
    let treestring: String = basestring.concat(basestring2)

    let re = treestring.compareTo("12345678901234abcdefghijklmn")
    arktest.assertEQ(re, 0);
}

function test_sliced() {
    let basestring: String = "12345678901234567890"
    let slicedstring: String = basestring.substring(1, 15) // 23456789012345
    let re = slicedstring.compareTo("23456789012345")
    arktest.assertEQ(re, 0);
}

function main(): int {
    let suite = new arktest.ArkTestsuite("StringCompareTo");
    suite.addTest("testLatin", testLatin);
    suite.addTest("testMixed", testMixed);
    suite.addTest("testUtf16", testUtf16);
    suite.addTest("test_tree", test_tree);
    suite.addTest("test_sliced", test_sliced);
    return suite.run();
}


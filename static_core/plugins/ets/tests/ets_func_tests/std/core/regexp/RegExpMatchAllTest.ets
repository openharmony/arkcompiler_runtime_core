/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

function main(): int {
    const suite = new arktest.ArkTestsuite("RegExp.MatchAllTest")
    suite.addTest("Test the results of match", matchResultTest)
    suite.addTest("Test the return results of matchAll and match", matchTest)

    suite.addTest("Test matchall regression 1", testMatchAllRegression1)
    suite.addTest("Test matchall regression 2", testMatchAllRegression2)
    suite.addTest("Test matchall regression 3", testMatchAllRegression3)

    suite.addTest("Test matchAll with large repeated latin1 input", testMatchAllLargeLatin1)
    suite.addTest("Test matchAll with large repeated utf16 input", testMatchAllLargeUtf16)
    suite.addTest("Test matchAll with large input and named groups", testMatchAllLargeNamedGroups)

    return suite.run()
}

function matchResultTest(): void {
    let failure = 0
    let emptyArray : FixedArray<RegExpMatchArray> = []
    failure += test(new RegExp("c", "g"), "c", [new RegExpMatchArray(0, "c", ["c"])])
    failure += test(new RegExp("b", "g"), "a", emptyArray)
    failure += test(new RegExp(".", "g"), "ab", [new RegExpMatchArray(0, "ab", ["a"]), new RegExpMatchArray(1, "ab", ["b"])])
    failure += test(new RegExp("(?:)", "g"), "d", [new RegExpMatchArray(0, "d", [""]), new RegExpMatchArray(1, "d", [""])])
    failure += test(new RegExp("", "g"), "d", [new RegExpMatchArray(0, "d", [""]), new RegExpMatchArray(1, "d", [""])])

    failure += test(new RegExp("c"), "c", [new RegExpMatchArray(0, "c", ["c"])])
    failure += test(new RegExp("b"), "a", emptyArray)
    failure += test(new RegExp("."), "ab", [new RegExpMatchArray(0, "ab", ["a"])])
    failure += test(new RegExp("(?:)"), "d", [new RegExpMatchArray(0, "d", [""])])
    failure += test(new RegExp(""), "d", [new RegExpMatchArray(0, "d", [""])])

    let regexp = new RegExp(".", "g")
    let s = "abc"
    regexp.exec(s)
    failure += checkTestResult(regexp.lastIndex.toInt(), 1)
    let iter = s.matchAll(regexp)
    regexp.exec(s)
    failure += checkTestResult(regexp.lastIndex.toInt(), 2)
    let next_result = iter.next()
    failure += checkTestResult(next_result.value!, new RegExpMatchArray(1, "abc", ["b"]))
    failure += checkTestResult(next_result.done, false)
    failure += checkTestResult(regexp.lastIndex.toInt(), 2)

    let regexplastIndextest = new RegExp("(?:ab|cd)\\d?","g");
    regexplastIndextest.lastIndex = -1;
    failure += checkTestResult(regexplastIndextest.test("aacd22"), true);
    arktest.assertEQ(failure, 0)
}

function test(r: RegExp, str: String, expected: FixedArray<RegExpMatchArray>) {
    let iter = r.matchAll(str)
    let failure = 0
    let v = iter.next();
    for (let i = 0; i < expected.length; ++i) {
        failure += checkTestResult(v.value!, expected[i]);
        v = iter.next()
    }
    failure += checkTestResult(v.done, true);
    return failure
}

function matchTest(): void {
    const regexp1 = new RegExp('n', 'g')
    const str1 = 'nn'
    let matchres1 = str1.matchAll(regexp1)
    arktest.assertEQ(Array.from(matchres1).length, 2)
    arktest.assertEQ(Array.from(matchres1).length, 0)

    const regexp2 = new RegExp('t', 'g')
    const str2 = 'ttt'
    let matchres2 = str2.match(regexp2)
    arktest.assertEQ(matchres2?.index, undefined)
    arktest.assertEQ(matchres2?.input, undefined)

    const regexp3 = new RegExp('t')
    let matchres3 = str2.match(regexp3)
    arktest.assertEQ(matchres3?.index, 0)
    arktest.assertEQ(matchres3?.input, 'ttt')

    const str4: string = '𐐀a'
    const regexp4 = new RegExp('(?=a)', 'gu')
    arktest.assertEQ(Array.from(str4.matchAll(regexp4)).length, 1)

    const str5: string = '😄'
    const regexp5 = new RegExp('(b)?', 'gu')
    arktest.assertEQ(Array.from(str5.matchAll(regexp5))[0][1], undefined)
}

function testMatchAllRegression1() {
  const str: string = "😀a";
  const regex: RegExp = new RegExp('(?<x>(?<=😀).)', 'gu');
  const result = Array.from(str.matchAll(regex));
  arktest.assertEQ(result[0].groups?.['x'], "a");
}

function testMatchAllRegression2() {
  const str: string = "1a";
  const regex: RegExp = new RegExp('(?<=1)(a)', 'g');
  const result = Array.from(str.matchAll(regex));
  arktest.assertEQ(result[0][0], "a");
  arktest.assertEQ(result[0][1], "a");
}

function testMatchAllRegression3() {
  const str: string = "1a";
  const regex: RegExp = new RegExp('(?=(a)?)', 'g');
  const result = Array.from(str.matchAll(regex));
  arktest.assertEQ(result[0][1], undefined);
}


function testMatchAllLargeLatin1(): void {
    const str = "test".repeat(10000)
    const regexp = new RegExp('test', 'g')
    const matches = Array.from(str.matchAll(regexp))
    arktest.assertEQ(matches.length, 10000)
    arktest.assertEQ(matches[0][0], "test")
    arktest.assertEQ(matches[0].index, 0)
    arktest.assertEQ(matches[9999][0], "test")
    arktest.assertEQ(matches[9999].index, 39996)
}

function testMatchAllLargeUtf16(): void {
    const str = "测试".repeat(5000)
    const regexp = new RegExp('测试', 'g')
    const matches = Array.from(str.matchAll(regexp))
    arktest.assertEQ(matches.length, 5000)
    arktest.assertEQ(matches[0][0], "测试")
    arktest.assertEQ(matches[0].index, 0)
    arktest.assertEQ(matches[4999][0], "测试")
    arktest.assertEQ(matches[4999].index, 9998)
}

function testMatchAllLargeNamedGroups(): void {
    let parts: string = ""
    for (let i = 0; i < 1000; i++) {
        parts += "a" + i.toString() + "b "
    }
    const str = parts
    const regexp = new RegExp('(?<char>a)(?<num>\\d+)(?<last>b)', 'g')
    const matches = Array.from(str.matchAll(regexp))
    arktest.assertEQ(matches.length, 1000)
    arktest.assertEQ(matches[0].groups?.['char'], "a")
    arktest.assertEQ(matches[0].groups?.['num'], "0")
    arktest.assertEQ(matches[0].groups?.['last'], "b")
    arktest.assertEQ(matches[999].groups?.['num'], "999")
}

function checkTestResult(actual: boolean, expected: boolean) : int {
    if (actual != expected) return 1;
    return 0;
}

function checkTestResult(actual: int, expected: int) : int {
    if (actual != expected) return 1;
    return 0;
}


function checkTestResult(actual : RegExpMatchArray | null, expected : RegExpMatchArray | null) : int {
    if (actual == null && expected == null) {
        return 0;
    }
    if (actual == null || expected == null) {
        return 1;
    }

    if (actual.isCorrect != expected.isCorrect) {
        console.println("isCorrect field of actual is not equals to expected")
        console.println("actual.isCorrect : " + actual.isCorrect)
        console.println("expected.isCorrect : " + expected.isCorrect)
        return 1;
    }

    if (actual.index != expected.index) {
        console.println("index field of actual is not equals to expected")
        console.println("actual.index : " + actual.index)
        console.println("expected.index : " + expected.index)
        return 1;
    }

    if (actual.result.length != expected.result.length) {
        console.println("size of result array of actual is not equals to expected")
        console.println("actual size : " + actual.result.length);
        console.println("expected size : " + expected.result.length);
        return 1;
    }

    for (let i = 0; i < actual.result.length; i++ ) {
        if (!actual.result[i]!.equals(expected.result[i])) {
            console.println ("check index : " + i);
            console.println("content of result array from actual is not equals to expected");
            console.println("actual  : " + actual.result[i]);
            console.println("expected : " + expected.result[i]);
            return 1;
        }
    }

    for (let i = 0; i < actual.length; i++ ) {
        if (!actual[i]!.equals(expected[i])) {
            console.println ("check index : " + i);
            console.println("content of result array from actual is not equals to expected");
            console.println("actual  : " + actual[i]);
            console.println("expected : " + expected[i]);
            return 1;
        }
    }

    for (let i = 0; i < actual.indices.length; i++) {
        let actInd = actual.indices[i]
        let expInd = expected.indices[i]
        for (let j = 0; j < actInd.length; j++) {
            if (actInd[j] != expInd[j]) {
            console.println("check index: " + i);
            console.println("content of result array from actual is not equals to expected");
            console.println("actual  : " + actInd[j]);
            console.println("expected : " + expInd[j]);
            }
        }
    }
    return 0;
}

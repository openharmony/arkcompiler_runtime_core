/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
    let failure = 0
    let emptyArray : RegExpMatchArray[] = []
    failure += test(new RegExp("c", "g"), "c", [new RegExpMatchArray(0, "c", ["c"])])
    failure += test(new RegExp("b", "g"), "a", emptyArray)
    failure += test(new RegExp(".", "g"), "ab", [new RegExpMatchArray(0, "ab", ["a"]), new RegExpMatchArray(1, "ab", ["b"])])
    failure += test(new RegExp("(?:)", "g"), "d", [new RegExpMatchArray(0, "d", [""]), new RegExpMatchArray(1, "d", [""])])
    failure += test(new RegExp("", "g"), "d", [new RegExpMatchArray(0, "d", [""]), new RegExpMatchArray(1, "d", [""])])
    return failure
}

function test(r: RegExp, str: String, expected: RegExpMatchArray[]) {
    let iter = str.matchAll(r)
    let failure = 0
    //NOTE(kirill-mitkin): Blocked due to verifier error
    /*let v = iter.next();
    for (let i = 0; i < expected.length; ++i) {
        failure += checkTestResult(v.value!, expected[i]);
        v = iter.next()
    }
    failure += checkTestResult(v.done, true);*/
    return failure
}

function checkTestResult(actual: boolean, expected: boolean) : int {
    if (actual != expected) return 1;
    return 0;
}

function checkTestResult(actual : RegExpExecArray | null, expected : RegExpExecArray | null) : int {
    if (actual == null && expected == null) {
        return 0;
    }
    if (actual == null || expected == null) {
        return 1;
    }

    if (actual!.isCorrect != expected!.isCorrect) {
        console.println("isCorrect field of actual is not equals to expected")
        console.println("actual.isCorrect : " + actual!.isCorrect)
        console.println("expected.isCorrect : " + expected!.isCorrect)
        return 1;
    }

    if (actual!.index != expected!.index) {
        console.println("index field of actual is not equals to expected")
        console.println("actual.index : " + actual!.index)
        console.println("expected.index : " + expected!.index)
        return 1;
    }

    if (actual!.result.length != expected!.result.length) {
        console.println("size of result array of actual is not equals to expected")
        console.println("actual size : " + actual!.result.length);
        console.println("expected size : " + expected!.result.length);
        return 1;
    }

    for (let i = 0; i < actual!.result.length; i++ ) {
        if (!actual!.result[i].equals(expected!.result[i])) {
            console.println ("check index : " + i);
            console.println("content of result array from actual is not equals to expected");
            console.println("actual  : " + actual!.result[i]);
            console.println("expected : " + expected!.result[i]);
            return 1;
        }
    }

    for (let i = 0; i < actual!.indices.length; i++) {
        let actInd = (Value.of(actual!.indices) as ArrayValue).getElement(i).getData() as number[]
        let expInd = (Value.of(expected!.indices) as ArrayValue).getElement(i).getData() as number[]
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
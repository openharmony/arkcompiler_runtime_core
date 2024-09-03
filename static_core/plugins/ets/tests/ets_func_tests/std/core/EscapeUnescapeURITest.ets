/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
    const set1 = ";/?:@&=+$,#"; // Reserved Characters
    const set2 = "-.!~*'()"; // Unreserved Marks
    const set3 = "ABC abc 123"; // Alphanumeric Characters + Space
    const numberExample: Number = 12345; // Number
    const booleanExampleTrue: Boolean = true; // Boolean true
    const booleanExampleFalse: Boolean = false; // Boolean false

    let fails = 0

    fails += testEquals(encodeURI(set1), ";/?:@&=+$,#")
    fails += testEquals(encodeURI(set2), "-.!~*'()");
    fails += testEquals(encodeURI(set3), "ABC%20abc%20123");

    fails += testEquals(encodeURIComponent(set1), "%3B%2F%3F%3A%40%26%3D%2B%24%2C%23");
    fails += testEquals(encodeURIComponent(set2), "-.!~*'()");
    fails += testEquals(encodeURIComponent(set3), "ABC%20abc%20123");

    fails += testEquals(encodeURIComponent(numberExample), "12345");
    fails += testEquals(encodeURIComponent(booleanExampleTrue), "true");
    fails += testEquals(encodeURIComponent(booleanExampleFalse), "false");

    let ok: boolean

    ok = false
    try {
        encodeURI("\uD800");
    } catch (e) {
        ok = true
    }
    if (!ok) {
        console.println("fail: single surrogate encoded")
        fails += 1
    }

    fails += testEquals(encodeURI("\uD800\uDFFF"), "%F0%90%8F%BF")

    fails += testEquals(decodeURI("/JavaScript%3A%20a_scripting_language"), "/JavaScript%3A a_scripting_language")
    fails += testEquals(decodeURIComponent("/JavaScript%3A%20a_scripting_language"), "/JavaScript: a_scripting_language")

    ok = false
    try {
        decodeURI("%E0%A4%A");
    } catch (e) {
        ok = true
    }
    if (!ok) {
        console.println("fail: decoded invalid")
        fails += 1
    }

    fails += testEquals(encodeURI("https://ru.wikipedia.org/wiki/Радуга#Радуга_и_ассоциированные_термины"), 'https://ru.wikipedia.org/wiki/%D0%A0%D0%B0%D0%B4%D1%83%D0%B3%D0%B0#%D0%A0%D0%B0%D0%B4%D1%83%D0%B3%D0%B0_%D0%B8_%D0%B0%D1%81%D1%81%D0%BE%D1%86%D0%B8%D0%B8%D1%80%D0%BE%D0%B2%D0%B0%D0%BD%D0%BD%D1%8B%D0%B5_%D1%82%D0%B5%D1%80%D0%BC%D0%B8%D0%BD%D1%8B')

    fails += testInverse("https://ru.wikipedia.org/wiki/Радуга#Радуга_и_ассоциированные_термины")
    fails += testInverse("🌈🌬️👩🏻‍🏭💅🏿")
    fails += testInverse("http://www.example.org:56789/a/b/c.txt?t=win&s=chess#para5")

    fails += testInverse("https://zh-classical.wikipedia.org/wiki/自然數#溤諾曼定義")
    fails += testEquals(encodeURI("https://zh-classical.wikipedia.org/wiki/自然數#溤諾曼定義"), 'https://zh-classical.wikipedia.org/wiki/%E8%87%AA%E7%84%B6%E6%95%B8#%E6%BA%A4%E8%AB%BE%E6%9B%BC%E5%AE%9A%E7%BE%A9')

    return fails
}

function testEquals(s1: string, s2: string): int {
    if (s1 == s2) {
        return 0
    }
    console.println('fail: "' + s1 + '"!= "' + s2 + '"')
    return 1
}

function testInverse1(s1: string): int {
    let a: string
    try {
        a = decodeURI(encodeURI(s1))
    } catch (e) {
        console.println('failed with exception for "' + s1 + '"')
        console.println('encodeURI === ' + encodeURI(s1))
        a = decodeURI(encodeURI(s1))
        return 1
    }
    if (a == s1) {
        return 0
    }
    console.println('failed: decodeURI(encodeURI("' + s1 +'")) == "' + a + '" != "' + s1 + '"')
    return 1
}

function testInverse2(s1: string): int {
    let a: string
    try {
        a = decodeURIComponent(encodeURIComponent(s1))
    } catch (e) {
        console.println('failed with exception for "' + s1 + '"')
        console.println('encodeURIComponent === ' + encodeURIComponent(s1))
        a = decodeURIComponent(encodeURIComponent(s1))
        return 1
    }
    if (a == s1) {
        return 0
    }
    console.println('failed: decodeURIComponent(encodeURIComponent("' + s1 +'")) == "' + a + '" != "' + s1 + '"')
    return 1
}

function testInverse(s1: string): int {
    return testInverse1(s1) + testInverse2(s1)
}

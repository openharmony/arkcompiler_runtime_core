/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

function runTimeZoneDependentParseTests(): int {
    let fails = 0;
    let tzOffset = new Date().getTimezoneOffset() * 1000;
    let tests: String[] = [
        "1970 02 01 00:00:00.000",
        "1970 01 02 00:00:00.000",
        "2044 01 28 23:00:00.001",
        "2033 01 21 22:40:40",
        "2176 03 26 20:00",
        "3099 04 25",
        "9999 05",
        "2099 06 23 14:00:00.001",
        "4999 07 22 12:00:00.001",
        "5000 08 21 10:00:00.001",
        "5001 09 20 08:00:00.001",
        "9998 10 08 04:00:00.001",
        "9998 11 15 00:00:00.001",
        "9998 12 01 01:59:00.001"
    ];
    let answers: long[] = [
        2678400000,
        86400000,
        2337634800001,
        1989960040000,
        6508152000000,
        35637667200000,
        253381132800000,
        4085906400001,
        95603544000001,
        95637664800001,
        95671785600001,
        253363435200001,
        253366704000001,
        253368093540001
    ];
    for (let i = 0; i < tests.length; i++) {
        try {
            let parsingRes = Date.parse(tests[i]) + tzOffset;
            if (parsingRes != answers[i]) {
                console.println("Bad parsing of " + tests[i]);
                console.println("result is " + parsingRes);
                console.println("correct answer is " + answers[i]);
                console.println("diff: " + (parsingRes - answers[i]));
            }
        } catch(e) {
            console.log(e.toString());
            console.println('Unable to parse ' + tests[i]);
            fails++;
        }
    }
    return fails;
}

function runTimeZoneIndependentParseTests(): int {
    let fails = 0;
    let tests: String[] = [
        "1970",
        "2370",
        "2044 01 28 23:00:00.001+01:15",
        "2099 06 23 14:00:00.001+02:15",
        "4999 07 22 12:00:00.001+03:15",
        "5000 08 21 10:02:00.001+12:15",
        "5001 09 20 08:00:22.301+23:15",
        "9998 10 08 04:00:00.004-00:15",
        "9998 11 15 00:00:00.001-12:15",
        "9998 12 01 01:59:00.001-23:15"
    ];
    let answers: long[] = [
        0,
        12622780800000,
        2337630300001,
        4085898300001,
        95603532300001,
        95637620820001,
        95671701922301,
        253363436100004,
        253366748100001,
        253368177240001
    ];
    for (let i = 0; i < tests.length; i++) {
        try {
            let parsingRes = Date.parse(tests[i]);
            if (parsingRes != answers[i]) {
                console.println("Bad parsing of " + tests[i]);
                console.println("result is " + parsingRes);
                console.println("correct answer is " + answers[i]);
                console.println("diff: " + (parsingRes - answers[i]));
            }
        } catch(e) {
            console.log(e.toString());
            console.println('Unable to parse ' + tests[i]);
            fails++;
        }
    }
    return fails;
}

function runParseTests(): int {
    return runTimeZoneDependentParseTests() + runTimeZoneIndependentParseTests();
}

function main(): int {
    let fails = 0;
    try {
        let date1 = new Date();
        date1.setUTCFullYear(1970);
        date1.setUTCMonth(1);
        date1.setUTCDate(0);
        date1.setUTCHours(0);
        date1.setUTCMinutes(0);
        date1.setUTCSeconds(0);
        date1.setUTCMilliseconds(0);
        let date2 = new Date();
        date2.setUTCFullYear(1971);
        date2.setUTCMonth(2);
        date2.setUTCDate(1);
        date2.setUTCHours(1);
        date2.setUTCMinutes(1);
        date2.setUTCSeconds(1);
        date2.setUTCMilliseconds(1);
        if (date2.valueOf() - date1.valueOf() != 34045261001) {
            fails++;
            console.log("Invalid setUTC<> work");
        }
    } catch (e) {
        console.log("Can't test setUTC<> functions due to thrown Error")
    }
    fails += runParseTests();
    return fails;
}
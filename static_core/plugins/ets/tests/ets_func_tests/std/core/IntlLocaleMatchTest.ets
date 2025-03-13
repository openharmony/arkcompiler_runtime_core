/**
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

const success: int = 0;
const fail: int = 1;
let en_US: string = intlBestFitLocale("en-US");
let zh_CN: string = intlBestFitLocale("zh-CN");
let ru_RU = intlBestFitLocale("ru-RU");
let er_ER = "er-or" // Fake locale
let localeList: string[] = [er_ER, ru_RU, zh_CN, en_US];

function main(): int {
    let failures: int = 0;
    failures += check(testBestFitLocaleMatchList(), "Intl bestfit locale match list");
    failures += check(testLookupLocaleMatchSingle(), "Intl lookup locale match single");
    failures += check(testLookupLocaleMatchList(), "Intl lookup locale match list");
    return failures;
}

function check(result: int, message: string): int {
    if (result == 0) {
        return success;
    }
    console.println("\nFAILED: " + message);
    return fail;
}

function testBestFitLocaleMatchList(): int {
    let result = intlBestFitLocale(localeList);
    // First existing locale should be selected
    if (result != ru_RU) {
        console.log(result, "!=", ru_RU)
        return fail;
    }
    return success
}

function testLookupLocaleMatchSingle(): int {
    let result = intlLookUpLocale(en_US);
    if (result != en_US) {
        console.log(result, "!=", en_US)
        return fail;
    }
    return success;
}

function testLookupLocaleMatchList(): int {
    let result = intlLookUpLocale(localeList);
    // First existing locale should be selected
    if (result != ru_RU) {
        console.log(result, "!=", ru_RU)
        return fail;
    }
    return success;
}


/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

//! CHECKER      INT keeps Turkic toLocaleCase locale semantics
//! RUN          options: "--compiler-enable-jit=false",
//!              entry: "ets_string_to_locale_case.ETSGLOBAL::main"

//! CHECKER      AOT keeps Turkic toLocaleCase locale semantics
//! RUN_PAOC     options: "--compiler-regex='.*__noinline__toLocale.*Case.*'"
//!
//! METHOD       "ets_string_to_locale_case.ETSGLOBAL::__noinline__toLocaleUpperCaseTurkish"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.StdCoreStringToLocaleUpperCase/
//!
//! METHOD       "ets_string_to_locale_case.ETSGLOBAL::__noinline__toLocaleLowerCaseTurkish"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.StdCoreStringToLocaleLowerCase/
//!
//! METHOD       "ets_string_to_locale_case.ETSGLOBAL::__noinline__toLocaleUpperCaseAzerbaijani"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.StdCoreStringToLocaleUpperCase/
//!
//! METHOD       "ets_string_to_locale_case.ETSGLOBAL::__noinline__toLocaleLowerCaseAzerbaijani"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.StdCoreStringToLocaleLowerCase/
//! RUN          entry: "ets_string_to_locale_case.ETSGLOBAL::main"

function __noinline__toLocaleUpperCaseTurkish(locale: string): string {
    return 'istanbul'.toLocaleUpperCase(locale);
}

function __noinline__toLocaleLowerCaseTurkish(locale: string): string {
    return 'ISTANBUL'.toLocaleLowerCase(locale);
}

function __noinline__toLocaleUpperCaseAzerbaijani(locale: string): string {
    return 'istanbul'.toLocaleUpperCase(locale);
}

function __noinline__toLocaleLowerCaseAzerbaijani(locale: string): string {
    return 'ISTANBUL'.toLocaleLowerCase(locale);
}

function assertTurkishUpper(locale: string): void {
    arktest.assertEQ(__noinline__toLocaleUpperCaseTurkish(locale), '\u0130STANBUL',
                     'Wrong upper-case result for locale ' + locale);
}

function assertTurkishLower(locale: string): void {
    arktest.assertEQ(__noinline__toLocaleLowerCaseTurkish(locale), '\u0131stanbul',
                     'Wrong lower-case result for locale ' + locale);
}

function assertAzerbaijaniUpper(locale: string): void {
    arktest.assertEQ(__noinline__toLocaleUpperCaseAzerbaijani(locale), '\u0130STANBUL',
                     'Wrong upper-case result for locale ' + locale);
}

function assertAzerbaijaniLower(locale: string): void {
    arktest.assertEQ(__noinline__toLocaleLowerCaseAzerbaijani(locale), '\u0131stanbul',
                     'Wrong lower-case result for locale ' + locale);
}

function main(): void {
    assertTurkishUpper('tr');
    assertTurkishUpper('tR');
    assertTurkishUpper('Tr');
    assertTurkishUpper('TR');
    assertTurkishUpper('tr-TR');
    assertTurkishUpper('TR-tr');
    assertTurkishUpper('tr-Latn-TR');

    assertTurkishLower('tr');
    assertTurkishLower('tR');
    assertTurkishLower('Tr');
    assertTurkishLower('TR');
    assertTurkishLower('tr-TR');
    assertTurkishLower('TR-tr');
    assertTurkishLower('tr-Latn-TR');

    assertAzerbaijaniUpper('az');
    assertAzerbaijaniUpper('aZ');
    assertAzerbaijaniUpper('Az');
    assertAzerbaijaniUpper('AZ');
    assertAzerbaijaniUpper('az-AZ');
    assertAzerbaijaniUpper('AZ-az');
    assertAzerbaijaniUpper('az-Latn-AZ');

    assertAzerbaijaniLower('az');
    assertAzerbaijaniLower('aZ');
    assertAzerbaijaniLower('Az');
    assertAzerbaijaniLower('AZ');
    assertAzerbaijaniLower('az-AZ');
    assertAzerbaijaniLower('AZ-az');
    assertAzerbaijaniLower('az-Latn-AZ');
}

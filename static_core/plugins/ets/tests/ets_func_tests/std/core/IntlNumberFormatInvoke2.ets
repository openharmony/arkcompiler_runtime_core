/**
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

function main(): int {
    let suite = new ArkTestsuite("Intl. NumberFormat");
    suite.addTest("$_invoke out-of-range maxFracDigits", testInvokeOutOfRangeMaxFracDigits);
    suite.addTest("$_invoke out-of-range minFracDigits", testInvokeOutOfRangeMinFracDigits);
    suite.addTest("$_invoke out-of-range maxSignDigits", testInvokeOutOfRangeMaxSignDigits);
    suite.addTest("$_invoke out-of-range minSignDigits", testInvokeOutOfRangeMinSignDigits);
    suite.addTest("$_invoke out-of-range minIntDigits", testInvokeOutOfRangeMinIntDigits);
    return suite.run();
}

function testInvokeOutOfRangeMaxFracDigits() {
    let options1: Intl.NumberFormatOptions = {
        maximumFractionDigits: 101
    }
    let options2: Intl.NumberFormatOptions = {
        maximumFractionDigits: -1
    }
    let options3: Intl.NumberFormatOptions = {
        maximumFractionDigits: 5,
        minimumFractionDigits: 3
    }
    let expectedError = new RangeError("maximumFractionDigits value is out of range");
    expectError((): void throws => { Intl.NumberFormat.$_invoke("zh", options1) }, expectedError);
    expectError((): void throws => { Intl.NumberFormat.$_invoke("zh", options2) }, expectedError);
    let getOptions = Intl.NumberFormat.$_invoke("zh", options3).resolvedOptions();
    assertTrue(getOptions.minimumFractionDigits == 3);
    assertTrue(getOptions.maximumFractionDigits == 5);
    assertTrue(getOptions.maximumFractionDigits > getOptions.minimumFractionDigits);
}

function testInvokeOutOfRangeMinFracDigits() {
    let options1: Intl.NumberFormatOptions = {
        minimumFractionDigits: 101
    }
    let options2: Intl.NumberFormatOptions = {
        minimumFractionDigits: -1
    }
    let expectedError = new RangeError("minimumFractionDigits value is out of range");
    expectError((): void throws => { Intl.NumberFormat.$_invoke("zh", options1) }, expectedError);
    expectError((): void throws => { Intl.NumberFormat.$_invoke("zh", options2) }, expectedError);
}

function testInvokeOutOfRangeMaxSignDigits() {
    let options1: Intl.NumberFormatOptions = {
        maximumSignificantDigits: 22
    }
    let options2: Intl.NumberFormatOptions = {
        maximumSignificantDigits: 0
    }
    let options3: Intl.NumberFormatOptions = {
        maximumSignificantDigits: 18,
        minimumSignificantDigits: 17
    }
    let expectedError = new RangeError("maximumSignificantDigits value is out of range");
    expectError((): void throws => { Intl.NumberFormat.$_invoke("zh", options1) }, expectedError);
    expectError((): void throws => { Intl.NumberFormat.$_invoke("zh", options2) }, expectedError);
    let getOptions = Intl.NumberFormat.$_invoke("zh", options3).resolvedOptions();
    assertTrue(getOptions.maximumSignificantDigits! == 18);
    assertTrue(getOptions.minimumSignificantDigits! == 17);
    assertTrue(getOptions.maximumSignificantDigits! > getOptions.minimumSignificantDigits!);
}

function testInvokeOutOfRangeMinSignDigits() {
    let options1: Intl.NumberFormatOptions = {
        minimumSignificantDigits: 22
    }
    let options2: Intl.NumberFormatOptions = {
        minimumSignificantDigits: 0
    }

    let expectedError = new RangeError("minimumSignificantDigits value is out of range");
    expectError((): void throws => { Intl.NumberFormat.$_invoke("zh", options1) },
        expectedError);
    expectError((): void throws => { Intl.NumberFormat.$_invoke("zh", options2) },
        expectedError);
}

function testInvokeOutOfRangeMinIntDigits() {
    let options1: Intl.NumberFormatOptions = {
        minimumIntegerDigits: 22
    }
    let options2: Intl.NumberFormatOptions = {
        minimumIntegerDigits: 0
    }
    let options3: Intl.NumberFormatOptions = {
        minimumIntegerDigits: 15
    }
    let expectedError = new RangeError("minimumIntegerDigits value is out of range");
    expectError((): void throws => { Intl.NumberFormat.$_invoke("zh", options1) }, expectedError);
    expectError((): void throws => { Intl.NumberFormat.$_invoke("zh", options2) }, expectedError);
    let getOptions = Intl.NumberFormat.$_invoke("zh", options3).resolvedOptions();
    assertTrue(getOptions.minimumIntegerDigits == 15);
}

function equaledNumberFormats(a: Intl.NumberFormat, b: Intl.NumberFormat, value: number | bigint): boolean {
    if (a.format(value) != b.format(value)) {
        return false;
    }
    return true;
}


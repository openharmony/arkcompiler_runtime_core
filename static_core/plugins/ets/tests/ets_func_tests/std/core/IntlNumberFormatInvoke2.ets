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
    let expectedError = new RangeError("Invalid maximumFractionDigits");
    expectError((): void throws => { Intl.NumberFormat.$_invoke("zh", options1) }, expectedError);
    expectError((): void throws => { Intl.NumberFormat.$_invoke("zh", options2) }, expectedError);
    let getOptions = Intl.NumberFormat.$_invoke("zh", options3).resolvedOptions();
    assertEQ(getOptions.minimumFractionDigits, 3.0);
    assertEQ(getOptions.maximumFractionDigits, 5.0);
    assertTrue(getOptions.maximumFractionDigits > getOptions.minimumFractionDigits);
}

function testInvokeOutOfRangeMinFracDigits() {
    let options1: Intl.NumberFormatOptions = {
        minimumFractionDigits: 101
    }
    let options2: Intl.NumberFormatOptions = {
        minimumFractionDigits: -1
    }
    let expectedError = new RangeError("Invalid minimumFractionDigits");
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
    let expectedError = new RangeError("Invalid maximumSignificantDigits");
    expectError((): void throws => { Intl.NumberFormat.$_invoke("zh", options1) }, expectedError);
    expectError((): void throws => { Intl.NumberFormat.$_invoke("zh", options2) }, expectedError);
    let getOptions = Intl.NumberFormat.$_invoke("zh", options3).resolvedOptions();
    assertEQ(getOptions.maximumSignificantDigits!, 18.0);
    assertEQ(getOptions.minimumSignificantDigits!, 17.0);
    assertTrue(getOptions.maximumSignificantDigits! > getOptions.minimumSignificantDigits!);
}

function testInvokeOutOfRangeMinSignDigits() {
    let options1: Intl.NumberFormatOptions = {
        minimumSignificantDigits: 22
    }
    let options2: Intl.NumberFormatOptions = {
        minimumSignificantDigits: 0
    }

    let expectedError = new RangeError("Invalid minimumSignificantDigits");
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
    let expectedError = new RangeError("Invalid minimumIntegerDigits");
    expectError((): void throws => { Intl.NumberFormat.$_invoke("zh", options1) }, expectedError);
    expectError((): void throws => { Intl.NumberFormat.$_invoke("zh", options2) }, expectedError);
    let getOptions = Intl.NumberFormat.$_invoke("zh", options3).resolvedOptions();
    assertEQ(getOptions.minimumIntegerDigits, 15.0);
}

function equaledNumberFormats(a: Intl.NumberFormat, b: Intl.NumberFormat, value: number | bigint): boolean {
    return a.format(value) == b.format(value);
}


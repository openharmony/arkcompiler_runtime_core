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

function eArabic(x: Number): string {
    return x.toLocaleString('ar-EG');
}

function testEArabic1(): void {
    assertEQ("ليس\u00A0رقم", eArabic(NaN), "Number toLocaleString not equals to ليس رقم");
}

function testEArabic2(): void {
    assertEQ("١٬٢٣٤", eArabic(1234), "1234 should format as '١٬٢٣٤'");
}

function main() {
    let myTestsuite = new ArkTestsuite("myTestsuite");
    myTestsuite.addTest("testEArabic1", testEArabic1);
    myTestsuite.addTest("testEArabic2", testEArabic2);
    myTestsuite.run();
}

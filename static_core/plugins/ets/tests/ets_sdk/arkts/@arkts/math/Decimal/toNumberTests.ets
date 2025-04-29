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

import { Decimal } from "@arkts.math.Decimal";

function t(n: string | number, expected: number) {
    let v = new Decimal(n).toNumber();
    assertEQ(expected, v, `Decimal(${n}).toNumber(), actual:${v} is not equal to ${expected}`);
}

function testToNumber() {
    t('0', 0);
    t('0.0', 0);
    t('0.000000000000', 0);
    t('0e+0', 0);
    t('0e-0', 0);

    t('-0', -0);
    t('-0.0', -0);
    t('-0.000000000000', -0);
    t('-0e+0', -0);
    t('-0e-0', -0);

    t(Infinity, Infinity);
    t('Infinity', Infinity);
    t(-Infinity, -Infinity);
    t('-Infinity', -Infinity);

    t(1, 1);
    t('1', 1);
    t('1.0', 1);
    t('1e+0', 1);
    t('1e-0', 1);

    t(-1, -1);
    t('-1', -1);
    t('-1.0', -1);
    t('-1e+0', -1);
    t('-1e-0', -1);

    t('123.456789876543', 123.456789876543);
    t('-123.456789876543', -123.456789876543);

    t('9007199254740991', 9007199254740991);
    t('-9007199254740991', -9007199254740991);

    t('1.7976931348623157e+308', 1.7976931348623157e+308);
}

function config() {
    Decimal.precision = 20;
    Decimal.rounding = 4;
    Decimal.toExpNeg = -7;
    Decimal.toExpPos = 21;
    Decimal.minE = -9e15;
    Decimal.maxE = 9e15;
}

function main() {
    const suite = new ArkTestsuite("Decimal toNumber API tests");
    suite.addTest("Decimal toNumber 001", testToNumber);
    config();
    return suite.run();
}
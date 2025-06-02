/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0(the "License");
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

function main(): int {
    const suite = new arktest.ArkTestsuite("Decimal cbrt API tests");
    suite.addTest("Decimal cbrt with default config", testCbrtAndStaticCbrt);
    suite.addTest("Decimal cbrt with specific config", testCbrtWithSpecificConfig);
    return suite.run();
}

function checkCbrtResult(n: string, pr: number, rm: number, expected: string) {
    Decimal.precision = pr;
    Decimal.rounding = rm;
    arktest.assertEQ(new Decimal(n).cbrt().valueOf(), expected);
}

function testCbrtAndStaticCbrt() {
    arktest.assertEQ(new Decimal(0).cbrt().valueOf(), Decimal.cbrt(0).valueOf());
    arktest.assertEQ(new Decimal(NaN).cbrt().valueOf(), Decimal.cbrt(NaN).valueOf());
    arktest.assertEQ(new Decimal(Infinity).cbrt().valueOf(), Decimal.cbrt(Infinity).valueOf());
    arktest.assertEQ(new Decimal(-Infinity).cbrt().valueOf(), Decimal.cbrt(-Infinity).valueOf());
    arktest.assertEQ(new Decimal(0.5).cbrt().valueOf(), Decimal.cbrt(0.5).valueOf());
    arktest.assertEQ(new Decimal(-0.5).cbrt().valueOf(), Decimal.cbrt(-0.5).valueOf());
    arktest.assertEQ(new Decimal(1).cbrt().valueOf(), Decimal.cbrt(1).valueOf());
    arktest.assertEQ(new Decimal(-1).cbrt().valueOf(), Decimal.cbrt(-1).valueOf());
}

function testCbrtWithSpecificConfig() {
    Decimal.set({ precision: 40, rounding: 4, toExpNeg: -9e15, toExpPos: 9e15, minE: -9e15, maxE: 9e15 });
    checkCbrtResult('NaN', 40, 4, 'NaN');
    checkCbrtResult('0', 40, 4, '0');
    checkCbrtResult('-0', 40, 4, '-0');
    checkCbrtResult('Infinity', 40, 4, 'Infinity');
    checkCbrtResult('-Infinity', 40, 4, '-Infinity');
    checkCbrtResult('1', 40, 4, '1');
    checkCbrtResult('-1', 40, 4, '-1');
    checkCbrtResult('8', 40, 4, '2');
    checkCbrtResult('-8', 40, 4, '-2');

    checkCbrtResult('-4', 5, 2, '-1.5874');
    checkCbrtResult('-0.8667', 7, 0, '-0.9534318');
    checkCbrtResult('-0.0000000071', 9, 2, '-0.00192199734');
    checkCbrtResult('-21791364.26687703933853824404550506158463693222', 3, 1, '-279');
    checkCbrtResult('36', 9, 6, '3.30192725');
    checkCbrtResult('0.898', 7, 0, '0.9647737');
    checkCbrtResult('-94444444444444444.444444444444444444444444444444444444444444414404441444441', 8, 2, '-455399.06');
    checkCbrtResult('-3782929120857330517139808', 5, 2, '-155810000');
    checkCbrtResult('966666666692107298099458326175656566655348300467903965479870253722013788135743211111111111111111111.22222', 1, 5, '1000000000000000000000000000000000');
    checkCbrtResult('16.84', 7, 4, '2.563189');
    checkCbrtResult('-58918', 2, 3, '-39');
    checkCbrtResult('-74535634133038239268820777237286', 2, 4, '-42000000000');
    checkCbrtResult('6888888888.66666666666602248', 3, 5, '1900');
    checkCbrtResult('608785700000000000000000100000000000000000000.000000500020900040006', 1, 5, '800000000000000');
    checkCbrtResult('-3834369', 8, 0, '-156.51813');
    checkCbrtResult('4345752', 8, 0, '163.18804');
    checkCbrtResult('5000300.5', 5, 5, '171');
    checkCbrtResult('0.73', 7, 3, '0.9004113');
    checkCbrtResult('0.612', 20, 3, '0.84901847487755466985');
    checkCbrtResult('-26403122.886392512193592', 9, 3, '-297.772851');
    checkCbrtResult('-15047905.6442424463707664865226022703526330746497470178339129', 10, 2, '-246.8834739');
    checkCbrtResult('-0.57216409476227961', 122, 4, '-0.83018242216788557900092187018458431268949382089457981268783675303907114542359515145805957769873268594828033331460924645988');
    checkCbrtResult('-58892300900.0000037', 72, 3, '-3890.62620353612987584901759020572729952315745798457018660549639374283766');
    checkCbrtResult('4312692950776.434730495181946194732352621783', 134, 0, '16277.3179326290103123151730291347428666068760542903239801418389563715424057253795836596796134792716104557634024961949417454938847683');
    checkCbrtResult('0.772144893677647945980591', 32, 4, '0.91741591091195652167447475342283');
    checkCbrtResult('-6404156859301838811126951723297929248851328010128081790499875', 82, 4, '-185703741377679767357.3800976029196648819860800202650959861930855922060702625367479');
    checkCbrtResult('82632936267840682945343488907.06731470799523118845064687431515', 138, 6, '4355630815.60590669653353707630763857376582544155292301618426362137145407016564135319408858966461324112682532653188194626242452174990441469');
}

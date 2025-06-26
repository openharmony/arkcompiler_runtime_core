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
    const suite = new arktest.ArkTestsuite("Decimal log API tests");
    suite.addTest("Decimal log with specific config", testLogWithSpecificConfigPart001);
    suite.addTest("Decimal log with specific config", testLogWithSpecificConfigPart002);
    suite.addTest("Decimal log with specific config", testLogWithSpecificConfigPart003);
    return suite.run();
}

function checkLogResult(n: string, base: string | number, expected: string, sd: number, rm: number) {
    Decimal.precision = sd;
    Decimal.rounding = rm;
    arktest.assertEQ(new Decimal(n).log(base).valueOf(), expected);
}

function testLogWithSpecificConfigPart001() {
    Decimal.set({ precision: 40, rounding: 4, toExpNeg: -9e15, toExpPos: 9e15, minE: -9e15, maxE: 9e15 });
    checkLogResult('22876792454961', '59049', '2.8', 2, 2);
    checkLogResult('32', '33554432', '0.2', 1, 2);
    checkLogResult('16', '1125899906842624', '0.08', 1, 2);
    checkLogResult('16777216', '1024', '2.4', 2, 2);
    checkLogResult('31381059609', '3486784401', '1.1', 2, 4);
    checkLogResult('131072', '16', '4.25', 3, 7);
    checkLogResult('17179869184', '65536', '2.125', 4, 2);
    checkLogResult('131072', '32', '3.4', 2, 5);
    checkLogResult('31381059609', '6561', '2.75', 3, 4);
    checkLogResult('1162261467', '81', '4.75', 3, 2);
    checkLogResult('5', '152587890625', '0.0625', 3, 8);
    checkLogResult('4', '1024', '0.2', 1, 2);
    checkLogResult('268435456', '1048576', '1.4', 2, 8);
    checkLogResult('456870693.58', '238920772.66', '1.0336035877093034523', 21, 4);
    checkLogResult('575547956.8582', '824684975.3545', '0.98248076', 8, 4);
    checkLogResult('82275648.874341603', '959190115.624130088', '0.88124641544168894893181429200832363', 35, 4);
    checkLogResult('74257343.4', '743703514.4', '0.88720377341908842250463392057841865999040289364224', 50, 4);
    checkLogResult('617556576.22', '1390349767.37', '0.96145220002205342499', 20, 4);
    checkLogResult('385659206.402956', '306197094.245356', '1.0118079926535367225661814147003237994862', 41, 4);
    checkLogResult('1739848017', '139741504', '1.134455757605027173760473871049514546484', 40, 4);
}

function testLogWithSpecificConfigPart002() {
    checkLogResult('684413372.332', '749444030.62', '0.99556', 5, 4);
    checkLogResult('1276559129.76358811', '1814329747.19301894', '0.983510102095361604388', 21, 4);
    checkLogResult('470873324.56', '770017206.95', '0.975963952980122531477453931545461086248352', 42, 4);
    checkLogResult('142843622.855', '188030025.676', '0.985573716314165', 15, 4);
    checkLogResult('208762187.506204', '15673510.715596', '1.1563', 5, 4);
    checkLogResult('1066260899.1963', '954219284.761', '1.005369396783858165862954752482856604', 37, 4);
    checkLogResult('98615189.15', '75483684.05', '1.0147363402964731399253', 23, 4);
    checkLogResult('134306349.93018997', '262971762.95484809', '0.965342550919082621945239', 24, 4);
    checkLogResult('964681161.089224', '1910911588.814815', '0.9680153968863558918522522557796148', 34, 4);
    checkLogResult('9303669', '272208139', '0.8262', 4, 4);
    checkLogResult('388804210', '196979048', '1.035603565223696855965', 22, 4);
    checkLogResult('699589959.2322617', '574032511.7854473', '1.0098079347111332288609', 23, 4);
    checkLogResult('100575245.36', '172874206.82', '0.971443699412905370317336892965778', 33, 4);
    checkLogResult('188632711.8541175', '1056627336.0975408', '0.9170754305183363941127042', 25, 4);
    checkLogResult('267522787.94', '528716571.79', '0.966083390988836341228896', 24, 4);
    checkLogResult('145509306.43395', '472783713.04935', '0.941003844701466585568051857', 28, 4);
    checkLogResult('991525965.6381098', '609527830.0476525', '1.024053580832128', 16, 4);
    checkLogResult('1023653880.6218838', '953120602.1428507', '1.00345303146', 13, 4);
    checkLogResult('55755796.19', '1330531177.01', '0.84899920538009273', 17, 4);
    checkLogResult('334096229.1342503', '563056758.6770503', '0.97409528', 8, 4);
}

function testLogWithSpecificConfigPart003() {
    checkLogResult('9635164', '231514430', '0.834932623823994616103829175346875687708', 39, 4);
    checkLogResult('131654133.157309973', '115412751.259558256', '1.007092396906741330059871530698890891053443', 43, 4);
    checkLogResult('26880.2432276408875624', 2, '14.7142585720457255', 19, 3);
    checkLogResult('18216.8140929585641372', 2, '14.152983050314836771855701', 26, 1);
    checkLogResult('28062.73494235358182', 2, '14.776367997755111083362495', 26, 0);
    checkLogResult('7408.82842447993', 2, '12.8550297084583087071', 21, 1);
    checkLogResult('395.067', 2, '8.62595353', 9, 3);
    checkLogResult('27442.6587462411378', 2, '14.74414', 7, 0);
    checkLogResult('29259.23925137426', 2, '14.83660463902', 13, 1);
}

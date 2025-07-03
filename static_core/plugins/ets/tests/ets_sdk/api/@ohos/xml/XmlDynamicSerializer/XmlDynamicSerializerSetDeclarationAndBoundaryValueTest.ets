/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import { xml } from "@ohos.xml"

function main(): int {
    const suite = new arktest.ArkTestsuite('XmlDynamicSerializer API tests');

    suite.addTest('Testing XML declaration with utf-8 encoding.', testXMLDeclarationWithUtf8Encoding);
    suite.addTest('Testing XML serialization with zero length.', testXMLSerializationWithZeroLength);

    return suite.run();
}

// Test cases ported from ArkTS 1.0: XML_00100
function testXMLDeclarationWithUtf8Encoding() {
    let thatSer = new xml.XmlDynamicSerializer('utf-8');
    thatSer.setDeclaration();
    let result = '<?xml version="1.0" encoding="utf-8"?>';
    let arrayBuffer = thatSer.getOutput();
    let view1 = '';
    let view = new Uint8Array(arrayBuffer);
    for (let i = 0; i < result.length; ++i) {
        view1 = view1 + String.fromCodePoint(view[i]);
    }
    arktest.assertEQ(view1, result);
}

// Test cases ported from ArkTS 1.0: XML_04600
function testXMLSerializationWithZeroLength() {
    let thatSer = new xml.XmlDynamicSerializer();
    let arrayBuffer = thatSer.getOutput();
    let view1 = '';
    let result = '';
    let view = new Uint8Array(arrayBuffer);
    for (let i = 0; i < view.length; ++i) {
        view1 = view1 + String.fromCodePoint(view[i]);
    }
    arktest.assertEQ(view1, result);
}

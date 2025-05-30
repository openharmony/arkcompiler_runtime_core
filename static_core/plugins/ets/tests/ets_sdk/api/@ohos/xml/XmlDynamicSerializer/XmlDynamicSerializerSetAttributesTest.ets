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

    suite.addTest('Testing writing an element with a single attribute.', testWriteElementWithSingleAttribute);
    suite.addTest('Testing writing an element with multiple attributes.', testWriteElementWithMultipleAttributes);

    return suite.run();
}

// Test cases ported from ArkTS 1.0: XML_01100
function testWriteElementWithSingleAttribute() {
    let thatSer = new xml.XmlDynamicSerializer();
    thatSer.startElement('note');
    thatSer.setAttributes('importance1', 'high1');
    thatSer.endElement();
    let result = '<note importance1="high1"/>';
    let arrayBuffer = thatSer.getOutput();
    let view1 = '';
    let view = new Uint8Array(arrayBuffer);
    for (let i = 0; i < result.length; ++i) {
        view1 = view1 + String.fromCodePoint(view[i]);
    }
    arktest.assertEQ(view1, result);
}

// Test cases ported from ArkTS 1.0: XML_01200
function testWriteElementWithMultipleAttributes() {
    let thatSer = new xml.XmlDynamicSerializer();
    thatSer.setDeclaration();
    thatSer.startElement('note');
    thatSer.setAttributes('importance1', 'high1');
    thatSer.setAttributes('importance2', 'high2');
    thatSer.endElement();
    let arrayBuffer = thatSer.getOutput();
    let result = '<?xml version="1.0" encoding="utf-8"?>\r\n<note importance1="high1" importance2="high2"/>';
    let view = new Uint8Array(arrayBuffer);
    let view1 = '';
    for (let i = 0; i < result.length; ++i) {
        view1 = view1 + String.fromCodePoint(view[i]);
    }
    arktest.assertEQ(view1, result);
}

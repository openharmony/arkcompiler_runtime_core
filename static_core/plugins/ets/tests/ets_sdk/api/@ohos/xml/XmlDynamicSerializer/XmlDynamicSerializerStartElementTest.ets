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

    suite.addTest('Testing single element start element serialization.', testSingleElementStartElement);
    suite.addTest('Testing multiple element start and end elements serialization.',
        testMultipleElementStartAndEndElements);
    suite.addTest('Testing nested element start and end elements serialization.', testNestedElementStartAndEndElements);
    suite.addTest('Testing deeply nested element start and end elements serialization.',
        testDeeplyNestedElementStartAndEndElements);
    suite.addTest('Testing sibling elements within a parent element serialization.', testSiblingElementsWithinParent);

    return suite.run();
}

// Test cases ported from ArkTS 1.0: XML_00600
function testSingleElementStartElement() {
    let thatSer = new xml.XmlDynamicSerializer();
    thatSer.startElement('note1');
    thatSer.endElement();
    let result = '<note1/>';
    let arrayBuffer = thatSer.getOutput();
    let view1 = '';
    let view = new Uint8Array(arrayBuffer);
    for (let i = 0; i < result.length; ++i) {
        view1 = view1 + String.fromCodePoint(view[i]);
    }
    arktest.assertEQ(view1, result);
}

// Test cases ported from ArkTS 1.0: XML_00700
function testMultipleElementStartAndEndElements() {
    let thatSer = new xml.XmlDynamicSerializer();
    thatSer.startElement('note1');
    thatSer.endElement();
    thatSer.startElement('note2');
    thatSer.endElement();
    let arrayBuffer = thatSer.getOutput();
    let result = '<note1/>\r\n<note2/>';
    let view = new Uint8Array(arrayBuffer);
    let view1 = '';
    for (let i = 0; i < result.length; ++i) {
        view1 = view1 + String.fromCodePoint(view[i]);
    }
    arktest.assertEQ(view1, result);
}

// Test cases ported from ArkTS 1.0: XML_00800
function testNestedElementStartAndEndElements() {
    let thatSer = new xml.XmlDynamicSerializer();
    thatSer.startElement('note1');
    thatSer.startElement('note2');
    thatSer.endElement();
    thatSer.endElement();
    let arrayBuffer = thatSer.getOutput();
    let result = '<note1>\r\n  <note2/>\r\n</note1>';
    let view = new Uint8Array(arrayBuffer);
    let view1 = '';
    for (let i = 0; i < result.length; ++i) {
        view1 = view1 + String.fromCodePoint(view[i]);
    }
    arktest.assertEQ(view1, result);
}

// Test cases ported from ArkTS 1.0: XML_00900
function testDeeplyNestedElementStartAndEndElements() {
    let thatSer = new xml.XmlDynamicSerializer();
    thatSer.startElement('note1');
    thatSer.startElement('note2');
    thatSer.startElement('note3');
    thatSer.endElement();
    thatSer.endElement();
    thatSer.endElement();
    let arrayBuffer = thatSer.getOutput();
    let result = '<note1>\r\n  <note2>\r\n    <note3/>\r\n  </note2>\r\n</note1>';
    let view = new Uint8Array(arrayBuffer);
    let view1 = '';
    for (let i = 0; i < result.length; ++i) {
        view1 = view1 + String.fromCodePoint(view[i]);
    }
    arktest.assertEQ(view1, result);
}

// Test cases ported from ArkTS 1.0: XML_01000
function testSiblingElementsWithinParent() {
    let thatSer = new xml.XmlDynamicSerializer();
    thatSer.startElement('note1');
    thatSer.startElement('note2');
    thatSer.endElement();
    thatSer.startElement('note3');
    thatSer.endElement();
    thatSer.endElement();
    let arrayBuffer = thatSer.getOutput();
    let result = '<note1>\r\n  <note2/>\r\n  <note3/>\r\n</note1>';
    let view = new Uint8Array(arrayBuffer);
    let view1 = '';
    for (let i = 0; i < result.length; ++i) {
        view1 = view1 + String.fromCodePoint(view[i]);
    }
    arktest.assertEQ(view1, result);
}

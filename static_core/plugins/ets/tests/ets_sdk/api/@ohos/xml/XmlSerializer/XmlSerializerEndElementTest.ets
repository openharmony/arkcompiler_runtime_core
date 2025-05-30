/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
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

import { xml } from "@ohos.xml"
import { BusinessError } from "@ohos.base"

const TypeErrorCodeId: number = 401;
const myMAX = 2048;

function main(): int {
    const suite = new arktest.ArkTestsuite("XmlSerializer API tests");

    suite.addTest("Testing end tag without attributes", testEndElementWithoutAttributes);
    suite.addTest("Testing end tag with single attribute", testEndElementWithSingleAttribute);
    suite.addTest("Testing end tag with multiple attributes", testEndElementWithMultipleAttributes);
    suite.addTest("Testing nested end tags", testNestedEndElements);
    suite.addTest("Testing end tag with attributes in nested elements", testEndElementWithAttributesInNestedElements);
    suite.addTest("Testing end tag with arrayBuffer overflow", testEndElementWithArrayBufferOverflow);

    return suite.run();
}

// Test cases ported from ArkTS 1.0: XML_01600
function testEndElementWithoutAttributes() {
    let arrayBuffer = new ArrayBuffer(myMAX);
    let thatSer = new xml.XmlSerializer(arrayBuffer);
    thatSer.startElement("note");
    thatSer.endElement();
    let result = '<note/>';
    let view = new Uint8Array(arrayBuffer);
    let view1 = "";
    for (let i = 0; i < result.length; ++i) {
        view1 = view1 + String.fromCodePoint(view[i]);
    }
    arktest.assertEQ(view1, result);
}

// Test cases ported from ArkTS 1.0: XML_01700
function testEndElementWithSingleAttribute() {
    let arrayBuffer = new ArrayBuffer(myMAX);
    let thatSer = new xml.XmlSerializer(arrayBuffer);
    thatSer.startElement("note");
    thatSer.setAttributes("importance", "high");
    thatSer.endElement();
    let result = '<note importance="high"/>';
    let view = new Uint8Array(arrayBuffer);
    let view1 = "";
    for (let i = 0; i < result.length; ++i) {
        view1 = view1 + String.fromCodePoint(view[i]);
    }
    arktest.assertEQ(view1, result);
}

// Test cases ported from ArkTS 1.0: XML_01800
function testEndElementWithMultipleAttributes() {
    let arrayBuffer = new ArrayBuffer(myMAX);
    let thatSer = new xml.XmlSerializer(arrayBuffer);
    thatSer.startElement("note");
    thatSer.setAttributes("impo", "hi");
    thatSer.endElement();
    let result = '<note impo="hi"/>';
    let view = new Uint8Array(arrayBuffer);
    let view1 = "";
    for (let i = 0; i < result.length; ++i) {
        view1 = view1 + String.fromCodePoint(view[i]);
    }
    arktest.assertEQ(view1, result);
}

// Test cases ported from ArkTS 1.0: XML_01900
function testNestedEndElements() {
    let arrayBuffer = new ArrayBuffer(myMAX);
    let thatSer = new xml.XmlSerializer(arrayBuffer);
    thatSer.startElement("note1");
    thatSer.startElement("note2");
    thatSer.endElement();
    thatSer.endElement();
    let result = '<note1>\r\n  <note2/>\r\n</note1>';
    let view = new Uint8Array(arrayBuffer);
    let view1 = "";
    for (let i = 0; i < result.length; ++i) {
        view1 = view1 + String.fromCodePoint(view[i]);
    }
    arktest.assertEQ(view1, result);
}

// Test cases ported from ArkTS 1.0: XML_02000
function testEndElementWithAttributesInNestedElements() {
    let arrayBuffer = new ArrayBuffer(myMAX);
    let thatSer = new xml.XmlSerializer(arrayBuffer);
    thatSer.startElement("note2");
    thatSer.setAttributes("importance", "high");
    thatSer.endElement();
    let result = '<note2 importance="high"/>';
    let view = new Uint8Array(arrayBuffer);
    let view1 = "";
    for (let i = 0; i < result.length; ++i) {
        view1 = view1 + String.fromCodePoint(view[i]);
    }
    arktest.assertEQ(view1, result);
}

function testEndElementWithArrayBufferOverflow() {
    let arrayBuffer = new ArrayBuffer(20);
    let thatSer = new xml.XmlSerializer(arrayBuffer);
    thatSer.setNamespace("h", "aaa");
    thatSer.startElement("note");
    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError) && ((e as BusinessError).code == TypeErrorCodeId) &&
            ((e as BusinessError).message == 'XmlSerializer:: EndElement memcpy_s failed');
    }
    arktest.expectThrow(() => {thatSer.endElement()}, exceptionCheck);
}

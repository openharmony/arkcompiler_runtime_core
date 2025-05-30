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

    suite.addTest("Testing invalid encoding for XmlSerializer", testInvalidEncodingForXmlSerializer);
    suite.addTest("Testing single element start tag", testSingleElementStartElement);
    suite.addTest("Testing multiple element start and end tags", testMultipleElementStartAndEndElements);
    suite.addTest("Testing nested element start and end tags", testNestedElementStartAndEndElements);
    suite.addTest("Testing deeply nested element start and end tags", testDeeplyNestedElementStartAndEndElements);
    suite.addTest("Testing sibling elements within a parent element", testSiblingElementsWithinParent);
    suite.addTest("Testing empty name for startElement", testEmptyNameForStartElement);
    suite.addTest("Testing startElement with arrayBuffer overflow", testStartElementWithArrayBufferOverflow);

    return suite.run();
}

function testInvalidEncodingForXmlSerializer() {
    let arrayBuffer = new ArrayBuffer(myMAX);
    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError) && ((e as BusinessError).code == TypeErrorCodeId) &&
            ((e as BusinessError).message == 'Parameter error.Just support utf-8');
    }
    arktest.expectThrow(() => {let thatSer = new xml.XmlSerializer(arrayBuffer, 'utf-16')}, exceptionCheck);
}

// Test cases ported from ArkTS 1.0: XML_00600
function testSingleElementStartElement() {
    let arrayBuffer = new ArrayBuffer(myMAX);
    let thatSer = new xml.XmlSerializer(arrayBuffer);
    thatSer.startElement("note1");
    thatSer.endElement();
    let result = '<note1/>';
    let view = new Uint8Array(arrayBuffer);
    let view1 = "";
    for (let i = 0; i < result.length; ++i) {
        view1 = view1 + String.fromCodePoint(view[i]);
    }
    arktest.assertEQ(view1, result);
}

// Test cases ported from ArkTS 1.0: XML_00700
function testMultipleElementStartAndEndElements() {
    let arrayBuffer = new ArrayBuffer(myMAX);
    let thatSer = new xml.XmlSerializer(arrayBuffer);
    thatSer.startElement("note1");
    thatSer.endElement();
    thatSer.startElement("note2");
    thatSer.endElement();
    let result = '<note1/>\r\n<note2/>';
    let view = new Uint8Array(arrayBuffer);
    let view1 = "";
    for (let i = 0; i < result.length; ++i) {
        view1 = view1 + String.fromCodePoint(view[i]);
    }
    arktest.assertEQ(view1, result);
}

// Test cases ported from ArkTS 1.0: XML_00800
function testNestedElementStartAndEndElements() {
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

// Test cases ported from ArkTS 1.0: XML_00900
function testDeeplyNestedElementStartAndEndElements() {
    let arrayBuffer = new ArrayBuffer(myMAX);
    let thatSer = new xml.XmlSerializer(arrayBuffer);
    thatSer.startElement("note1");
    thatSer.startElement("note2");
    thatSer.startElement("note3");
    thatSer.endElement();
    thatSer.endElement();
    thatSer.endElement();
    let result = '<note1>\r\n  <note2>\r\n    <note3/>\r\n  </note2>\r\n</note1>';
    let view = new Uint8Array(arrayBuffer);
    let view1 = "";
    for (let i = 0; i < result.length; ++i) {
        view1 = view1 + String.fromCodePoint(view[i]);
    }
    arktest.assertEQ(view1, result);
}

// Test cases ported from ArkTS 1.0: XML_01000
function testSiblingElementsWithinParent() {
    let arrayBuffer = new ArrayBuffer(myMAX);
    let thatSer = new xml.XmlSerializer(arrayBuffer);
    thatSer.startElement("note1");
    thatSer.startElement("note2");
    thatSer.endElement();
    thatSer.startElement("note3");
    thatSer.endElement();
    thatSer.endElement();
    let result = '<note1>\r\n  <note2/>\r\n  <note3/>\r\n</note1>';
    let view = new Uint8Array(arrayBuffer);
    let view1 = "";
    for (let i = 0; i < result.length; ++i) {
        view1 = view1 + String.fromCodePoint(view[i]);
    }
    arktest.assertEQ(view1, result);
}

function testEmptyNameForStartElement() {
    let arrayBuffer = new ArrayBuffer(myMAX);
    let thatSer = new xml.XmlSerializer(arrayBuffer);
    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError) && ((e as BusinessError).code == TypeErrorCodeId) &&
            ((e as BusinessError).message == 'Parameter error. Parameter cannot be empty');
    }
    arktest.expectThrow(() => {thatSer.startElement("")}, exceptionCheck);
}

function testStartElementWithArrayBufferOverflow() {
    let arrayBuffer = new ArrayBuffer(30);
    let thatSer = new xml.XmlSerializer(arrayBuffer);
    thatSer.setNamespace("h", "aaa");
    thatSer.startElement("note");
    thatSer.endElement();
    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError) && ((e as BusinessError).code == TypeErrorCodeId) &&
            ((e as BusinessError).message == 'XmlSerializer:: StartElement memcpy_s failed');
    }
    arktest.expectThrow(() => {thatSer.startElement("note1")}, exceptionCheck);
}

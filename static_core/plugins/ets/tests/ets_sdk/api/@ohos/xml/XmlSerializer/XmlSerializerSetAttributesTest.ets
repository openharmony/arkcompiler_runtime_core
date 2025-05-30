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

    suite.addTest("Testing single attribute in an element", testSingleAttributeInElement);
    suite.addTest("Testing multiple attributes in an element", testMultipleAttributesInElement);
    suite.addTest("Testing three attributes in an element", testThreeAttributesInElement);
    suite.addTest("Testing four attributes in an element", testFourAttributesInElement);
    suite.addTest("Testing non-sequential attributes in an element", testNonSequentialAttributesInElement);
    suite.addTest("Testing illegal position for setting attributes", testIllegalPositionForAttributes);
    suite.addTest("Testing empty attribute name", testEmptyAttributeName);
    suite.addTest("Testing attribute setting with arrayBuffer overflow", testAttributeSettingWithOverflow);

    return suite.run();
}

// Test cases ported from ArkTS 1.0: XML_01100
function testSingleAttributeInElement() {
    let arrayBuffer = new ArrayBuffer(myMAX);
    let thatSer = new xml.XmlSerializer(arrayBuffer);
    thatSer.startElement("note");
    thatSer.setAttributes("importance1", "high1");
    thatSer.endElement();
    let result = '<note importance1="high1"/>';
    let view = new Uint8Array(arrayBuffer);
    let view1 = "";
    for (let i = 0; i < result.length; ++i) {
        view1 = view1 + String.fromCodePoint(view[i]);
    }
    arktest.assertEQ(view1, result);
}

// Test cases ported from ArkTS 1.0: XML_01200
function testMultipleAttributesInElement() {
    let arrayBuffer = new ArrayBuffer(myMAX);
    let thatSer = new xml.XmlSerializer(arrayBuffer);
    thatSer.startElement("note");
    thatSer.setAttributes("importance1", "high1");
    thatSer.setAttributes("importance2", "high2");
    thatSer.endElement();
    let result = '<note importance1="high1" importance2="high2"/>';
    let view = new Uint8Array(arrayBuffer);
    let view1 = "";
    for (let i = 0; i < result.length; ++i) {
        view1 = view1 + String.fromCodePoint(view[i]);
    }
    arktest.assertEQ(view1, result);
}

// Test cases ported from ArkTS 1.0: XML_01300
function testThreeAttributesInElement() {
    let arrayBuffer = new ArrayBuffer(myMAX);
    let thatSer = new xml.XmlSerializer(arrayBuffer);
    thatSer.startElement("note");
    thatSer.setAttributes("importance1", "high1");
    thatSer.setAttributes("importance2", "high2");
    thatSer.setAttributes("importance3", "high3");
    thatSer.endElement();
    let result = '<note importance1="high1" importance2="high2" importance3="high3"/>';
    let view = new Uint8Array(arrayBuffer);
    let view1 = "";
    for (let i = 0; i < result.length; ++i) {
        view1 = view1 + String.fromCodePoint(view[i]);
    }
    arktest.assertEQ(view1, result);
}

// Test cases ported from ArkTS 1.0: XML_01400
function testFourAttributesInElement() {
    let arrayBuffer = new ArrayBuffer(myMAX);
    let thatSer = new xml.XmlSerializer(arrayBuffer);
    thatSer.startElement("note");
    thatSer.setAttributes("importance1", "high1");
    thatSer.setAttributes("importance2", "high2");
    thatSer.setAttributes("importance3", "high3");
    thatSer.setAttributes("importance4", "high4");
    thatSer.endElement();
    let result = '<note importance1="high1" importance2="high2" importance3="high3" importance4="high4"/>';
    let view = new Uint8Array(arrayBuffer);
    let view1 = "";
    for (let i = 0; i < result.length; ++i) {
        view1 = view1 + String.fromCodePoint(view[i]);
    }
    arktest.assertEQ(view1, result);
}

// Test cases ported from ArkTS 1.0: XML_01500
function testNonSequentialAttributesInElement() {
    let arrayBuffer = new ArrayBuffer(myMAX);
    let thatSer = new xml.XmlSerializer(arrayBuffer);
    thatSer.startElement("note");
    thatSer.setAttributes("importance1", "high1");
    thatSer.setAttributes("importance2", "high2");
    thatSer.setAttributes("importance3", "high3");
    thatSer.setAttributes("importance5", "high5");
    thatSer.endElement();
    let result = '<note importance1="high1" importance2="high2" importance3="high3" importance5="high5"/>';
    let view = new Uint8Array(arrayBuffer);
    let view1 = "";
    for (let i = 0; i < result.length; ++i) {
        view1 = view1 + String.fromCodePoint(view[i]);
    }
    arktest.assertEQ(view1, result);
}

function testIllegalPositionForAttributes() {
    let arrayBuffer = new ArrayBuffer(myMAX);
    let thatSer = new xml.XmlSerializer(arrayBuffer);
    thatSer.startElement("note");
    thatSer.endElement();
    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError) && ((e as BusinessError).code == TypeErrorCodeId) &&
            ((e as BusinessError).message == 'Illegal position for attribute');
    }
    arktest.expectThrow(() => {thatSer.setAttributes("a","b")}, exceptionCheck);
}

function testEmptyAttributeName() {
    let arrayBuffer = new ArrayBuffer(myMAX);
    let thatSer = new xml.XmlSerializer(arrayBuffer);
    thatSer.startElement("note");
    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError) && ((e as BusinessError).code == TypeErrorCodeId) &&
            ((e as BusinessError).message == 'Parameter error. Parameter cannot be empty');
    }
    arktest.expectThrow(() => {thatSer.setAttributes("","b")}, exceptionCheck);
}

function testAttributeSettingWithOverflow() {
    let arrayBuffer = new ArrayBuffer(35);
    let thatSer = new xml.XmlSerializer(arrayBuffer);
    thatSer.setNamespace("h", "aaa");
    thatSer.startElement("note");
    thatSer.endElement();
    thatSer.startElement("note1");
    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError) && ((e as BusinessError).code == TypeErrorCodeId) &&
            ((e as BusinessError).message == 'XmlSerializer:: SetAttributes memcpy_s failed');
    }
    arktest.expectThrow(() => {thatSer.setAttributes("a", "b")}, exceptionCheck);
}

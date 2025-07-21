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
import { BusinessError } from "@ohos.base"

const BufferOverFlowErrorCodeId: number = 10200062;
const IllegalPositionErrorCodeId: number = 10200063;
const EmptyErrorCodeId: number = 10200064;
const NoElementMatchErrorCodeId: number = 10200065;
const EncodingErrorCodeId: number = 10200066;

function main(): int {
    const suite = new arktest.ArkTestsuite('XmlDynamicSerializer API tests');

    suite.addTest('Testing XML length exceeding xml.MaxXmlLength and checking error code.',
        testXMLBoundaryValueExceedsMaxLength);
    suite.addTest('Testing mismatched start and end elements and checking error code.',
        testMismatchedStartAndEndElements);
    suite.addTest('Testing illegal position to set attributes and checking error code.',
        testIllegalPositionToSetAttributes);
    suite.addTest('Testing setting empty name for attributes and checking error code.', testSetEmptyNameForAttributes);
    suite.addTest('Testing setting unsupported encoding for XML and checking error code.', testSetUnsupportedEncoding);

    return suite.run();
}

// Test cases ported from ArkTS 1.0: XML_04800
function testXMLBoundaryValueExceedsMaxLength() {
    let thatSer = new xml.XmlDynamicSerializer();
    for (let i = 0; i < xml.MaxXmlLength / 25; ++i) {
        thatSer.setNamespace('h', 'aaa');
        thatSer.startElement('note');
        thatSer.endElement();
    }
    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError) && ((e as BusinessError).code == BufferOverFlowErrorCodeId);
    }
    arktest.expectThrow(() => {thatSer.startElement('note1')}, exceptionCheck);
}

// Test cases ported from ArkTS 1.0: XML_04900
function testMismatchedStartAndEndElements() {
    let thatSer = new xml.XmlDynamicSerializer();
    thatSer.startElement('note');
    thatSer.endElement();
    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError) && ((e as BusinessError).code == NoElementMatchErrorCodeId);
    }
    arktest.expectThrow(() => {thatSer.endElement()}, exceptionCheck);
}

// Test cases ported from ArkTS 1.0: XML_05000
function testIllegalPositionToSetAttributes() {
    let thatSer = new xml.XmlDynamicSerializer();
    thatSer.startElement('note');
    thatSer.endElement();
    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError) && ((e as BusinessError).code == IllegalPositionErrorCodeId);
    }
    arktest.expectThrow(() => {thatSer.setAttributes('a', 'b')}, exceptionCheck);
}

// Test cases ported from ArkTS 1.0: XML_05100
function testSetEmptyNameForAttributes() {
    let thatSer = new xml.XmlDynamicSerializer();
    thatSer.startElement('note');
    thatSer.endElement();
    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError) && ((e as BusinessError).code == EmptyErrorCodeId);
    }
    arktest.expectThrow(() => {thatSer.setAttributes('', 'b')}, exceptionCheck);
}

// Test cases ported from ArkTS 1.0: XML_05200
function testSetUnsupportedEncoding() {
    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError) && ((e as BusinessError).code == EncodingErrorCodeId);
    }
    arktest.expectThrow(() => {let thatSer = new xml.XmlDynamicSerializer('utf-16')}, exceptionCheck);
}

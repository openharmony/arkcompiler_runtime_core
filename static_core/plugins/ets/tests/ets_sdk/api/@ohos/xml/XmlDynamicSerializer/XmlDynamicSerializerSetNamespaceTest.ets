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

    suite.addTest('Testing namespace without setting attributes.', testNamespaceWithoutAttributes);
    suite.addTest('Testing namespace with a single attribute.', testNamespaceWithSingleAttribute);
    suite.addTest('Testing namespace with nested elements.', testNamespaceWithNestedElements);

    return suite.run();
}

// Test cases ported from ArkTS 1.0: XML_02100
function testNamespaceWithoutAttributes() {
    let thatSer = new xml.XmlDynamicSerializer();
    thatSer.setDeclaration();
    thatSer.setNamespace('h', 'http://www.w3.org/TR/html4/');
    thatSer.startElement('note');
    thatSer.endElement();
    let result = '<?xml version="1.0" encoding="utf-8"?>\r\n<h:note xmlns:h="http://www.w3.org/TR/html4/"/>';
    let arrayBuffer = thatSer.getOutput();
    let view1 = '';
    let view = new Uint8Array(arrayBuffer);
    for (let i = 0; i < result.length; ++i) {
        view1 = view1 + String.fromCodePoint(view[i]);
    }
    arktest.assertEQ(view1, result);
}

// Test cases ported from ArkTS 1.0: XML_02200
function testNamespaceWithSingleAttribute() {
    let thatSer = new xml.XmlDynamicSerializer();
    thatSer.setDeclaration();
    thatSer.setNamespace('b', 'http://www.w3.org/TR/html4/');
    thatSer.startElement('note');
    thatSer.setAttributes('a', '1');
    thatSer.endElement();
    let arrayBuffer = thatSer.getOutput();
    let result = '<?xml version="1.0" encoding="utf-8"?>\r\n<b:note a="1" xmlns:b="http://www.w3.org/TR/html4/"/>';
    let view = new Uint8Array(arrayBuffer);
    let view1 = '';
    for (let i = 0; i < result.length; ++i) {
        view1 = view1 + String.fromCodePoint(view[i]);
    }
    arktest.assertEQ(view1, result);
}

// Test cases ported from ArkTS 1.0: XML_02400
function testNamespaceWithNestedElements() {
    let thatSer = new xml.XmlDynamicSerializer();
    thatSer.setDeclaration();
    thatSer.setNamespace('h', 'http://www.w3.org/TR/html4/');
    thatSer.startElement('note1');
    thatSer.startElement('note2');
    thatSer.endElement();
    thatSer.endElement();
    let arrayBuffer = thatSer.getOutput();
    let result = '<?xml version="1.0" encoding="utf-8"?>\r\n<h:note1 xmlns:h="http://www.w3.org/TR/html4/">' +
    '\r\n  <h:note2/>\r\n</h:note1>';
    let view = new Uint8Array(arrayBuffer);
    let view1 = '';
    for (let i = 0; i < result.length; ++i) {
        view1 = view1 + String.fromCodePoint(view[i]);
    }
    arktest.assertEQ(view1, result);
}

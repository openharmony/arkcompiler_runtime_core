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

    suite.addTest("Testing empty prefix for namespace", testEmptyPrefixForNamespace);
    suite.addTest("Testing empty namespace value", testEmptyNamespaceValue);
    suite.addTest("Testing namespace setting with arrayBuffer overflow", testNamespaceSettingWithOverflow);

    return suite.run();
}

function testEmptyPrefixForNamespace() {
    let arrayBuffer = new ArrayBuffer(myMAX);
    let thatSer = new xml.XmlSerializer(arrayBuffer);
    thatSer.startElement("note");
    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError) && ((e as BusinessError).code == TypeErrorCodeId) &&
            ((e as BusinessError).message == 'Parameter error. Parameter cannot be empty');
    }
    arktest.expectThrow(() => {thatSer.setNamespace("","http://www.w3.org/TR/html4/")}, exceptionCheck);
}

function testEmptyNamespaceValue() {
    let arrayBuffer = new ArrayBuffer(myMAX);
    let thatSer = new xml.XmlSerializer(arrayBuffer);
    thatSer.startElement("note");
    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError) && ((e as BusinessError).code == TypeErrorCodeId) &&
            ((e as BusinessError).message == 'Parameter error. The type of  must be string');
    }
    arktest.expectThrow(() => {thatSer.setNamespace("h","")}, exceptionCheck);
}

function testNamespaceSettingWithOverflow() {
    let arrayBuffer = new ArrayBuffer(35);
    let thatSer = new xml.XmlSerializer(arrayBuffer);
    thatSer.setNamespace("h", "aaa");
    thatSer.startElement("note");
    thatSer.endElement();
    thatSer.startElement("note1");
    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError) && ((e as BusinessError).code == TypeErrorCodeId) &&
            ((e as BusinessError).message == 'XmlSerializer:: SetNamespace memcpy_s failed');
    }
    arktest.expectThrow(() => {thatSer.setNamespace("b", "bbb")}, exceptionCheck);
}

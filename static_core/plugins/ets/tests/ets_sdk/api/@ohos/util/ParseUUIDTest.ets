/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
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

import {util} from "@ohos.util";
import { BusinessError } from "@ohos.base";

const syntaxErrorCode: number = 10200002;

function main(): int {
    const suite = new arktest.ArkTestsuite("CommonUtil parseUUID API tests")

    suite.addTest("Verify that a valid RFC 4122 version 4 UUID is correctly parsed into a 16-byte array.",
        testParseValidUUID)
    suite.addTest("Verify that an invalid UUID string with missing characters throws the correct exception.",
        testParseInvalidUUIDMissingChars)
    suite.addTest("Verify that an invalid UUID string with non-hexadecimal characters throws the correct exception.",
        testParseInvalidUUIDNonHexChars)

    return suite.run()
}

function testParseValidUUID() {
    const result = util.parseUUID('84bdf796-66cc-4655-9b89-d6218d100f9c');
    arktest.assertEQ(result.length, 16);
}

function testParseInvalidUUIDMissingChars() {
    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError && (e as BusinessError).name == 'BusinessError') &&
            ((e as BusinessError).message == 'Syntax Error.Invalid 84df796-66cc-4655-9b89-d6218d100f9c string') &&
            ((e as BusinessError).code == syntaxErrorCode);
    }
    arktest.expectThrow(() => { util.parseUUID('84df796-66cc-4655-9b89-d6218d100f9c') }, exceptionCheck);
}

function testParseInvalidUUIDNonHexChars() {
    let exceptionCheck = (e: Error | Exception): boolean | string => {
        return (e instanceof BusinessError && (e as BusinessError).name == 'BusinessError') &&
            ((e as BusinessError).message == 'Syntax Error.Invalid 84Wdf796-66cc-4655-9b89-d6218d100f9c string') &&
            ((e as BusinessError).code == syntaxErrorCode);
    }
    arktest.expectThrow(() => { util.parseUUID('84Wdf796-66cc-4655-9b89-d6218d100f9c') }, exceptionCheck);
}

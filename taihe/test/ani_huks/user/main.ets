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
import * as huks from "@generated/huks";
import * as System from "std/core/System";

loadLibrary("ani_huks");

function test_huks_generateKey() {
    // Create input data for the test
    const numbers1: byte[] = [1, 2, 3, 4, 5];
    let arrbuf1: ArrayBuffer = new ArrayBuffer(numbers1.length);
    for (let i = 0; i < numbers1.length; i++) {
        arrbuf1.set(i, numbers1[i]);
    }

    const numbers2: byte[] = [3, 4, 5, 6, 7];
    let arrbuf2: ArrayBuffer = new ArrayBuffer(numbers2.length);
    for (let i = 0; i < numbers2.length; i++) {
        arrbuf2.set(i, numbers2[i]);
    }

    // Create HUKS parameters and options
    const huksParam1: huks.HuksParam = {
        tag: huks.HuksTag.HUKS_TAG_KEY_SIZE,
        value: huks.HuksKeySize.HUKS_RSA_KEY_SIZE_512 as number
    };
    const huksParam2:
        huks.HuksParam = {tag: huks.HuksTag.HUKS_TAG_KEY, value: arrbuf1};

    const huksOptions: huks.HuksOptions = {
        properties: [huksParam1, huksParam2],
        inData: arrbuf2
    };

    // Test that parameters are set correctly
    assertNE(huksOptions.properties, undefined, "properties should be defined");
    assertEQ(huksOptions.properties!.length, 2, "Should have 2 properties");
    assertEQ(
        huksOptions.properties![0].tag, huks.HuksTag.HUKS_TAG_KEY_SIZE,
        "First tag should be KEY_SIZE");
    assertEQ(
        huksOptions.properties![0].value, 512, "KEY_SIZE value should be 512");
    assertEQ(
        huksOptions.properties![1].tag, huks.HuksTag.HUKS_TAG_KEY,
        "Second tag should be KEY");

    // Verify buffer contents
    assertNE(
        huksOptions.properties![1].value, undefined,
        "Key buffer should be defined");
    const keyBuffer = huksOptions.properties![1].value as ArrayBuffer;
    for (let i = 0; i < numbers1.length; i++) {
        assertEQ(
            keyBuffer.at(i), numbers1[i],
            `Key buffer at index ${i} should match`);
    }

    // Verify inData buffer
    assertNE(huksOptions.inData, undefined, "inData should be defined");
    for (let i = 0; i < numbers2.length; i++) {
        assertEQ(
            huksOptions.inData!.at(i), numbers2[i],
            `inData buffer at index ${i} should match`);
    }

    // Call the function
    const result = huks.generateKey("alias", huksOptions);

    // Verify the result based on the expected output
    assertEQ(result.errorCode, 0, "Error code should be 0");

    // Verify outData (should be 3 bytes of value 0x12)
    assertNE(result.outData, undefined, "outData should be defined");
    assertEQ(result.outData!.byteLength, 3, "outData should have 3 bytes");
    for (let i = 0; i < 3; i++) {
        assertEQ(
            result.outData!.at(i), 0x12, `outData byte ${i} should be 0x12`);
    }

    // Verify certChains
    assertNE(result.certChains, undefined, "certChains should be defined");
    assertEQ(result.certChains!.length, 5, "Should have 5 cert chains");
    for (let i = 0; i < 5; i++) {
        assertEQ(
            result.certChains![i], "Hello", `certChain ${i} should be "Hello"`);
    }

    // Verify properties
    assertNE(result.properties, undefined, "properties should be defined");
    assertEQ(result.properties!.length, 7, "Should have 7 properties");

    // Each property should have tag HUKS_TAG_ACCESS_TIME and a buffer of 4
    // bytes with value 0xcc
    for (let i = 0; i < 7; i++) {
        assertEQ(
            result.properties![i].tag, huks.HuksTag.HUKS_TAG_ACCESS_TIME,
            `Property ${i} tag should be ACCESS_TIME`);

        const propBuffer = result.properties![i].value as ArrayBuffer;
        assertEQ(
            propBuffer.byteLength, 4,
            `Property ${i} buffer should have 4 bytes`);

        for (let j = 0; j < 4; j++) {
            assertEQ(
                propBuffer.at(j), 0xcc as byte,
                `Property ${i} buffer byte ${j} should be 0xcc`);
        }
    }
}

function main() {
    const suite = new ArkTestsuite("HUKS Tests");
    suite.addTest("Test HUKS generateKey", test_huks_generateKey);
    System.exit(suite.run());
}
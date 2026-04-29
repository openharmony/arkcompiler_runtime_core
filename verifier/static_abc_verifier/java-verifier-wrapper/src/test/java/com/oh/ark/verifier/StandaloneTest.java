/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

package com.oh.ark.verifier;

import java.io.File;
import java.io.FileOutputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/**
 * Standalone test (no JUnit dependency) for the Static ABC Verifier Java SDK.
 * Run: java -Djava.library.path=<path-to-so> com.oh.ark.verifier.StandaloneTest
 */
public class StandaloneTest {

    private static int passed = 0;
    private static int failed = 0;

    private static final int HEADER_SIZE = 72;
    private static final int FILE_CONTENT_OFFSET = 12;

    public static void main(String[] args) {
        System.out.println("=== Static ABC Verifier - Java SDK Standalone Tests ===\n");

        runTest("testValidAbcFromFile", StandaloneTest::testValidAbcFromFile);
        runTest("testInvalidMagic", StandaloneTest::testInvalidMagic);
        runTest("testInvalidChecksum", StandaloneTest::testInvalidChecksum);
        runTest("testNonExistentFile", StandaloneTest::testNonExistentFile);
        runTest("testNullFilePathThrows", StandaloneTest::testNullFilePathThrows);
        runTest("testTooSmallFile", StandaloneTest::testTooSmallFile);
        runTest("testVerifyResultToString", StandaloneTest::testVerifyResultToString);

        System.out.println("\n=== Results: " + passed + " passed, " + failed + " failed ===");
        System.exit(failed > 0 ? 1 : 0);
    }

    private static void runTest(String name, Runnable test) {
        System.out.print("  " + name + " ... ");
        try {
            test.run();
            System.out.println("PASS");
            passed++;
        } catch (Throwable t) {
            System.out.println("FAIL: " + t.getMessage());
            failed++;
        }
    }

    private static void assertTrue(String msg, boolean condition) {
        if (!condition) throw new AssertionError(msg);
    }

    private static void assertFalse(String msg, boolean condition) {
        assertTrue(msg, !condition);
    }

    private static File writeTempAbc(byte[] content) {
        try {
            File tmp = File.createTempFile("sav_test_", ".abc");
            tmp.deleteOnExit();
            try (FileOutputStream fos = new FileOutputStream(tmp)) {
                fos.write(content);
            }
            return tmp;
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    private static byte[] buildMinimalValidAbc() {
        ByteBuffer buf = ByteBuffer.allocate(HEADER_SIZE);
        buf.order(ByteOrder.LITTLE_ENDIAN);
        buf.put(new byte[]{'P', 'A', 'N', 'D', 'A', 0, 0, 0});
        buf.putInt(0); // checksum placeholder
        buf.put(new byte[]{0, 0, 0, 6}); // version
        buf.putInt(HEADER_SIZE); // fileSize
        for (int i = 0; i < 13; i++) buf.putInt(0); // remaining header fields
        byte[] data = buf.array();

        int checksum = computeAdler32(data, FILE_CONTENT_OFFSET, data.length - FILE_CONTENT_OFFSET);
        data[8]  = (byte)(checksum & 0xFF);
        data[9]  = (byte)((checksum >> 8) & 0xFF);
        data[10] = (byte)((checksum >> 16) & 0xFF);
        data[11] = (byte)((checksum >> 24) & 0xFF);
        return data;
    }

    private static int computeAdler32(byte[] data, int offset, int length) {
        int a = 1, b = 0;
        for (int i = offset; i < offset + length; i++) {
            a = (a + (data[i] & 0xFF)) % 65521;
            b = (b + a) % 65521;
        }
        return (b << 16) | a;
    }

    static void testValidAbcFromFile() {
        byte[] abc = buildMinimalValidAbc();
        File tmp = writeTempAbc(abc);
        VerifyResult result = StaticAbcVerifier.verify(tmp.getAbsolutePath());
        assertTrue("Expected valid from file", result.isValid());
        assertTrue("Expected code 0", result.getErrorCode() == 0);
    }

    static void testInvalidMagic() {
        byte[] abc = buildMinimalValidAbc();
        abc[0] = 'X';
        File tmp = writeTempAbc(abc);
        VerifyResult result = StaticAbcVerifier.verify(tmp.getAbsolutePath());
        assertFalse("Expected invalid", result.isValid());
        assertTrue("Expected error msg about magic",
                   result.getErrorMessage().contains("magic"));
    }

    static void testInvalidChecksum() {
        byte[] abc = buildMinimalValidAbc();
        abc[8] = (byte)0xFF;
        abc[9] = (byte)0xFF;
        File tmp = writeTempAbc(abc);
        VerifyResult result = StaticAbcVerifier.verify(tmp.getAbsolutePath());
        assertFalse("Expected invalid", result.isValid());
    }

    static void testNonExistentFile() {
        VerifyResult result = StaticAbcVerifier.verify("/no/such/file.abc");
        assertFalse("Expected invalid for missing file", result.isValid());
    }

    static void testNullFilePathThrows() {
        try {
            StaticAbcVerifier.verify((String) null);
            throw new AssertionError("Expected IllegalArgumentException");
        } catch (IllegalArgumentException e) {
            // expected
        }
    }

    static void testTooSmallFile() {
        byte[] tiny = {0x50, 0x41, 0x4E};
        File tmp = writeTempAbc(tiny);
        VerifyResult result = StaticAbcVerifier.verify(tmp.getAbsolutePath());
        assertFalse("Expected invalid for tiny file", result.isValid());
    }

    static void testVerifyResultToString() {
        VerifyResult valid = new VerifyResult(true, 0, "");
        assertTrue("toString should contain valid=true",
                   valid.toString().contains("valid=true"));
        VerifyResult invalid = new VerifyResult(false, 2, "bad magic");
        assertTrue("toString should contain valid=false",
                   invalid.toString().contains("valid=false"));
    }
}

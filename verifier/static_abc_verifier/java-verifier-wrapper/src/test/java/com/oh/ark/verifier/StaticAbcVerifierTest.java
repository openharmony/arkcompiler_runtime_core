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

import org.junit.Test;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import static org.junit.Assert.*;

/**
 * Unit tests for StaticAbcVerifier Java SDK.
 *
 * <p>Tests build minimal ABC payloads and verify them via filesystem paths only.
 */
public class StaticAbcVerifierTest {

    private static final byte[] ABC_MAGIC = {'P', 'A', 'N', 'D', 'A', 0, 0, 0};
    private static final byte[] STATIC_VERSION = {0, 0, 0, 6};
    private static final int HEADER_SIZE = 72;
    private static final int FILE_CONTENT_OFFSET = 12;

    /**
     * Build a minimal valid ABC header as a byte array.
     */
    private static byte[] buildMinimalValidAbc() {
        ByteBuffer buf = ByteBuffer.allocate(HEADER_SIZE);
        buf.order(ByteOrder.LITTLE_ENDIAN);

        buf.put(ABC_MAGIC);             // magic (8 bytes)
        buf.putInt(0);                  // checksum placeholder (4 bytes)
        buf.put(STATIC_VERSION);        // version (4 bytes)
        buf.putInt(HEADER_SIZE);        // fileSize
        buf.putInt(0);                  // foreignOff
        buf.putInt(0);                  // foreignSize
        buf.putInt(0);                  // quickenedFlag
        buf.putInt(0);                  // numClasses
        buf.putInt(0);                  // classIdxOff
        buf.putInt(0);                  // numExportTable
        buf.putInt(0);                  // exportTableOff
        buf.putInt(0);                  // numLnps
        buf.putInt(0);                  // lnpIdxOff
        buf.putInt(0);                  // numLiteralarrays
        buf.putInt(0);                  // literalarrayIdxOff
        buf.putInt(0);                  // numIndexes
        buf.putInt(0);                  // indexSectionOff

        byte[] data = buf.array();

        // Compute and set Adler-32 checksum over bytes[12..end]
        int checksum = computeAdler32(data, FILE_CONTENT_OFFSET, data.length - FILE_CONTENT_OFFSET);
        data[8]  = (byte) (checksum & 0xFF);
        data[9]  = (byte) ((checksum >> 8) & 0xFF);
        data[10] = (byte) ((checksum >> 16) & 0xFF);
        data[11] = (byte) ((checksum >> 24) & 0xFF);

        return data;
    }

    private static int computeAdler32(byte[] data, int offset, int length) {
        int a = 1;
        int b = 0;
        final int MOD = 65521;
        for (int i = offset; i < offset + length; i++) {
            a = (a + (data[i] & 0xFF)) % MOD;
            b = (b + a) % MOD;
        }
        return (b << 16) | a;
    }

    private static File writeTempAbc(byte[] content) throws IOException {
        File tmpFile = File.createTempFile("test_abc_verifier_", ".abc");
        tmpFile.deleteOnExit();
        try (FileOutputStream fos = new FileOutputStream(tmpFile)) {
            fos.write(content);
        }
        return tmpFile;
    }

    @Test
    public void testValidAbcFromFile() throws IOException {
        byte[] abc = buildMinimalValidAbc();
        File tmpFile = writeTempAbc(abc);

        VerifyResult result = StaticAbcVerifier.verify(tmpFile.getAbsolutePath());
        assertTrue("Expected valid result from file", result.isValid());
        assertEquals(0, result.getErrorCode());
    }

    @Test
    public void testInvalidMagic() throws IOException {
        byte[] abc = buildMinimalValidAbc();
        abc[0] = 'X';
        File tmpFile = writeTempAbc(abc);

        VerifyResult result = StaticAbcVerifier.verify(tmpFile.getAbsolutePath());
        assertFalse("Expected invalid result", result.isValid());
        assertNotEquals(0, result.getErrorCode());
        assertTrue(result.getErrorMessage().contains("magic"));
    }

    @Test
    public void testInvalidChecksum() throws IOException {
        byte[] abc = buildMinimalValidAbc();
        abc[8] = (byte) 0xFF;
        abc[9] = (byte) 0xFF;
        File tmpFile = writeTempAbc(abc);

        VerifyResult result = StaticAbcVerifier.verify(tmpFile.getAbsolutePath());
        assertFalse("Expected invalid result", result.isValid());
    }

    @Test
    public void testNonExistentFile() {
        VerifyResult result = StaticAbcVerifier.verify("/no/such/file.abc");
        assertFalse("Expected invalid result for missing file", result.isValid());
    }

    @Test(expected = IllegalArgumentException.class)
    public void testNullFilePath() {
        StaticAbcVerifier.verify((String) null);
    }

    @Test
    public void testTooSmallFile() throws IOException {
        byte[] tiny = {0x50, 0x41, 0x4E};
        File tmpFile = writeTempAbc(tiny);

        VerifyResult result = StaticAbcVerifier.verify(tmpFile.getAbsolutePath());
        assertFalse("Expected invalid result for tiny file", result.isValid());
    }

    @Test
    public void testVerifyResultToString() {
        VerifyResult valid = new VerifyResult(true, 0, "");
        assertTrue(valid.toString().contains("valid=true"));

        VerifyResult invalid = new VerifyResult(false, 2, "bad magic");
        assertTrue(invalid.toString().contains("valid=false"));
        assertTrue(invalid.toString().contains("bad magic"));
    }
}

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

/**
 * Lightweight structural integrity verifier for Static ABC bytecode files.
 *
 * <p>This is the main entry point for the Static ABC Verifier SDK.
 * All verification is performed by a native C++ library accessed through JNI.
 *
 * <p>Usage example:
 * <pre>{@code
 *   // Verify from file path
 *   VerifyResult result = StaticAbcVerifier.verify("/path/to/file.abc");
 *   if (result.isValid()) {
 *       System.out.println("File is valid");
 *   } else {
 *       System.err.println("Error: " + result.getErrorMessage());
 *   }
 *
 * }</pre>
 *
 * <p>The verifier checks:
 * <ul>
 *   <li>File magic number ("PANDA")</li>
 *   <li>Bytecode version range</li>
 *   <li>Adler-32 checksum integrity</li>
 *   <li>Header field consistency (offsets within file bounds)</li>
 *   <li>Class index bounds</li>
 *   <li>Literal array index bounds</li>
 *   <li>Region header integrity</li>
 *   <li>Foreign section bounds</li>
 * </ul>
 *
 * <p>Thread safety: all methods are stateless and thread-safe.
 */
public final class StaticAbcVerifier {

    static {
        NativeLoader.load();
    }

    private StaticAbcVerifier() {
    }

    /**
     * Verify a static ABC file at the given filesystem path.
     *
     * @param filePath absolute or relative path to the .abc file
     * @return verification result
     * @throws IllegalArgumentException if filePath is null
     */
    public static VerifyResult verify(String filePath) {
        if (filePath == null) {
            throw new IllegalArgumentException("filePath must not be null");
        }
        return nativeVerifyFile(filePath);
    }

    // JNI native method
    private static native VerifyResult nativeVerifyFile(String filePath);
}

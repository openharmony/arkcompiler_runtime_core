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
 * Result of a static ABC file verification.
 */
public class VerifyResult {

    private final boolean valid;
    private final int errorCode;
    private final String errorMessage;

    /**
     * Constructs a verification result (called from JNI).
     *
     * @param valid        true if the file passed all structural checks
     * @param errorCode    0 if valid; otherwise the first error code encountered
     * @param errorMessage human-readable description of the error(s), empty if valid
     */
    public VerifyResult(boolean valid, int errorCode, String errorMessage) {
        this.valid = valid;
        this.errorCode = errorCode;
        this.errorMessage = errorMessage != null ? errorMessage : "";
    }

    /** Returns true if the ABC file is structurally valid. */
    public boolean isValid() {
        return valid;
    }

    /**
     * Returns the error code. 0 means success.
     * <ul>
     *   <li>1 = FILE_OPEN_FAILED</li>
     *   <li>2 = INVALID_MAGIC</li>
     *   <li>3 = INVALID_VERSION</li>
     *   <li>4 = CHECKSUM_MISMATCH</li>
     *   <li>5 = FILE_SIZE_MISMATCH</li>
     *   <li>6 = INVALID_HEADER</li>
     *   <li>7 = INVALID_CLASS_INDEX</li>
     *   <li>8 = INVALID_LITERAL_ARRAY_INDEX</li>
     *   <li>9 = INVALID_REGION_HEADER</li>
     *   <li>10 = INVALID_FOREIGN_SECTION</li>
     * </ul>
     */
    public int getErrorCode() {
        return errorCode;
    }

    /** Returns a human-readable error description, empty string if valid. */
    public String getErrorMessage() {
        return errorMessage;
    }

    @Override
    public String toString() {
        if (valid) {
            return "VerifyResult{valid=true}";
        }
        return "VerifyResult{valid=false, errorCode=" + errorCode
                + ", errorMessage='" + errorMessage + "'}";
    }
}

/**
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

#ifndef STATIC_ABC_VERIFIER_VERIFIER_H
#define STATIC_ABC_VERIFIER_VERIFIER_H

#include "abc_file.h"

#include <functional>
#include <string>
#include <vector>

namespace ark::static_verifier {

enum class ErrorCode : int {
    OK = 0,
    FILE_OPEN_FAILED = 1,
    INVALID_MAGIC = 2,
    INVALID_VERSION = 3,
    CHECKSUM_MISMATCH = 4,
    FILE_SIZE_MISMATCH = 5,
    INVALID_HEADER = 6,
    INVALID_CLASS_INDEX = 7,
    INVALID_LITERAL_ARRAY_INDEX = 8,
    INVALID_REGION_HEADER = 9,
    INVALID_FOREIGN_SECTION = 10,

    INVALID_SOURCE_LANG = 11,
    INVALID_CLASS_DATA = 12,
    INVALID_METHOD_DATA = 13,
    INVALID_CODE_DATA = 14,
    INVALID_LITERAL_ARRAY_DATA = 15,
    INVALID_BYTECODE = 16,
    INVALID_REGISTER_INDEX = 17,
    INVALID_JUMP_TARGET = 18,
    INVALID_TRY_CATCH = 19,
};

struct VerifyError {
    ErrorCode code;
    std::string message;
};

struct VerifyResult {
    bool valid;
    std::vector<VerifyError> errors;
};

class StaticAbcVerifier {
public:
    explicit StaticAbcVerifier(const std::string &filename);
    StaticAbcVerifier(const uint8_t *data, size_t size);
    ~StaticAbcVerifier() = default;

    VerifyResult Verify();

    // Phase 1: basic structural checks (self-contained, no libarkfile)
    bool VerifyMagic();
    bool VerifyVersion();
    bool VerifyChecksum();
    bool VerifyFileSize();
    bool VerifyHeaderConsistency();
    bool VerifyClassIndex();
    bool VerifyLiteralArrayIndex();
    bool VerifyRegionHeaders();
    bool VerifyForeignSection();

#ifdef STATIC_ABC_VERIFIER_USE_ARKFILE
    // Phase 2: deep structural checks (requires libarkfile)
    bool VerifyClassesDeep();
    bool VerifyMethodsAndCode();
    bool VerifyLiteralArraysDeep();
#endif

    const std::vector<VerifyError> &GetErrors() const { return errors_; }
    void ClearErrors() { errors_.clear(); }

    const std::string &GetFilePath() const { return filePath_; }

private:
    void AddError(ErrorCode code, const std::string &message);
    bool VerifyRegionSubIndex(const char *name, uint32_t regionIdx,
                              uint32_t off, uint32_t size);
    static uint32_t ComputeAdler32(const uint8_t *data, size_t len);
    static bool VersionInRange(const std::array<uint8_t, VERSION_SIZE> &ver,
                               const std::array<uint8_t, VERSION_SIZE> &minVer,
                               const std::array<uint8_t, VERSION_SIZE> &maxVer);
    static int CompareVersion(const std::array<uint8_t, VERSION_SIZE> &a,
                              const std::array<uint8_t, VERSION_SIZE> &b);

    std::string filePath_;
    std::unique_ptr<AbcFile> file_;
    std::vector<VerifyError> errors_;
};

}  // namespace ark::static_verifier

#endif  // STATIC_ABC_VERIFIER_VERIFIER_H

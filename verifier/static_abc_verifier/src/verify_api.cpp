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

#include "verify_api.h"
#include "verifier.h"

#include <algorithm>
#include <cstdint>
#include <string>

namespace {

void FillErrorBuf(char *errorBuf, int errorBufLen, const std::string &msg)
{
    if (errorBuf == nullptr || errorBufLen <= 0) {
        return;
    }
    size_t copyLen = std::min(static_cast<size_t>(errorBufLen - 1), msg.size());
    std::copy_n(msg.data(), copyLen, errorBuf);
    errorBuf[copyLen] = '\0';
}

std::string BuildErrorMessage(const ark::static_verifier::VerifyResult &result)
{
    if (result.errors.empty()) {
        return "";
    }
    std::string msg;
    for (size_t i = 0; i < result.errors.size(); ++i) {
        if (i > 0) {
            msg += "; ";
        }
        msg += "[E" + std::to_string(static_cast<int>(result.errors[i].code)) + "] ";
        msg += result.errors[i].message;
    }
    return msg;
}

}  // namespace

int StaticAbcVerifyFile(const char *filePath, char *errorBuf, int errorBufLen)
{
    if (filePath == nullptr) {
        FillErrorBuf(errorBuf, errorBufLen, "File path is null");
        return static_cast<int>(ark::static_verifier::ErrorCode::FILE_OPEN_FAILED);
    }

    std::string path{filePath};
    ark::static_verifier::StaticAbcVerifier verifier{path};
    auto result = verifier.Verify();
    if (result.valid) {
        FillErrorBuf(errorBuf, errorBufLen, "");
        return 0;
    }

    FillErrorBuf(errorBuf, errorBufLen, BuildErrorMessage(result));
    return result.errors.empty() ? 1 : static_cast<int>(result.errors[0].code);
}

int StaticAbcVerifyMemory(const unsigned char *data, size_t size, char *errorBuf, int errorBufLen)
{
    if (data == nullptr || size == 0) {
        FillErrorBuf(errorBuf, errorBufLen, "Buffer is null or empty");
        return static_cast<int>(ark::static_verifier::ErrorCode::FILE_OPEN_FAILED);
    }

    ark::static_verifier::StaticAbcVerifier verifier(reinterpret_cast<const uint8_t *>(data), size);
    auto result = verifier.Verify();
    if (result.valid) {
        FillErrorBuf(errorBuf, errorBufLen, "");
        return 0;
    }

    FillErrorBuf(errorBuf, errorBufLen, BuildErrorMessage(result));
    return result.errors.empty() ? 1 : static_cast<int>(result.errors[0].code);
}

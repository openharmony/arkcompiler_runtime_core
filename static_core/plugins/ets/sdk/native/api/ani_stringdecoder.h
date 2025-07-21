/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef UTIL_ANI_STRINGDECODER_H
#define UTIL_ANI_STRINGDECODER_H

#include <ani.h>
#include <string>
#include "unicode/ucnv.h"

namespace ark::ets::sdk::util {
class UtilHelper {
public:
    static UConverter *CreateConverter(const std::string &encStr, UErrorCode &codeflag);
    static std::string ConvertToString(UChar *uchar);
};

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class StringDecoder {
public:
    explicit StringDecoder(const std::string &encoding);
    ~StringDecoder()
    {
        ucnv_close(conv_);
    }
    ani_string Write(ani_env *env, const char *source, int32_t byteOffset, uint32_t length, UBool flush = 0);
    ani_string End(ani_env *env, const char *source, int32_t byteOffset, int32_t length);
    ani_string End(ani_env *env);

    static StringDecoder *Unwrapp(ani_env *env, ani_object object)
    {
        ani_long stringDecoder {};
        if (ANI_OK != env->Object_GetFieldByName_Long(object, "nativeDecoder_", &stringDecoder)) {
            return nullptr;
        }
        return reinterpret_cast<StringDecoder *>(stringDecoder);
    }

private:
    ani_object ThrowError(ani_env *env, const std::string &message);
    void FreedMemory(UChar *&pData);
    const char *pend_ {};
    int pendingLen_ {};
    UConverter *conv_ {};
};
}  // namespace ark::ets::sdk::util
#endif  // UTIL_ANI_STRINGDECODER_H

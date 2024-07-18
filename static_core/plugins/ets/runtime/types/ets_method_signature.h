/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_FFI_CLASSES_ETS_METHOD_SIGNATURE_H_
#define PANDA_PLUGINS_ETS_RUNTIME_FFI_CLASSES_ETS_METHOD_SIGNATURE_H_

#include "libpandabase/utils/logger.h"
#include "runtime/include/method.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_value.h"

namespace ark::ets {

// Arguments type separated from return type by ":". Object names bounded by 'L' and ';'
class EtsMethodSignature {
public:
    explicit EtsMethodSignature(const PandaString &signature)
    {
        size_t dots = signature.find(':');
        // Return if ':' wasn't founded or was founded at the end
        if (dots == PandaString::npos || dots == signature.size() - 1) {
            return;
        }
        // Process return type after ':'
        if (ProcessParameter(signature, dots + 1) != signature.size() - 1) {
            return;
        }
        // Process arguments
        size_t i;
        for (i = 0; i < dots; i++) {
            i = ProcessParameter(signature, i);
            if (i == PandaString::npos) {
                return;
            }
        }
        isValid_ = true;
        for (auto &paramType : paramTypes_) {
            pandaProto_.GetRefTypes().push_back(paramType);
        }
    }

    size_t ProcessParameter(const PandaString &signature, size_t i)
    {
        EtsType paramType = GetTypeByFirstChar(signature[i]);
        // Check that type is valid and return type is not void
        if (paramType == EtsType::UNKNOWN || (paramType == EtsType::VOID && (i != signature.size() - 1))) {
            return PandaString::npos;
        }
        pandaProto_.GetShorty().push_back(ets::ConvertEtsTypeToPandaType(paramType));

        if (paramType != EtsType::OBJECT) {
            return i;
        }
        size_t nameStart = i;
        i = ProcessObjectParameter(signature, i);
        if (i == PandaString::npos) {
            return i;
        }
        paramTypes_.push_back(signature.substr(nameStart, i + 1 - nameStart));
        return i;
    }

    bool IsValid()
    {
        return isValid_;
    }

    Method::Proto &GetProto()
    {
        return pandaProto_;
    }

private:
    PandaSmallVector<PandaString> paramTypes_;
    Method::Proto pandaProto_;
    bool isValid_ {false};

    size_t ProcessObjectParameter(const PandaString &signature, size_t i)
    {
        while (signature[i] == '[') {
            ++i;
            if (i >= signature.size()) {
                return PandaString::npos;
            }
        }
        if (GetTypeByFirstChar(signature[i]) == EtsType::UNKNOWN) {
            return PandaString::npos;
        }

        if (signature[i] == 'L') {
            // Get object name bounded by 'L' and ';'
            size_t prevI = i;
            i = signature.find(';', i);
            if (i == PandaString::npos || i == (prevI + 1)) {
                return PandaString::npos;
            }
        }
        return i;
    }
    EtsType GetTypeByFirstChar(char c)
    {
        switch (c) {
            case 'L':
            case '[':
                return EtsType::OBJECT;
            case 'Z':
                return EtsType::BOOLEAN;
            case 'B':
                return EtsType::BYTE;
            case 'C':
                return EtsType::CHAR;
            case 'S':
                return EtsType::SHORT;
            case 'I':
                return EtsType::INT;
            case 'J':
                return EtsType::LONG;
            case 'F':
                return EtsType::FLOAT;
            case 'D':
                return EtsType::DOUBLE;
            case 'V':
                return EtsType::VOID;
            default:
                return EtsType::UNKNOWN;
        }
    }
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_FFI_CLASSES_ETS_METHOD_SIGNATURE_H_
/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/intrinsics/helpers/reflection_helpers.h"

namespace ark::ets::intrinsics {

extern "C" EtsBoolean IsLiteralInitializedInterfaceImpl(EtsObject *target)
{
    return helpers::IsLiteralInitializedInterface(target);
}

extern "C" EtsBoolean IsFuncObjAsyncImpl(EtsObject *target)
{
    static constexpr std::string_view ANNO_NAME = "Lstd/core/AsyncFunctionObject;";

    if (target == nullptr) {
        ThrowNullPointerException();
        return false;
    }

    auto *runtimeClass = target->GetClass()->GetRuntimeClass();
    const panda_file::File &pf = *runtimeClass->GetPandaFile();
    panda_file::ClassDataAccessor cda(pf, runtimeClass->GetFileId());
    EtsBoolean result = false;
    cda.EnumerateAnnotations([&pf, &result](panda_file::File::EntityId annId) {
        panda_file::AnnotationDataAccessor ada(pf, annId);
        const char *className = utf::Mutf8AsCString(pf.GetStringData(ada.GetClassId()).data);
        if (className == ANNO_NAME) {
            result = true;
            return;
        }
    });

    return result;
}

}  // namespace ark::ets::intrinsics

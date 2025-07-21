/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_STACKTRACE_ELEMENT_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_STACKTRACE_ELEMENT_H

#include "macros.h"
#include "mem/object_pointer.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "types/ets_primitives.h"

namespace ark::ets {

class EtsStackTraceElement : public ObjectHeader {
public:
    EtsStackTraceElement() = delete;
    ~EtsStackTraceElement() = delete;

    NO_COPY_SEMANTIC(EtsStackTraceElement);
    NO_MOVE_SEMANTIC(EtsStackTraceElement);

    EtsObject *AsObject()
    {
        return EtsObject::FromCoreType(this);
    }

    const EtsObject *AsObject() const
    {
        return EtsObject::FromCoreType(this);
    }

    inline void SetClassName(EtsString *className)
    {
        ASSERT(className != nullptr);
        ObjectAccessor::SetObject(this, MEMBER_OFFSET(EtsStackTraceElement, className_),
                                  className->AsObject()->GetCoreType());
    }

    inline void SetMethodName(EtsString *methodName)
    {
        ASSERT(methodName != nullptr);
        ObjectAccessor::SetObject(this, MEMBER_OFFSET(EtsStackTraceElement, methodName_),
                                  methodName->AsObject()->GetCoreType());
    }

    inline void SetSourceFileName(EtsString *sourceFileName)
    {
        ASSERT(sourceFileName != nullptr);
        ObjectAccessor::SetObject(this, MEMBER_OFFSET(EtsStackTraceElement, sourceFileName_),
                                  sourceFileName->AsObject()->GetCoreType());
    }

    inline void SetLineNumber(EtsInt lineNumber)
    {
        ObjectAccessor::SetPrimitive(this, MEMBER_OFFSET(EtsStackTraceElement, lineNumber_), lineNumber);
    }

    inline static EtsStackTraceElement *Create(EtsCoroutine *etsCoroutine)
    {
        EtsClass *klass = etsCoroutine->GetPandaVM()->GetClassLinker()->GetClass(
            panda_file_items::class_descriptors::STACK_TRACE_ELEMENT.data());
        EtsObject *etsObject = EtsObject::Create(etsCoroutine, klass);
        return reinterpret_cast<EtsStackTraceElement *>(etsObject);
    }

private:
    ObjectPointer<EtsString> className_;
    ObjectPointer<EtsString> methodName_;
    ObjectPointer<EtsString> sourceFileName_;
    FIELD_UNUSED EtsInt lineNumber_;  // note alignment
};

}  // namespace ark::ets
#endif
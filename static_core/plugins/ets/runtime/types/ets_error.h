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

#ifndef PANDA_PLUGINS_ETS_ERROR_H
#define PANDA_PLUGINS_ETS_ERROR_H

#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_stacktrace_element.h"

namespace ark::ets {

namespace test {
class EtsErrorTest;
}  // namespace test

// class Error is mirror class for std.core.Error
// Currently, there is no necessity to declare this class as mirror in yaml files,
// this this class is auxiliary for describing std.core error classes as mirror classes.
class EtsError : public EtsObject {
public:
    NO_COPY_SEMANTIC(EtsError);
    NO_MOVE_SEMANTIC(EtsError);

    EtsError() = delete;
    ~EtsError() = delete;

    static EtsError *Create(EtsExecutionContext *executionCtx, EtsClass *klass)
    {
        return FromEtsObject(EtsObject::Create(executionCtx, klass));
    }

    static EtsError *FromEtsObject(EtsObject *object)
    {
        return static_cast<EtsError *>(object);
    }

    EtsObject *AsObject()
    {
        return this;
    }

    const EtsObject *AsObject() const
    {
        return this;
    }

    void SetName(EtsExecutionContext *executionCtx, EtsString *name)
    {
        ASSERT(name != nullptr);
        ObjectAccessor::SetObject(executionCtx->GetMT(), this, MEMBER_OFFSET(EtsError, name_), name->GetCoreType());
    }

    void SetMessage(EtsExecutionContext *executionCtx, EtsString *msg)
    {
        ASSERT(msg != nullptr);
        ObjectAccessor::SetObject(executionCtx->GetMT(), this, MEMBER_OFFSET(EtsError, message_), msg->GetCoreType());
    }

    void SetStack(EtsExecutionContext *executionCtx, EtsString *stack)
    {
        ASSERT(stack != nullptr);
        ObjectAccessor::SetObject(executionCtx->GetMT(), this, MEMBER_OFFSET(EtsError, stack_), stack->GetCoreType());
    }

    void SetStackLines(EtsExecutionContext *executionCtx, EtsTypedObjectArray<EtsStackTraceElement> *stackLines)
    {
        ASSERT(stackLines != nullptr);
        ObjectAccessor::SetObject(executionCtx->GetMT(), this, MEMBER_OFFSET(EtsError, stackLines_),
                                  stackLines->GetCoreType());
    }

private:
    ObjectPointer<EtsString> name_;
    ObjectPointer<EtsString> message_;
    ObjectPointer<EtsTypedObjectArray<EtsStackTraceElement>> stackLines_;
    ObjectPointer<EtsString> stack_;  // non-mandatory field in `class Error`
    ObjectPointer<EtsObject> cause_;  // non-mandatory field in `class Error`
    FIELD_UNUSED ObjectPointer<EtsInt> code_;

    friend class test::EtsErrorTest;
};

// Purpose of this class is to have preallocated OOM object in runtime, that can be useful in situation
// when all heap is filled and we don't have space to allocate stdlib OOM error.
class EtsOutOfMemoryError final : public EtsError {
public:
    static constexpr std::string_view OOM_ERROR_NAME = "OutOfMemoryError";
    // Default message should be empty string to avoid allocations in .toString()
    // NOLINTNEXTLINE(readability-redundant-string-init)
    static constexpr std::string_view DEFAULT_OOM_MSG = "";
    static constexpr std::string_view DEFAULT_OOM_STACK = "Heap is full, no space to collect stack";

public:
    NO_COPY_SEMANTIC(EtsOutOfMemoryError);
    NO_MOVE_SEMANTIC(EtsOutOfMemoryError);

    EtsOutOfMemoryError() = delete;
    ~EtsOutOfMemoryError() = delete;

    static EtsOutOfMemoryError *Create(EtsExecutionContext *executionCtx)
    {
        [[maybe_unused]] EtsHandleScope scope(executionCtx);

        auto oomH = EtsHandle<EtsOutOfMemoryError>(
            executionCtx, FromError(EtsError::Create(executionCtx, PlatformTypes(executionCtx)->coreOutOfMemoryError)));

        ASSERT(oomH.GetPtr() != nullptr);
        auto *name = EtsString::CreateFromMUtf8(OOM_ERROR_NAME.data());
        oomH->SetName(executionCtx, name);

        auto *message = EtsString::CreateFromMUtf8(DEFAULT_OOM_MSG.data());
        oomH->SetMessage(executionCtx, message);

        auto *stack = EtsString::CreateFromMUtf8(DEFAULT_OOM_STACK.data());
        oomH->SetStack(executionCtx, stack);

        auto *stackLines = EtsTypedObjectArray<EtsStackTraceElement>::Create(
            PlatformTypes(executionCtx)->arkruntimeStackTraceElement, 0U);
        oomH->SetStackLines(executionCtx, stackLines);

        return oomH.GetPtr();
    }

    static EtsOutOfMemoryError *FromError(EtsError *error)
    {
        ASSERT(error->GetClass() == PlatformTypes()->coreOutOfMemoryError);
        return static_cast<EtsOutOfMemoryError *>(error);
    }
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_ERROR_H

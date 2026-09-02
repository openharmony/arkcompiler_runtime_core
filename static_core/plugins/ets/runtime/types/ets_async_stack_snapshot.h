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
#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ASYNC_STACK_SNAPSHOT_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ASYNC_STACK_SNAPSHOT_H

#include "libarkbase/macros.h"
#include "libarkbase/mem/object_pointer.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_primitives.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "runtime/include/object_accessor-inl.h"

namespace ark::ets {

namespace test {
class EtsAsyncStackSnapshotTest;
}  // namespace test

class EtsAsyncStackSourceIdentity : public EtsObject {
public:
    EtsAsyncStackSourceIdentity() = delete;
    ~EtsAsyncStackSourceIdentity() = delete;

    NO_COPY_SEMANTIC(EtsAsyncStackSourceIdentity);
    NO_MOVE_SEMANTIC(EtsAsyncStackSourceIdentity);

    static EtsAsyncStackSourceIdentity *Create(EtsExecutionContext *executionCtx)
    {
        auto *object = EtsObject::Create(executionCtx, PlatformTypes(executionCtx)->coreAsyncStackSourceIdentity);
        return FromEtsObject(object);
    }

    static EtsAsyncStackSourceIdentity *FromEtsObject(EtsObject *object)
    {
        return static_cast<EtsAsyncStackSourceIdentity *>(object);
    }

    EtsObject *AsObject()
    {
        return this;
    }

    const EtsObject *AsObject() const
    {
        return this;
    }

    EtsString *GetPandaFile(EtsExecutionContext *executionCtx) const
    {
        auto *object = ObjectAccessor::GetObject(executionCtx->GetMT(), this,
                                                 MEMBER_OFFSET(EtsAsyncStackSourceIdentity, pandaFile_));
        return EtsString::FromEtsObject(EtsObject::FromCoreType(object));
    }

    void SetPandaFile(EtsExecutionContext *executionCtx, EtsString *pandaFile)
    {
        ObjectAccessor::SetObject(executionCtx->GetMT(), this, MEMBER_OFFSET(EtsAsyncStackSourceIdentity, pandaFile_),
                                  pandaFile->GetCoreType());
    }

    EtsLong GetMethodId() const
    {
        return methodId_;
    }

    void SetMethodId(EtsLong methodId)
    {
        methodId_ = methodId;
    }

    EtsInt GetBytecodeOffset() const
    {
        return bytecodeOffset_;
    }

    void SetBytecodeOffset(EtsInt bytecodeOffset)
    {
        bytecodeOffset_ = bytecodeOffset;
    }

    EtsLong GetNativePc() const
    {
        return nativePc_;
    }

    void SetNativePc(EtsLong nativePc)
    {
        nativePc_ = nativePc;
    }

private:
    ObjectPointer<EtsString> pandaFile_;
#ifdef PANDA_32_BIT_MANAGED_POINTER
    EtsInt bytecodeOffset_;
    EtsLong methodId_;
    EtsLong nativePc_;
#else
    EtsLong methodId_;
    EtsLong nativePc_;
    EtsInt bytecodeOffset_;
#endif

    friend class test::EtsAsyncStackSnapshotTest;
};

class EtsAsyncStackFrame : public EtsObject {
public:
    EtsAsyncStackFrame() = delete;
    ~EtsAsyncStackFrame() = delete;

    NO_COPY_SEMANTIC(EtsAsyncStackFrame);
    NO_MOVE_SEMANTIC(EtsAsyncStackFrame);

    static EtsAsyncStackFrame *Create(EtsExecutionContext *executionCtx)
    {
        auto *object = EtsObject::Create(executionCtx, PlatformTypes(executionCtx)->coreAsyncStackFrame);
        return FromEtsObject(object);
    }

    static EtsAsyncStackFrame *FromEtsObject(EtsObject *object)
    {
        return static_cast<EtsAsyncStackFrame *>(object);
    }

    EtsObject *AsObject()
    {
        return this;
    }

    const EtsObject *AsObject() const
    {
        return this;
    }

    EtsString *GetFunctionName(EtsExecutionContext *executionCtx) const
    {
        auto *object =
            ObjectAccessor::GetObject(executionCtx->GetMT(), this, MEMBER_OFFSET(EtsAsyncStackFrame, functionName_));
        return EtsString::FromEtsObject(EtsObject::FromCoreType(object));
    }

    void SetFunctionName(EtsExecutionContext *executionCtx, EtsString *functionName)
    {
        ObjectAccessor::SetObject(executionCtx->GetMT(), this, MEMBER_OFFSET(EtsAsyncStackFrame, functionName_),
                                  functionName->GetCoreType());
    }

    EtsAsyncStackSourceIdentity *GetSourceIdentity(EtsExecutionContext *executionCtx) const
    {
        return EtsAsyncStackSourceIdentity::FromEtsObject(EtsObject::FromCoreType(ObjectAccessor::GetObject(
            executionCtx->GetMT(), this, MEMBER_OFFSET(EtsAsyncStackFrame, sourceIdentity_))));
    }

    void SetSourceIdentity(EtsExecutionContext *executionCtx, EtsAsyncStackSourceIdentity *sourceIdentity)
    {
        ObjectAccessor::SetObject(executionCtx->GetMT(), this, MEMBER_OFFSET(EtsAsyncStackFrame, sourceIdentity_),
                                  sourceIdentity->GetCoreType());
    }

private:
    ObjectPointer<EtsString> functionName_;
    ObjectPointer<EtsAsyncStackSourceIdentity> sourceIdentity_;

    friend class test::EtsAsyncStackSnapshotTest;
};

class EtsAsyncStackSegment : public EtsObject {
public:
    EtsAsyncStackSegment() = delete;
    ~EtsAsyncStackSegment() = delete;

    NO_COPY_SEMANTIC(EtsAsyncStackSegment);
    NO_MOVE_SEMANTIC(EtsAsyncStackSegment);

    static EtsAsyncStackSegment *Create(EtsExecutionContext *executionCtx)
    {
        auto *object = EtsObject::Create(executionCtx, PlatformTypes(executionCtx)->coreAsyncStackSegment);
        return FromEtsObject(object);
    }

    static EtsAsyncStackSegment *FromEtsObject(EtsObject *object)
    {
        return static_cast<EtsAsyncStackSegment *>(object);
    }

    EtsObject *AsObject()
    {
        return this;
    }

    const EtsObject *AsObject() const
    {
        return this;
    }

    EtsString *GetDescription(EtsExecutionContext *executionCtx) const
    {
        auto *object =
            ObjectAccessor::GetObject(executionCtx->GetMT(), this, MEMBER_OFFSET(EtsAsyncStackSegment, description_));
        return EtsString::FromEtsObject(EtsObject::FromCoreType(object));
    }

    void SetDescription(EtsExecutionContext *executionCtx, EtsString *description)
    {
        ObjectAccessor::SetObject(executionCtx->GetMT(), this, MEMBER_OFFSET(EtsAsyncStackSegment, description_),
                                  description->GetCoreType());
    }

    EtsLong GetGeneration() const
    {
        return generation_;
    }

    void SetGeneration(EtsLong generation)
    {
        generation_ = generation;
    }

    EtsObjectArray *GetFrames(EtsExecutionContext *executionCtx) const
    {
        return EtsObjectArray::FromCoreType(
            ObjectAccessor::GetObject(executionCtx->GetMT(), this, MEMBER_OFFSET(EtsAsyncStackSegment, frames_)));
    }

    void SetFrames(EtsExecutionContext *executionCtx, EtsObjectArray *frames)
    {
        ObjectAccessor::SetObject(executionCtx->GetMT(), this, MEMBER_OFFSET(EtsAsyncStackSegment, frames_),
                                  frames->GetCoreType());
    }

private:
    ObjectPointer<EtsString> description_;
    ObjectPointer<EtsObjectArray> frames_;
    EtsLong generation_;

    friend class test::EtsAsyncStackSnapshotTest;
};

class EtsAsyncStackSnapshot : public EtsObject {
public:
    EtsAsyncStackSnapshot() = delete;
    ~EtsAsyncStackSnapshot() = delete;

    NO_COPY_SEMANTIC(EtsAsyncStackSnapshot);
    NO_MOVE_SEMANTIC(EtsAsyncStackSnapshot);

    static EtsAsyncStackSnapshot *Create(EtsExecutionContext *executionCtx)
    {
        auto *object = EtsObject::Create(executionCtx, PlatformTypes(executionCtx)->coreAsyncStackSnapshot);
        return FromEtsObject(object);
    }

    static EtsAsyncStackSnapshot *FromEtsObject(EtsObject *object)
    {
        return static_cast<EtsAsyncStackSnapshot *>(object);
    }

    EtsObject *AsObject()
    {
        return this;
    }

    const EtsObject *AsObject() const
    {
        return this;
    }

    EtsLong GetGeneration() const
    {
        return generation_;
    }

    void SetGeneration(EtsLong generation)
    {
        generation_ = generation;
    }

    EtsObjectArray *GetSegments(EtsExecutionContext *executionCtx) const
    {
        return EtsObjectArray::FromCoreType(
            ObjectAccessor::GetObject(executionCtx->GetMT(), this, MEMBER_OFFSET(EtsAsyncStackSnapshot, segments_)));
    }

    void SetSegments(EtsExecutionContext *executionCtx, EtsObjectArray *segments)
    {
        ObjectAccessor::SetObject(executionCtx->GetMT(), this, MEMBER_OFFSET(EtsAsyncStackSnapshot, segments_),
                                  segments->GetCoreType());
    }

private:
    ObjectPointer<EtsObjectArray> segments_;
    EtsLong generation_;

    friend class test::EtsAsyncStackSnapshotTest;
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ASYNC_STACK_SNAPSHOT_H

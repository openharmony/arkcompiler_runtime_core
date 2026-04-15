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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ASYNC_CONTEXT_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ASYNC_CONTEXT_H

#include <cstdint>
#include "libarkbase/mem/object_pointer.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_primitives.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_promise.h"

namespace ark::ets {

class EtsExecutionContext;

namespace test {
class EtsAsyncContextTest;
}  // namespace test

class EtsAsyncContext final : public EtsObject {
public:
    /**
     * Creates a new EtsAsyncContext instance.
     * executionCtx - current execution context
     * refSize, primSize and pc are coming from compiler (-1 means we are in interpreter frame)
     * refSize - maximum number of references in current graph
     * primSize - maximum number of primitives in current graph
     * pc - the program counter of the suspension point, used for stack unwinding in stack
     */
    static EtsAsyncContext *Create(EtsExecutionContext *executionCtx, int32_t refSize = -1, int32_t primSize = -1,
                                   int32_t pc = -1);
    static EtsAsyncContext *GetCurrent(EtsExecutionContext *executionCtx);

    inline void SetReturnValue(EtsExecutionContext *executionCtx, EtsPromise *returnValue);
    inline void SetAwaitee(EtsExecutionContext *executionCtx, EtsPromise *awaitee);
    inline void SetRefValues(EtsExecutionContext *executionCtx, EtsObjectArray *refValues);
    inline void SetPrimValues(EtsExecutionContext *executionCtx, EtsLongArray *primValues);
    inline void SetRefCount(EtsLong refCount);
    inline void SetPrimCount(EtsLong primCount);
    inline void SetFrameOffsets(EtsExecutionContext *executionCtx, EtsShortArray *frameOffsets);
    inline void SetCompiledCode(EtsLong compiledCode);
    inline void SetAwaitId(EtsLong awaitId);
    inline void SetPc(EtsInt pc);

    inline EtsPromise *GetReturnValue(EtsExecutionContext *executionCtx) const;
    inline EtsPromise *GetAwaitee(EtsExecutionContext *executionCtx) const;
    inline EtsObjectArray *GetRefValues(EtsExecutionContext *executionCtx) const;
    inline EtsLongArray *GetPrimValues(EtsExecutionContext *executionCtx) const;
    inline EtsLong GetRefCount() const;
    inline EtsLong GetPrimCount() const;
    inline EtsShortArray *GetFrameOffsets(EtsExecutionContext *executionCtx) const;
    inline EtsLong GetCompiledCode() const;
    inline EtsLong GetAwaitId() const;
    inline EtsInt GetPc() const;

    void AddReference(EtsExecutionContext *executionCtx, uint32_t idx, EtsObject *ref);
    void AddPrimitive(EtsExecutionContext *executionCtx, uint32_t idx, EtsLong primitive);
    EtsShort GetVregOffset(EtsExecutionContext *executionCtx, uint32_t idx) const;

    EtsObject *AsObject()
    {
        return this;
    }

    const EtsObject *AsObject() const
    {
        return this;
    }

    static EtsAsyncContext *FromCoreType(ObjectHeader *asyncCtx)
    {
        return reinterpret_cast<EtsAsyncContext *>(asyncCtx);
    }

    static EtsAsyncContext *FromEtsObject(EtsObject *asyncCtx)
    {
        return reinterpret_cast<EtsAsyncContext *>(asyncCtx);
    }

    static constexpr uint32_t GetAwaiteeOffset()
    {
        return MEMBER_OFFSET(EtsAsyncContext, awaitee_);
    }
    static constexpr uint32_t GetReturnValueOffset()
    {
        return MEMBER_OFFSET(EtsAsyncContext, returnValue_);
    }
    static constexpr uint32_t GetRefValuesOffset()
    {
        return MEMBER_OFFSET(EtsAsyncContext, refValues_);
    }
    static constexpr uint32_t GetPrimValuesOffset()
    {
        return MEMBER_OFFSET(EtsAsyncContext, primValues_);
    }
    static constexpr uint32_t GetFrameOffsetsOffset()
    {
        return MEMBER_OFFSET(EtsAsyncContext, frameOffsets_);
    }
    static constexpr uint32_t GetRefCountOffset()
    {
        return MEMBER_OFFSET(EtsAsyncContext, refCount_);
    }
    static constexpr uint32_t GetPrimCountOffset()
    {
        return MEMBER_OFFSET(EtsAsyncContext, primCount_);
    }
    static constexpr uint32_t GetAwaitIdOffset()
    {
        return MEMBER_OFFSET(EtsAsyncContext, awaitId_);
    }
    static constexpr uint32_t GetCompiledCodeOffset()
    {
        return MEMBER_OFFSET(EtsAsyncContext, compiledCode_);
    }

private:
    ObjectPointer<EtsPromise> awaitee_;
    ObjectPointer<EtsPromise> returnValue_;

    ObjectPointer<EtsObjectArray> refValues_;
    ObjectPointer<EtsLongArray> primValues_;
    ObjectPointer<EtsShortArray> frameOffsets_;

// NOTE: Refactor layout #34102
#ifdef PANDA_32_BIT_MANAGED_POINTER
    EtsInt pc_;
#endif
    EtsLong refCount_;
    EtsLong primCount_;

    EtsLong awaitId_;
    EtsLong compiledCode_;
#ifndef PANDA_32_BIT_MANAGED_POINTER
    EtsInt pc_;
#endif

    friend class test::EtsAsyncContextTest;
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ASYNC_CONTEXT_H

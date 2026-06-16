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

#include "runtime/include/managed_thread.h"
#include "plugins/ets/runtime/ets_execution_context.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/types/ets_reflect_method.h"
#include "runtime/arch/helpers.h"
#include "libarkfile/include/type.h"
#include "plugins/ets/runtime/types/ets_box_primitive-inl.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/types/ets_primitives.h"

#include <array>
#include <optional>
#include <type_traits>
#include <variant>

// Explicitly instantiation for arm.
template class ark::ets::EtsBoxPrimitive<ark::ets::EtsDouble>;
template class ark::ets::EtsBoxPrimitive<ark::ets::EtsLong>;
template class ark::ets::EtsBoxPrimitive<ark::ets::EtsShort>;
template class ark::ets::EtsBoxPrimitive<ark::ets::EtsChar>;
template class ark::ets::EtsBoxPrimitive<ark::ets::EtsFloat>;
template class ark::ets::EtsBoxPrimitive<ark::ets::EtsByte>;
template class ark::ets::EtsBoxPrimitive<ark::ets::EtsBoolean>;
template class ark::ets::EtsBoxPrimitive<ark::ets::EtsInt>;

namespace ark::ets::entrypoints {

extern "C" void EtsProxyEntryPoint();

const void *GetEtsProxyEntryPoint()
{
    return reinterpret_cast<const void *>(EtsProxyEntryPoint);
}

#if defined(__clang__)
#pragma clang diagnostic push
// CC-OFFNXT(warning_suppression) gcc false positive
#pragma clang diagnostic ignored "-Wgnu-label-as-value"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
// CC-OFFNXT(warning_suppression) gcc false positive
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

static constexpr size_t ARG_INLINE_CAPACITY = 8U;

// Variant holding either a GC-rooted handle (reference arg, created eagerly in phase 1)
// or a raw primitive value (boxed in phase 2, after all refs are already rooted).
// Types mirror exactly what ARCH_COPY_METHOD_ARGS dispatches to Write (U32/U64 excluded —
// those are not valid ETS argument types and are caught in ArgValueWriter::Write).
using ArgRaw = std::variant<EtsHandle<EtsObject>, int8_t, uint8_t, int16_t, uint16_t, int32_t, float, double, int64_t>;

class ArgValueWriter final {
public:
    NO_COPY_SEMANTIC(ArgValueWriter);
    NO_MOVE_SEMANTIC(ArgValueWriter);

    explicit ArgValueWriter(PandaSmallVector<ArgRaw, ARG_INLINE_CAPACITY> *args, EtsExecutionContext *executionCtx)
        : args_(args), executionCtx_(executionCtx)
    {
    }

    ~ArgValueWriter() = default;

    template <class T>
    ALWAYS_INLINE void Write(T v)
    {
        if constexpr (std::is_same_v<T, ObjectHeader **>) {
            // Reference arg: root the handle immediately so GC cannot move the object
            // before phase-2 boxing allocations run.
            args_->emplace_back(EtsHandle<EtsObject>(executionCtx_, EtsObject::FromCoreType(*v)));
        } else if constexpr (std::is_same_v<T, uint32_t> || std::is_same_v<T, uint64_t>) {
            // U32/U64 are not valid ETS argument types;
            LOG(FATAL, ETS) << "Unexpected unsigned argument type in proxy dispatch";
        } else {
            args_->emplace_back(v);
        }
    }

private:
    PandaSmallVector<ArgRaw, ARG_INLINE_CAPACITY> *args_;
    EtsExecutionContext *executionCtx_;
};

static bool UnboxValue(EtsObject *boxedValue, EtsType type, Value *unboxedValue, EtsExecutionContext *executionCtx)
{
    ASSERT(unboxedValue != nullptr);
    ASSERT(executionCtx != nullptr);

    auto *platformTypes = PlatformTypes(executionCtx);

// CC-OFFNXT(G.PRE.02-CPP) code generation
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TYPE_CASE(boxedObj, T, resultPtr, boxedClass)                              \
    {                                                                              \
        /* CC-OFFNXT(G.PRE.02) codegen */                                          \
        if (LIKELY(boxedObj != nullptr && (boxedObj->GetClass() == boxedClass))) { \
            /* CC-OFFNXT(G.PRE.02) codegen */                                      \
            *resultPtr = Value(EtsBoxPrimitive<T>::Unbox(boxedObj));               \
            return true; /* CC-OFF(G.PRE.05) codegen */                            \
        }                                                                          \
        break; /* CC-OFF(G.PRE.05) codegen */                                      \
    }

    switch (type) {
        case EtsType::BOOLEAN:
            TYPE_CASE(boxedValue, EtsBoolean, unboxedValue, platformTypes->coreBoolean)
        case EtsType::BYTE:
            TYPE_CASE(boxedValue, EtsByte, unboxedValue, platformTypes->coreByte)
        case EtsType::CHAR:
            TYPE_CASE(boxedValue, EtsChar, unboxedValue, platformTypes->coreChar)
        case EtsType::SHORT:
            TYPE_CASE(boxedValue, EtsShort, unboxedValue, platformTypes->coreShort)
        case EtsType::INT:
            TYPE_CASE(boxedValue, EtsInt, unboxedValue, platformTypes->coreInt)
        case EtsType::LONG:
            TYPE_CASE(boxedValue, EtsLong, unboxedValue, platformTypes->coreLong)
        case EtsType::FLOAT:
            TYPE_CASE(boxedValue, EtsFloat, unboxedValue, platformTypes->coreFloat)
        case EtsType::DOUBLE:
            TYPE_CASE(boxedValue, EtsDouble, unboxedValue, platformTypes->coreDouble)
        case EtsType::OBJECT: {
            if (boxedValue == nullptr) {
                *unboxedValue = Value(nullptr);
            } else {
                *unboxedValue = Value(boxedValue->GetCoreType());
            }
            return true;
        }
        case EtsType::UNKNOWN:
            break;
        default:
            LOG(FATAL, ETS) << "Unexpected argument type";
    }
    return false;
}

static int64_t UnboxResult(EtsExecutionContext *executionCtx, EtsMethod *ifaceMethod, Value value)
{
    if (UNLIKELY(executionCtx->GetMT()->HasPendingException())) {
        return 0;
    }

    auto *boxedResult = value.GetAs<ObjectHeader *>();
    auto effectiveReturnType = ifaceMethod->GetEffectiveReturnValueType();
    if (effectiveReturnType == EtsType::VOID) {
        return 0;
    }

    Value unboxedResult;
    if (!UnboxValue(EtsObject::FromCoreType(boxedResult), effectiveReturnType, &unboxedResult, executionCtx)) {
        PandaOStringStream msg;
        msg << "result has type " << EtsTypeToString(effectiveReturnType) << ", but got ";
        if (boxedResult == nullptr) {
            msg << "undefined";
        } else if (boxedResult == executionCtx->GetNullValue()) {
            msg << "null";
        } else {
            msg << EtsObject::FromCoreType(boxedResult)->GetClass()->GetName()->GetMutf8();
        }
        ark::ets::ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreIllegalArgumentError,
                                    msg.str().c_str());
    }
    return unboxedResult.GetAsLong();
}

static std::optional<EtsHandle<EtsObject>> BoxArg(const ArgRaw &arg, EtsExecutionContext *executionCtx)
{
    return std::visit(
        [executionCtx](auto v) -> std::optional<EtsHandle<EtsObject>> {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, EtsHandle<EtsObject>>) {
                return v;
            } else {
                auto *box = EtsBoxPrimitive<T>::Create(executionCtx, v);
                if (UNLIKELY(box == nullptr)) {
                    return std::nullopt;
                }
                return EtsHandle<EtsObject>(executionCtx, box->AsObject());
            }
        },
        arg);
}

static Value PrepareArgumentsAndInvoke(const EtsHandle<EtsObject> &thisH, EtsMethod *ifaceMethod,
                                       const PandaSmallVector<ArgRaw, ARG_INLINE_CAPACITY> &rawArgs,
                                       EtsExecutionContext *executionCtx)
{
    auto *platformTypes = PlatformTypes(executionCtx);

    EtsHandle<EtsReflectMethod> reflectMethodH(executionCtx,
                                               EtsReflectMethod::CreateFromEtsMethod(executionCtx, ifaceMethod));
    if (UNLIKELY(reflectMethodH.GetPtr() == nullptr)) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return Value(0U);
    }

    std::array<Value, 3U> invokeArgs;

    EtsMethod *methodToInvoke = nullptr;

    if (ifaceMethod->IsSetter()) {
        ASSERT(rawArgs.size() == 1U);
        auto boxed = BoxArg(rawArgs[0], executionCtx);
        if (UNLIKELY(!boxed.has_value())) {
            ASSERT(executionCtx->GetMT()->HasPendingException());
            return Value(0U);
        }
        // nullptr in case of arg = `undefined`.
        auto *boxedPtr = boxed->GetPtr();
        invokeArgs[2U] = boxedPtr == nullptr ? Value(nullptr) : Value(boxedPtr->GetCoreType());
        methodToInvoke = platformTypes->coreReflectProxyInvokeSet;
    } else if (ifaceMethod->IsGetter()) {
        ASSERT(rawArgs.empty());
        methodToInvoke = platformTypes->coreReflectProxyInvokeGet;
    } else {
        EtsHandle<EtsObjectArray> argsArrayH(executionCtx,
                                             EtsObjectArray::Create(platformTypes->coreObject, rawArgs.size()));
        if (UNLIKELY(argsArrayH.GetPtr() == nullptr)) {
            ASSERT(executionCtx->GetMT()->HasPendingException());
            return Value(0U);
        }

        for (size_t idx = 0; idx < rawArgs.size(); ++idx) {
            auto boxed = BoxArg(rawArgs[idx], executionCtx);
            if (UNLIKELY(!boxed.has_value())) {
                ASSERT(executionCtx->GetMT()->HasPendingException());
                return Value(0U);
            }
            argsArrayH->Set(idx, boxed->GetPtr());
        }
        invokeArgs[2U] = Value(argsArrayH->AsObject()->GetCoreType());
        methodToInvoke = platformTypes->coreReflectProxyInvoke;
    }

    // Read this/method from handles after all allocations.
    invokeArgs[0U] = Value(thisH->GetCoreType());
    invokeArgs[1U] = Value(reflectMethodH->AsObject()->GetCoreType());

    auto proxyFlag = CallFlags {CallFlags::IS_PROXY};
    return methodToInvoke->GetPandaMethod()->Invoke(executionCtx->GetMT(), invokeArgs.data(), proxyFlag);
}

/*
    In case of i2c the following flow take place:
    interpreter --> HandleCall --> i2c bridge --> call entrypoint(proxy bridge) --> call EtsProxyMethodInvoke

    When some method genereted by proxy called with N arguments, then runtime calls i2c bridge
    and some of arguments saved in caller-saved registers and some on the stack according to the target calling
    convention. Then i2c bridge calls proxy bridge (that was set as entrypoint to the Method *).
    Proxy bridge pushes on the stack all incoming argument registers with purpose of passing
    it as array `args` to the EtsProxyMethodInvoke.
    `inStackArgs` is array of arguments from the stack that was formed by i2c bridge.

    So, proxy bridge does 2 main things:
    1) maps signature of given managed interface function to the signature of `EtsProxyMethodInvoke`.
    2) forms arguments to have opportunity of passing N arguments as one array argument.

    // CC-OFFNXT(G.CMT.04-CPP) false positive
0x0000                               AMD64 STACK:

    |            |                top of the stack                  |
  a |            |                                                  |
  d |            ---------------------------------------------------- <-- call of EtsProxyMethodInvoke(%rdi, %rsi, %rdx)
  d |            | Arguments that pushed in registers by i2c (%rsi) |
  r |            |               GPR arguments                      |
  e |            |               FPR arguments                      |
  s |            ---------------------------------------------------- <-- call of proxy bridge
  s |            |               return address                     |
  e |            ----------------------------------------------------
  s |            |               in stack arguments (%rdx)          |
    V            ----------------------------------------------------
                 |               Method* (%rdi)                     |
                 ----------------------------------------------------
                 | ................................................ |
                                                                      <-- call of i2c bridge by HandleCall
0xFFFF
*/
extern "C" int64_t EtsProxyMethodInvoke(Method *method, uint8_t *args, uint8_t *inStackArgs)
{
    ASSERT(method != nullptr);
    ASSERT(args != nullptr);
    ASSERT(inStackArgs != nullptr);

    ASSERT_MANAGED_CODE();

    // Proxy instance override some interface method, handler.invoke should accept interface method.
    auto *ifaceMethod = EtsMethod::FromRuntimeMethod(method)->GetInterfaceMethodIfProxy();
    ASSERT(ifaceMethod != nullptr);

    auto *executionCtx = EtsExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);

    // Passed pointer to stack `args`, because it passed as GPR and FPR due to calling proxy bridge from i2c.
    Span<uint8_t> gprArgs(args, arch::ExtArchTraits<RUNTIME_ARCH>::GP_ARG_NUM_BYTES);
    Span<uint8_t> fprArgs(gprArgs.end(), arch::ExtArchTraits<RUNTIME_ARCH>::FP_ARG_NUM_BYTES);
    arch::ArgReader<RUNTIME_ARCH> argReader(gprArgs, fprArgs, inStackArgs);

    [[maybe_unused]] auto *readedMethod = argReader.Read<Method *>();
    ASSERT(readedMethod == method);

    // When i2c bridge calls proxy bridge, `this` should be always passed as second argument (method is first).
    auto *thisObj = EtsObject::FromCoreType(argReader.Read<ObjectHeader *>());
    [[maybe_unused]] EtsHandleScope scope(executionCtx);

    EtsHandle<EtsObject> thisH(executionCtx, thisObj);

    // Phase 1: read all args from registers/stack — no managed allocation.
    // Reference args create their handle immediately (GC-rooted before phase-2 boxing).
    PandaSmallVector<ArgRaw, ARG_INLINE_CAPACITY> rawArgs;
    ArgValueWriter writer(&rawArgs, executionCtx);

    ARCH_COPY_METHOD_ARGS(ifaceMethod->GetPandaMethod(), argReader, writer);

    // Phase 2: box primitives and invoke; reference args are already rooted from phase 1.
    Value result = PrepareArgumentsAndInvoke(thisH, ifaceMethod, rawArgs, executionCtx);
    // result is boxed Any value.
    return UnboxResult(executionCtx, ifaceMethod, result);
}

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

}  // namespace ark::ets::entrypoints

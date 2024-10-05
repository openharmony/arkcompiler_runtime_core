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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_INTEROP_COMMON_H_
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_INTEROP_COMMON_H_

#include "runtime/include/thread_scopes.h"
#include "runtime/mem/refstorage/global_object_storage.h"
#include "plugins/ets/runtime/interop_js/logger.h"
#include "plugins/ets/runtime/types/ets_bigint.h"
#include "utils/small_vector.h"

#include <node_api.h>

namespace ark::ets::interop::js {

constexpr uint32_t BIGINT_BITMASK_30 = 0x3FFFFFFF;
constexpr size_t BIGINT_BITS_NUM = 30;
constexpr size_t BIT_64 = 64;

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

[[noreturn]] PANDA_PUBLIC_API void InteropFatal(const char *message);
[[noreturn]] PANDA_PUBLIC_API void InteropFatal(const std::string &message);
[[noreturn]] PANDA_PUBLIC_API void InteropFatal(const char *message, napi_status status);

std::pair<SmallVector<uint64_t, 4U>, int> GetBigInt(napi_env env, napi_value jsVal);
SmallVector<uint64_t, 4U> ConvertBigIntArrayFromEtsToJs(const std::vector<uint32_t> &etsArray);
std::vector<EtsInt> ConvertBigIntArrayFromJsToEts(SmallVector<uint64_t, 4U> &jsArray, int signBit);

// Alternative for ASSERT(!expr) with interop stacktraces, enabled in NDEBUG
#define INTEROP_FATAL_IF(expr)                     \
    do {                                           \
        bool _expr = (expr);                       \
        if (UNLIKELY(_expr)) {                     \
            InteropFatal("INTEROP_FATAL: " #expr); \
            UNREACHABLE();                         \
        }                                          \
    } while (0)

#ifndef NDEBUG
void InteropTrace(const char *func, const char *file, int line);
#define INTEROP_TRACE()                             \
    do {                                            \
        InteropTrace(__func__, __FILE__, __LINE__); \
    } while (0)
#else
#define INTEROP_TRACE()
#endif

#if !defined(NDEBUG)
#define NAPI_ASSERT_OK(expr) INTEROP_FATAL_IF(expr)
#else
#define NAPI_ASSERT_OK(expr) \
    do {                     \
        (expr);              \
    } while (0)
#endif

// Assertion for napi_* calls success, enabled in NDEBUG
#define NAPI_CHECK_FATAL(status)                                 \
    do {                                                         \
        napi_status _status = (status);                          \
        if (UNLIKELY(_status != napi_ok)) {                      \
            InteropFatal("NAPI_CHECK_FATAL: " #status, _status); \
            UNREACHABLE();                                       \
        }                                                        \
    } while (0)

// NOLINTEND(cppcoreguidelines-macro-usage)

class NapiScope {
public:
    explicit NapiScope(napi_env env) : env_(env)
    {
        [[maybe_unused]] auto status = napi_open_handle_scope(env_, &scope_);
        ASSERT(status == napi_ok);
    }

    ~NapiScope()
    {
        [[maybe_unused]] auto status = napi_close_handle_scope(env_, scope_);
        ASSERT(status == napi_ok);
    }

    NO_COPY_SEMANTIC(NapiScope);
    NO_MOVE_SEMANTIC(NapiScope);

private:
    napi_env env_ {};
    napi_handle_scope scope_ {};
};

class NapiEscapableScope {
public:
    explicit NapiEscapableScope(napi_env env) : env_(env)
    {
        [[maybe_unused]] auto status = napi_open_escapable_handle_scope(env_, &scope_);
        ASSERT(status == napi_ok);
    }

    void Escape(napi_value &val)
    {
        [[maybe_unused]] auto status = napi_escape_handle(env_, scope_, val, &val);
        ASSERT(status == napi_ok);
    }

    ~NapiEscapableScope()
    {
        [[maybe_unused]] auto status = napi_close_escapable_handle_scope(env_, scope_);
        ASSERT(status == napi_ok);
    }

    NO_COPY_SEMANTIC(NapiEscapableScope);
    NO_MOVE_SEMANTIC(NapiEscapableScope);

private:
    napi_env env_ {};
    napi_escapable_handle_scope scope_ {};
};

inline napi_valuetype GetValueType(napi_env env, napi_value val)
{
    napi_valuetype vtype;
    NAPI_CHECK_FATAL(napi_typeof(env, val, &vtype));
    return vtype;
}

inline napi_value GetReferenceValue(napi_env env, napi_ref ref)
{
    napi_value val;
    NAPI_CHECK_FATAL(napi_get_reference_value(env, ref, &val));
    return val;
}

inline napi_value GetUndefined(napi_env env)
{
    napi_value jsValueUndefined {};
    NAPI_CHECK_FATAL(napi_get_undefined(env, &jsValueUndefined));
    return jsValueUndefined;
}

inline napi_value GetNull(napi_env env)
{
    napi_value jsValueNull {};
    NAPI_CHECK_FATAL(napi_get_null(env, &jsValueNull));
    return jsValueNull;
}

inline napi_value GetGlobal(napi_env env)
{
    napi_value jsValueGlobal {};
    NAPI_CHECK_FATAL(napi_get_global(env, &jsValueGlobal));
    return jsValueGlobal;
}

inline bool IsNull(napi_env env, napi_value val)
{
    return GetValueType(env, val) == napi_null;
}

inline bool IsUndefined(napi_env env, napi_value val)
{
    return GetValueType(env, val) == napi_undefined;
}

inline bool IsNullOrUndefined(napi_env env, napi_value val)
{
    napi_valuetype vtype = GetValueType(env, val);
    return vtype == napi_undefined || vtype == napi_null;
}

inline std::string GetString(napi_env env, napi_value jsVal)
{
    size_t length;
    NAPI_CHECK_FATAL(napi_get_value_string_utf8(env, jsVal, nullptr, 0, &length));
    std::string value;
    value.resize(length);
    // +1 for NULL terminated string!!!
    NAPI_CHECK_FATAL(napi_get_value_string_utf8(env, jsVal, value.data(), value.size() + 1, &length));
    return value;
}

inline int GeBigIntSign(std::vector<uint32_t> &array)
{
    int sign = 1;
    if (array.back() == 0) {
        sign = 0;
    }
    array.pop_back();

    return sign;
}

inline void DecrementBigInt(std::vector<uint32_t> &array)
{
    for (auto &elem : array) {
        if (elem != 0) {
            --elem;
            break;
        }

        elem = ~elem & BIGINT_BITMASK_30;
    }
}

inline void ConvertFromTwosComplement(std::vector<uint32_t> &array)
{
    DecrementBigInt(array);

    for (auto &elem : array) {
        elem = ~elem & BIGINT_BITMASK_30;
    }
}

inline bool NapiIsExceptionPending(napi_env env)
{
    bool pending;
    NAPI_CHECK_FATAL(napi_is_exception_pending(env, &pending));
    return pending;
}

inline bool NapiThrownGeneric(napi_status rc)
{
    INTEROP_FATAL_IF(rc != napi_ok && rc != napi_generic_failure);
    return rc == napi_generic_failure;
}

inline napi_status NapiObjectSeal([[maybe_unused]] napi_env env, [[maybe_unused]] napi_value jsVal)
{
// Ark js vm crashes in napi_object_seal.
// Disable the call temporary
#ifndef PANDA_TARGET_OHOS
    return napi_object_seal(env, jsVal);
#else
    return napi_ok;
#endif  // PANDA_TARGET_OHOS
}

inline napi_status NapiCallFunction(napi_env env, napi_value recv, napi_value func, size_t argc, const napi_value *argv,
                                    napi_value *result)
{
#ifdef PANDA_TARGET_OHOS
    napi_value dummy;
    result = result != nullptr ? result : &dummy;
#endif  // PANDA_TARGET_OHOS
    return napi_call_function(env, recv, func, argc, argv, result);
}

inline napi_status NapiWrap(napi_env env, napi_value jsObject, void *nativeObject, napi_finalize finalizeCb,
                            void *finalizeHint, napi_ref *result)
{
#ifdef PANDA_TARGET_OHOS
    napi_status status = napi_wrap(env, jsObject, nativeObject, finalizeCb, finalizeHint, nullptr);
    if (result == nullptr || UNLIKELY(status != napi_ok)) {
        return status;
    }
    return napi_create_reference(env, jsObject, 0, result);
#else
    ASSERT(result == nullptr || finalizeCb != nullptr);
    return napi_wrap(env, jsObject, nativeObject, finalizeCb, finalizeHint, result);
#endif
}

}  // namespace ark::ets::interop::js

#endif  // PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_INTEROP_COMMON_H_

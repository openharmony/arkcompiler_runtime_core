/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "concurrency_helpers.h"

#include <atomic>

namespace arkts::concurrency_helpers {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UNLIKELY(exp) (__builtin_expect((exp) != 0, false))

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECK_RETURN_IF_EQ(a, b, ret) \
    do {                              \
        if (UNLIKELY((a) == (b))) {   \
            return ret;               \
        }                             \
    } while (false)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECK_PTR_ARG(arg) CHECK_RETURN_IF_EQ(arg, nullptr, WorkStatus::INVALID_ARGS)

WorkStatus CreateAsyncWork(ani_env *env, ExecuteCallback execute, CompleteCallback complete,
                           [[maybe_unused]] void *data, AsyncWork **result)
{
    CHECK_PTR_ARG(env);
    CHECK_PTR_ARG(execute);
    CHECK_PTR_ARG(complete);
    CHECK_PTR_ARG(result);

    return WorkStatus::OK;
}

WorkStatus QueueAsyncWork(ani_env *env, AsyncWork *work)
{
    CHECK_PTR_ARG(env);
    CHECK_PTR_ARG(work);

    return WorkStatus::OK;
}

WorkStatus CancelAsyncWork(ani_env *env, AsyncWork *work)
{
    CHECK_PTR_ARG(env);
    CHECK_PTR_ARG(work);

    return WorkStatus::OK;
}

WorkStatus DeleteAsyncWork(ani_env *env, AsyncWork *work)
{
    CHECK_PTR_ARG(env);
    CHECK_PTR_ARG(work);

    return WorkStatus::OK;
}

}  // namespace arkts::concurrency_helpers

/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_NAPI_ETS_NAPI_MACROS_H
#define PANDA_PLUGINS_ETS_RUNTIME_NAPI_ETS_NAPI_MACROS_H

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

#define ETS_NAPI_ABORT_HOOK()                    \
    do {                                         \
        std::string msg("ETS_NAPI Abort: ");     \
        msg += __func__;                         \
        msg += ",";                              \
        msg += __FILE__;                         \
        msg += ":";                              \
        msg += std::to_string(__LINE__);         \
        ark::ets::PandaEtsVM::Abort(msg.data()); \
    } while (false)

#define ETS_NAPI_ABORT_IF(pred)    \
    do {                           \
        if (pred) {                \
            ETS_NAPI_ABORT_HOOK(); \
        }                          \
    } while (false)

#define ETS_NAPI_ABORT_IF_NULL(p)  \
    do {                           \
        if ((p) == nullptr) {      \
            ETS_NAPI_ABORT_HOOK(); \
        }                          \
    } while (false)

#define ETS_NAPI_ABORT_IF_LZ(p)    \
    do {                           \
        if ((p) < 0) {             \
            ETS_NAPI_ABORT_HOOK(); \
        }                          \
    } while (false)

#define ETS_NAPI_ABORT_IF_NE(a, b) \
    do {                           \
        if ((a) != (b)) {          \
            ETS_NAPI_ABORT_HOOK(); \
        }                          \
    } while (false)

#define ETS_NAPI_ABORT_IF_LT(a, b) \
    do {                           \
        if ((a) < (b)) {           \
            ETS_NAPI_ABORT_HOOK(); \
        }                          \
    } while (false)

#define ETS_NAPI_ABORT_IF_LE(a, b) \
    do {                           \
        if ((a) <= (b)) {          \
            ETS_NAPI_ABORT_HOOK(); \
        }                          \
    } while (false)

#define ETS_NAPI_ABORT_IF_GT(a, b) \
    do {                           \
        if ((a) > (b)) {           \
            ETS_NAPI_ABORT_HOOK(); \
        }                          \
    } while (false)

#define ETS_NAPI_ABORT_IF_GE(a, b) \
    do {                           \
        if ((a) >= (b)) {          \
            ETS_NAPI_ABORT_HOOK(); \
        }                          \
    } while (false)

#define ETS_NAPI_RETURN_VOID_IF_NULL(p) \
    do {                                \
        if ((p) == nullptr) {           \
            return;                     \
        }                               \
    } while (false)

#define ETS_NAPI_RETURN_IF_EQ(a, b, ret) \
    do {                                 \
        if (UNLIKELY((a) == (b))) {      \
            return ret;                  \
        }                                \
    } while (false)

#define ETS_NAPI_RETURN_IF_NE(a, b, ret) \
    do {                                 \
        if (UNLIKELY((a) != (b))) {      \
            return ret;                  \
        }                                \
    } while (false)

#define ETS_NAPI_RETURN_IF_GT(a, b, ret) \
    do {                                 \
        if (UNLIKELY((a) > (b))) {       \
            return ret;                  \
        }                                \
    } while (false)

#define ETS_NAPI_RETURN_IF_GE(a, b, ret) \
    do {                                 \
        if (UNLIKELY((a) >= (b))) {      \
            return ret;                  \
        }                                \
    } while (false)

#define ETS_NAPI_RETURN_IF_LT(a, b, ret) \
    do {                                 \
        if (UNLIKELY((a) < (b))) {       \
            return ret;                  \
        }                                \
    } while (false)

#define ETS_NAPI_RETURN_IF_LE(a, b, ret) \
    do {                                 \
        if (UNLIKELY((a) <= (b))) {      \
            return ret;                  \
        }                                \
    } while (false)

// NOLINTEND(cppcoreguidelines-macro-usage)

#endif  // PANDA_PLUGINS_ETS_RUNTIME_NAPI_ETS_NAPI_MACROS_H

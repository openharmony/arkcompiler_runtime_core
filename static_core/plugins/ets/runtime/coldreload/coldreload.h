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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_COLDRELOAD_COLDRELOAD_H_
#define PANDA_PLUGINS_ETS_RUNTIME_COLDRELOAD_COLDRELOAD_H_

#include "libarkbase/macros.h"

namespace ark::ets::coldreload {

/**
 * @brief Cold reload: prepend a patch abc to a module's class search order.
 *
 * Runs at application startup, before any user code, after a process restart:
 * there is no live state to preserve. The patch is inserted at the front of
 * the search order, so a class present in the patch is loaded from the patch,
 * and a class absent from it falls through to the original files.
 */

/**
 * @brief Error codes for cold reload.
 *
 * The ordinals are part of the API surface (the callers pass them through as `int`), so values
 * may only be APPENDED, never inserted or reordered. `GetErrorString` keeps a name table in
 * ordinal order; a `static_assert` pins its length to the last value, so a value added without
 * its name (or the other way round) fails the build.
 */
enum class Error {
    NONE,
    INVALID_INPUT_ARG,       ///< patch path is empty, or the receiver is null
    PATCH_OPEN_FAILED,       ///< patch file cannot be opened or is malformed
    AOT_LOADED,              ///< AOT files are loaded: compiled code may embed the old classes
    CLASSES_ALREADY_LOADED,  ///< the receiver's context already has loaded classes (not startup)
    INTERNAL,                ///< unexpected internal failure
};

/** @returns the human-readable name of @a err, for logging only. */
PANDA_PUBLIC_API const char *GetErrorString(Error err);

}  // namespace ark::ets::coldreload

#endif  // PANDA_PLUGINS_ETS_RUNTIME_COLDRELOAD_COLDRELOAD_H_

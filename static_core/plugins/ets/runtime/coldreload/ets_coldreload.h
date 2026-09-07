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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_COLDRELOAD_ETS_COLDRELOAD_H_
#define PANDA_PLUGINS_ETS_RUNTIME_COLDRELOAD_ETS_COLDRELOAD_H_

#include <memory>

#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/coldreload/coldreload.h"
#include "runtime/include/managed_thread.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/include/thread_scopes.h"

namespace ark::ets {
class EtsAbcRuntimeLinker;
}  // namespace ark::ets

namespace ark::ets::coldreload {

/**
 * @brief Drives the cold reload transaction for one patch.
 *
 * Reads the patch (a plain abc or a package entry), validates the startup contract, and
 * prepends the patch to the AbcRuntimeLinker's abcFiles array, so that subsequent class
 * lookups find patch classes first. The object itself carries no thread or coroutine state
 * and may be freely passed around; the executing coroutine is given per call.
 */
class EtsColdReload final {
public:
    EtsColdReload() = default;

    NO_COPY_SEMANTIC(EtsColdReload);
    NO_MOVE_SEMANTIC(EtsColdReload);
    ~EtsColdReload() = default;

    /**
     * @brief Prepend the patch @a patchPath to @a runtimeLinker's search order.
     *
     * Must be called from managed state on @a coro; the file read happens
     * inside a native scope. Never throws: all failures come back as Error values.
     *
     * @param coro the coroutine executing the reload.
     * @param patchPath path of the plain .abc patch file.
     * @param runtimeLinker the AbcRuntimeLinker receiving the patch.
     * @returns Error::NONE on success.
     */
    Error Reload(EtsCoroutine *coro, const PandaString &patchPath, EtsAbcRuntimeLinker *runtimeLinker);

private:
    // Owns the patch panda file until it is wrapped in a managed EtsAbcFile;
    // on the error paths it releases the file here.
    std::unique_ptr<const panda_file::File> patchFile_;
};

}  // namespace ark::ets::coldreload

#endif  // PANDA_PLUGINS_ETS_RUNTIME_COLDRELOAD_ETS_COLDRELOAD_H_

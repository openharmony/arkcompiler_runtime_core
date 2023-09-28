/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "file_ext.h"
#include "panda_file_external.h"
#include "os/mutex.h"
#include <dlfcn.h>
#include "libpandafile/class_data_accessor-inl.h"
#include "libpandafile/code_data_accessor-inl.h"

namespace panda_api::panda_file {

decltype(OpenPandafileFromFdExt) *PandaFileWrapper::pOpenPandafileFromFdExt = nullptr;
decltype(OpenPandafileFromMemoryExt) *PandaFileWrapper::pOpenPandafileFromMemoryExt = nullptr;
decltype(QueryMethodSymByOffsetExt) *PandaFileWrapper::pQueryMethodSymByOffsetExt = nullptr;
decltype(QueryMethodSymAndLineByOffsetExt) *PandaFileWrapper::pQueryMethodSymAndLineByOffsetExt = nullptr;
decltype(QueryAllMethodSymsExt) *PandaFileWrapper::pQueryAllMethodSymsExt = nullptr;

using OnceCallback = void();
static void CallOnce(bool *once_flag, OnceCallback call_back)
{
    static panda::os::memory::Mutex once_lock;
    panda::os::memory::LockHolder lock(once_lock);
    if (!(*once_flag)) {
        call_back();
        *once_flag = true;
    }
}

void LoadPandFileExt()
{
    static bool dlopen_once = false;
    CallOnce(&dlopen_once, []() {
        const char pandafileext[] = "libpandafileExt.so";
        void *hd = dlopen(pandafileext, RTLD_NOW);
        if (hd == nullptr) {
            return;
        }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOAD_FUNC(CLASS, FUNC)                                                 \
    do {                                                                       \
        CLASS::p##FUNC = reinterpret_cast<decltype(FUNC) *>(dlsym(hd, #FUNC)); \
        if (CLASS::p##FUNC == nullptr) {                                       \
            return;                                                            \
        }                                                                      \
    } while (0)

        LOAD_FUNC(PandaFileWrapper, OpenPandafileFromFdExt);
        LOAD_FUNC(PandaFileWrapper, OpenPandafileFromMemoryExt);
        LOAD_FUNC(PandaFileWrapper, QueryMethodSymByOffsetExt);
        LOAD_FUNC(PandaFileWrapper, QueryMethodSymAndLineByOffsetExt);
        LOAD_FUNC(PandaFileWrapper, QueryAllMethodSymsExt);
#undef LOAD_FUNC
    });
}

}  // namespace panda_api::panda_file

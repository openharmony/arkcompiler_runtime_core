/*
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

#include "runtime/include/loadable_agent.h"

#include "runtime/include/runtime.h"

namespace panda {
LibraryAgent::LibraryAgent(os::memory::Mutex &mutex, PandaString libraryPath, PandaString loadCallbackName,
                           PandaString unloadCallbackName)
    : lock_(mutex),
      library_path_(std::move(libraryPath)),
      load_callback_name_(std::move(loadCallbackName)),
      unload_callback_name_(std::move(unloadCallbackName))
{
}

bool LibraryAgent::Load()
{
    ASSERT(!handle_.IsValid());

    auto handle = os::library_loader::Load(library_path_);
    if (!handle) {
        LOG(ERROR, RUNTIME) << "Couldn't load library '" << library_path_ << "': " << handle.Error().ToString();
        return false;
    }

    auto load_callback = os::library_loader::ResolveSymbol(handle.Value(), load_callback_name_);
    if (!load_callback) {
        LOG(ERROR, RUNTIME) << "Couldn't resolve '" << load_callback_name_ << "' in '" << library_path_
                            << "':" << load_callback.Error().ToString();
        return false;
    }

    auto unload_callback = os::library_loader::ResolveSymbol(handle.Value(), unload_callback_name_);
    if (!unload_callback) {
        LOG(ERROR, RUNTIME) << "Couldn't resolve '" << unload_callback_name_ << "' in '" << library_path_
                            << "':" << unload_callback.Error().ToString();
        return false;
    }

    if (!CallLoadCallback(load_callback.Value())) {
        LOG(ERROR, RUNTIME) << "'" << load_callback_name_ << "' failed in '" << library_path_ << "'";
        return false;
    }

    handle_ = std::move(handle.Value());
    unload_callback_ = unload_callback.Value();

    return true;
}

bool LibraryAgent::Unload()
{
    ASSERT(handle_.IsValid());

    if (!CallUnloadCallback(unload_callback_)) {
        LOG(ERROR, RUNTIME) << "'" << unload_callback_name_ << "' failed in '" << library_path_ << "'";
        return false;
    }

    return true;
}
}  // namespace panda

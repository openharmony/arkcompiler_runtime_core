/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_ETS_RUNTIME_ETS_REMOTE_PROMISE_RESOLVER_H
#define PLUGINS_ETS_RUNTIME_ETS_REMOTE_PROMISE_RESOLVER_H

#include "plugins/ets/runtime/types/ets_object.h"

namespace ark::ets {

/// @brief interface of resolving/rejection of promise from remote side
class RemotePromiseResolver {
public:
    enum class Action : uint8_t { RESOLVE, REJECT };

    RemotePromiseResolver() = default;
    NO_COPY_SEMANTIC(RemotePromiseResolver);
    NO_MOVE_SEMANTIC(RemotePromiseResolver);
    virtual ~RemotePromiseResolver() = default;

    /**
     * @brief method should post to remote side callback to resolve/reject promise
     * @param resolveValue: EtsObject to resolve/reject promise. If you want to reject promise, it should be exception
     * with error description. If you want to resolve promise, you can use any object.
     * @param action: use Action::Resolve if you want to resolve remote promise, otherwise use Action::Reject
     */
    virtual void ResolveViaCallback(EtsObject *resolveValue, Action action) = 0;

    /**
     * @brief method should resolve/reject promise in current thread in please. Implement this method only if it's
     * needed.
     * @param resolveValue: EtsObject to resolve/reject promise. If you want to reject promise, it should be exception
     * with error description. If you want to resolve promise, you can use any object.
     * @param action: use Action::Resolve if you want to resolve remote promise, otherwise use Action::Reject
     */
    virtual void ResolveInPlace(EtsObject *resolveValue, Action action) = 0;
};

}  // namespace ark::ets

#endif  // PLUGINS_ETS_RUNTIME_ETS_REMOTE_PROMISE_RESOLVER_H
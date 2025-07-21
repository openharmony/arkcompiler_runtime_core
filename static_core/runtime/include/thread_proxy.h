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
#ifndef PANDA_RUNTIME_THREAD_PROXY_H
#define PANDA_RUNTIME_THREAD_PROXY_H

#include "runtime/include/thread_proxy_static.h"
#include "runtime/include/thread_proxy_hybrid.h"

namespace ark {

class MutatorLock;

#ifndef ARK_HYBRID
class ThreadProxy : public ThreadProxyStatic {
public:
    explicit ThreadProxy(MutatorLock *mutatorLock) : ThreadProxyStatic(mutatorLock) {}
};
#else
class ThreadProxy : public ThreadProxyHybrid {
public:
    explicit ThreadProxy(MutatorLock *mutatorLock) : ThreadProxyHybrid(mutatorLock) {}
};
#endif

}  // namespace ark

#endif  // PANDA_RUNTIME_THREAD_PROXY_H

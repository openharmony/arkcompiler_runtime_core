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

#ifndef PANDA_RUNTIME_GLOBAL_OBJECT_LOCK_H
#define PANDA_RUNTIME_GLOBAL_OBJECT_LOCK_H

#include "runtime/include/object_header.h"

namespace panda {
class GlobalObjectLock {
public:
    explicit GlobalObjectLock(const ObjectHeader *obj);

    bool Wait(bool ignoreInterruption = false) const;

    bool TimedWait(uint64_t timeout) const;

    void Notify();

    void NotifyAll();

    ~GlobalObjectLock();

    NO_COPY_SEMANTIC(GlobalObjectLock);
    NO_MOVE_SEMANTIC(GlobalObjectLock);
};
}  // namespace panda

#endif  // PANDA_RUNTIME_GLOBAL_OBJECT_LOCK_H

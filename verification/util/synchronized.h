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

#ifndef PANDA_VERIFIER_SYNCHRONIZED_HPP_
#define PANDA_VERIFIER_SYNCHRONIZED_HPP_

#include "verification/util/callable.h"

#include "libpandabase/os/mutex.h"

#include "macros.h"

#include <utility>

namespace panda::verifier {
template <class C, class Friend1 = C, class Friend2 = C>
class Synchronized {
    struct ConstProxy {
        ConstProxy() = delete;
        ConstProxy(const ConstProxy &) = delete;
        ConstProxy(ConstProxy &&other)
        {
            obj = other.obj;
            other.obj = nullptr;
        }
        const Synchronized *obj;
        ConstProxy(const Synchronized *param_obj) ACQUIRE_SHARED(obj->rw_lock_) : obj {param_obj}
        {
            obj->rw_lock_.ReadLock();
        }
        const C *operator->() const
        {
            ASSERT(obj != nullptr);
            return &obj->c;
        }
        ~ConstProxy() RELEASE_SHARED(obj->rw_lock_)
        {
            if (obj != nullptr) {
                obj->rw_lock_.Unlock();
            }
        }
    };

    struct Proxy {
        Proxy() = delete;
        Proxy(const Proxy &) = delete;
        Proxy(Proxy &&other)
        {
            obj = other.obj;
            other.obj = nullptr;
        }
        Synchronized *obj;
        Proxy(Synchronized *param_obj) ACQUIRE(obj->rw_lock_) : obj {param_obj}
        {
            obj->rw_lock_.WriteLock();
        }
        C *operator->()
        {
            ASSERT(obj != nullptr);
            return &obj->c;
        }
        ~Proxy() RELEASE(obj->rw_lock_)
        {
            if (obj != nullptr) {
                obj->rw_lock_.Unlock();
            }
        }
    };

    C c;

    // GetObj() should be ideally annotated with REQUIRES/REQUIRES_SHARED and c with GUARDED_BY, but then the current
    // design of LibCache::FastAPI can't be annotated correctly
    C &GetObj()
    {
        return c;
    }

    const C &GetObj() const
    {
        return c;
    }

    void WriteLock() ACQUIRE(rw_lock_)
    {
        rw_lock_.WriteLock();
    }

    void ReadLock() ACQUIRE_SHARED(rw_lock_)
    {
        rw_lock_.ReadLock();
    }

    void Unlock() RELEASE_GENERIC(rw_lock_)
    {
        rw_lock_.Unlock();
    }

    friend Friend1;
    friend Friend2;

public:
    template <typename... Args>
    Synchronized(Args &&... args) : c(std::forward<Args>(args)...)
    {
    }

    ConstProxy operator->() const
    {
        return {this};
    }

    Proxy operator->()
    {
        return {this};
    }

    template <typename Handler>
    auto Apply(Handler &&handler)
    {
        os::memory::WriteLockHolder lock_holder {rw_lock_};
        return handler(c);
    }

    template <typename Handler>
    auto Apply(Handler &&handler) const
    {
        os::memory::ReadLockHolder lock_holder {rw_lock_};
        return handler(c);
    }

private:
    mutable panda::os::memory::RWLock rw_lock_;
};
}  // namespace panda::verifier

#endif  // !PANDA_VERIFIER_SYNCHRONIZED_HPP_

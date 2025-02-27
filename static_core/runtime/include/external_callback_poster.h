/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ETS_EXTERNAL_CALLBACK_POSTER_H
#define PANDA_PLUGINS_ETS_RUNTIME_ETS_EXTERNAL_CALLBACK_POSTER_H

#include <functional>

#include "libpandabase/macros.h"
#include "runtime/include/mem/panda_smart_pointers.h"

namespace ark {

class Coroutine;

/// @brief Interface of class that should post a callback to remote side
class CallbackPoster {
public:
    CallbackPoster() = default;
    virtual ~CallbackPoster() = default;
    NO_COPY_SEMANTIC(CallbackPoster);
    NO_MOVE_SEMANTIC(CallbackPoster);

    template <class Callback, class... Args>
    void Post(Callback callback, Args... args)
    {
        static_assert(std::is_invocable_v<Callback, Args...>);
        static_assert(std::is_void_v<std::result_of_t<Callback(Args...)>>);
        PostImpl([callback = std::move(callback), targs = std::tuple<Args...>(std::move(args)...)]() {
            std::apply(std::move(callback), std::move(targs));
        });
    }

    void SetDestroyInPlace()
    {
        destroyInPlace_ = true;
    }

    bool NeedDestroyInPlace() const
    {
        return destroyInPlace_;
    }

protected:
    using WrappedCallback = std::function<void()>;

    virtual void PostImpl(WrappedCallback &&callback) = 0;

private:
    bool destroyInPlace_ = false;
};

class CallbackPosterFactoryIface {
public:
    CallbackPosterFactoryIface() = default;
    virtual ~CallbackPosterFactoryIface() = default;
    NO_COPY_SEMANTIC(CallbackPosterFactoryIface);
    NO_MOVE_SEMANTIC(CallbackPosterFactoryIface);

    virtual PandaUniquePtr<CallbackPoster> CreatePoster() = 0;
};

}  // namespace ark

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ETS_EXTERNAL_CALLBACK_POSTER_H
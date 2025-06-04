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

#include "ani_gtest.h"
#include "runtime/coroutines/coroutine.h"
#include "runtime/include/runtime.h"

namespace ark::ets::test {

class Event {
public:
    void Wait()
    {
        os::memory::LockHolder lh(lock_);
        while (!happened_) {
            cv_.Wait(&lock_);
        }
    }

    void Fire()
    {
        os::memory::LockHolder lh(lock_);
        happened_ = true;
        cv_.SignalAll();
    }

private:
    bool happened_ = false;
    os::memory::Mutex lock_;
    os::memory::ConditionVariable cv_;
};

class EtsNativeExclusiveWorkerTest : public ani::testing::AniTest {
public:
    template <typename... Args>
    void RunRoutineInExclusiveWorker(std::string_view routineName, std::string_view routineSignature, Args &&...args)
    {
        auto event = Event();

        std::thread worker([this, &event, routineName, routineSignature,
                            args = std::make_tuple(std::forward<Args>(args)...)]() mutable {
            WorkerRoutine(event, routineName, routineSignature, std::forward<Args>(args)...);
        });

        event.Wait();
        worker.detach();
    }

    template <typename... Args>
    static bool CallStaticBooleanMethod(ani_env *env, std::string_view methodName, std::string_view signature,
                                        Args &&...args)
    {
        auto func = ResolveFunction(env, methodName, signature);
        ani_boolean result;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        env->Function_Call_Boolean(func, &result, std::forward<Args>(args)...);
        return result;
    }

    template <typename... Args>
    static void CallStaticVoidMethod(ani_env *env, std::string_view methodName, std::string_view signature,
                                     Args &&...args)
    {
        auto func = ResolveFunction(env, methodName, signature);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        env->Function_Call_Void(func, std::forward<Args>(args)...);
    }

    std::vector<ani_option> GetExtraAniOptions() override
    {
        return {
            ani_option {"--gc-type=g1-gc", nullptr},
            ani_option {"--compiler-enable-jit", nullptr},
        };
    }

private:
    template <typename... Args>
    void WorkerRoutine(Event &event, std::string_view routineName, std::string_view routineSignature, Args &&...args)
    {
        ASSERT(Thread::GetCurrent() == nullptr);
        [[maybe_unused]] ani_env *workerEnv = nullptr;
        ani_options aniArgs {0, nullptr};
        [[maybe_unused]] auto status = vm_->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &workerEnv);
        ASSERT(status == ANI_OK);

        event.Fire();

        ani_int eWorkerId = os::thread::GetCurrentThreadId();
        CallStaticVoidMethod(workerEnv, "setWorkerId", "I:V", eWorkerId);

        ani_boolean res {};
        res = CallStaticBooleanMethod(workerEnv, routineName, routineSignature, std::forward<Args>(args)...);
        ASSERT_EQ(res, true);

        status = vm_->DetachCurrentThread();
        ASSERT(status == ANI_OK);
    }

    static ani_function ResolveFunction(ani_env *env, std::string_view methodName, std::string_view signature)
    {
        ani_module md;
        [[maybe_unused]] auto status = env->FindModule("Lexclusive_worker_tests;", &md);
        ASSERT(status == ANI_OK);
        ani_function func;
        status = env->Module_FindFunction(md, methodName.data(), signature.data(), &func);
        ASSERT(status == ANI_OK);
        return func;
    }
};

TEST_F(EtsNativeExclusiveWorkerTest, CallMethod)
{
    RunRoutineInExclusiveWorker("call", ":Z");
}

TEST_F(EtsNativeExclusiveWorkerTest, AsyncCallMethod)
{
    RunRoutineInExclusiveWorker("asyncCall", ":Z");
}

TEST_F(EtsNativeExclusiveWorkerTest, LaunchCallMethod)
{
    RunRoutineInExclusiveWorker("launchCall", ":Z");
}

TEST_F(EtsNativeExclusiveWorkerTest, ConcurrentWorkerAndRuntimeDestroy)
{
    RunRoutineInExclusiveWorker("eWorkerRoutine", ":Z");
    CallStaticVoidMethod(env_, "mainRoutine", ":V");
}

}  // namespace ark::ets::test

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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "runtime/coroutines/coroutine.h"
#include "runtime/include/runtime.h"

#include "plugins/ets/runtime/napi/ets_napi.h"
#include "plugins/ets/runtime/types/ets_method.h"

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

class EtsNativeExclusiveWorkerTest : public testing::Test {
public:
    template <typename... Args>
    void RunRoutineInExclusiveWorker(std::string_view routineName, Args &&...args)
    {
        auto event = Event();

        std::thread worker([this, &event, routineName, args = std::make_tuple(std::forward<Args>(args)...)]() mutable {
            WorkerRoutine(event, routineName, std::forward<Args>(args)...);
        });

        event.Wait();
        worker.detach();
    }

    template <typename... Args>
    static bool CallStaticBooleanMethod(std::string_view methodName, Args &&...args)
    {
        auto *env = PandaEtsNapiEnv::GetCurrent();
        auto [cls, fn] = ResolveMethod(env, methodName);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        return env->CallStaticBooleanMethod(cls, fn, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void CallStaticVoidMethod(std::string_view methodName, Args &&...args)
    {
        auto *env = PandaEtsNapiEnv::GetCurrent();
        auto [cls, fn] = ResolveMethod(env, methodName);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        env->CallStaticVoidMethod(cls, fn, std::forward<Args>(args)...);
    }

protected:
    void SetUp() override
    {
        const char *stdlib = std::getenv("ARK_ETS_STDLIB_PATH");
        ASSERT_NE(stdlib, nullptr);

        std::vector<EtsVMOption> optionsVector {{EtsOptionType::ETS_GC_TYPE, "g1-gc"},
                                                {EtsOptionType::ETS_JIT, nullptr},
                                                {EtsOptionType::ETS_BOOT_FILE, stdlib}};

        auto abcPath = std::getenv("ARK_ETS_GTEST_ABC_PATH");
        if (abcPath != nullptr) {
            optionsVector.push_back({EtsOptionType::ETS_BOOT_FILE, abcPath});
        }

        EtsVMInitArgs vmArgs;
        vmArgs.version = ETS_NAPI_VERSION_1_0;
        vmArgs.options = optionsVector.data();
        vmArgs.nOptions = static_cast<ets_int>(optionsVector.size());

        ASSERT_TRUE(ETS_CreateVM(&vm_, &env_, &vmArgs) == ETS_OK) << "Cannot create ETS VM";
    }

    void TearDown() override
    {
        ASSERT_TRUE(vm_->DestroyEtsVM() == ETS_OK) << "Cannot destroy ETS VM";
    }

private:
    template <typename... Args>
    void WorkerRoutine(Event &event, std::string_view routineName, Args &&...args)
    {
        ASSERT(Thread::GetCurrent() == nullptr);
        [[maybe_unused]] EtsEnv *workerEnv = nullptr;
        [[maybe_unused]] void *resultJsEnv = nullptr;
        [[maybe_unused]] auto status = vm_->AttachThread(&workerEnv, &resultJsEnv);
        ASSERT(status == ETS_OK);

        event.Fire();

        ets_int eWorkerId = os::thread::GetCurrentThreadId();
        CallStaticVoidMethod("setWorkerId", eWorkerId);

        ets_boolean res {};
        res = CallStaticBooleanMethod(routineName, std::forward<Args>(args)...);
        ASSERT_EQ(res, true);

        status = vm_->DetachThread();
        ASSERT(status == ETS_OK);
    }

    static std::pair<ets_class, ets_method> ResolveMethod(EtsEnv *env, std::string_view methodName)
    {
        ets_class cls = env->FindClass("ETSGLOBAL");
        ASSERT(cls != nullptr);
        ets_method fn = env->GetStaticp_method(cls, methodName.data(), nullptr);
        return {cls, fn};
    }

    EtsVM *vm_ {nullptr};
    EtsEnv *env_ {nullptr};
};

TEST_F(EtsNativeExclusiveWorkerTest, CallMethod)
{
    RunRoutineInExclusiveWorker("call");
}

TEST_F(EtsNativeExclusiveWorkerTest, AsyncCallMethod)
{
    RunRoutineInExclusiveWorker("asyncCall");
}

TEST_F(EtsNativeExclusiveWorkerTest, LaunchCallMethod)
{
    RunRoutineInExclusiveWorker("launchCall");
}

TEST_F(EtsNativeExclusiveWorkerTest, ConcurrentWorkerAndRuntimeDestroy)
{
    RunRoutineInExclusiveWorker("eWorkerRoutine");
    CallStaticVoidMethod("mainRoutine");
}

}  // namespace ark::ets::test

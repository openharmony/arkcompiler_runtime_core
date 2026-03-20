/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include "ets_coroutine.h"
#include "types/ets_string.h"
#include "intrinsics/helpers/ets_to_string_cache.h"
#include "intrinsics/helpers/ets_intrinsics_helpers.h"
#include "ets_vm.h"
#include "gtest/gtest.h"
#include "runtime/include/runtime.h"
#include "plugins/ets/tests/runtime/types/ets_test_mirror_classes.h"

#include <array>
#include <thread>
#include <random>
#include <sstream>

#include "plugins/ets/runtime/intrinsics/helpers/ets_to_string_cache.cpp"

namespace ark::ets::test {

static constexpr uint32_t TEST_ARRAY_SIZE = 1000;

enum GenType { COPY, SHUFFLE, INDEPENDENT };

class EtsToStringCacheTest : public testing::Test {
public:
    explicit EtsToStringCacheTest(const char *gcType = nullptr, bool cacheEnabled = true)
        : engine_(std::random_device {}())
    {
        // Logger::InitializeStdLogging(Logger::Level::INFO, Logger::ComponentMaskFromString("runtime") |
        // Logger::ComponentMaskFromString("coroutines"));

        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(true);
        options.SetShouldInitializeIntrinsics(true);
        options.SetLoadRuntimes({"ets"});
        options.SetUseStringCaches(cacheEnabled);

        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to etsstdlib.abc" << std::endl;
            std::abort();
        }
        options.SetBootPandaFiles({stdlib});

        if (gcType != nullptr) {
            options.SetGcType(gcType);
        }
        options.SetCompilerEnableJit(false);
        if (!Runtime::Create(options)) {
            UNREACHABLE();
        }
    }

    ~EtsToStringCacheTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EtsToStringCacheTest);
    NO_MOVE_SEMANTIC(EtsToStringCacheTest);

    void SetUp() override
    {
        ASSERT(Runtime::GetCurrent() != nullptr);
        ASSERT(PandaEtsVM::GetCurrent() != nullptr);
        mainCoro_ = EtsCoroutine::CastFromThread(PandaEtsVM::GetCurrent()->GetCoroutineManager()->GetMainThread());
        ASSERT(mainCoro_ != nullptr);
    }
    void TestMainLoop(double value, [[maybe_unused]] bool needCheck)
    {
        auto *thread = ManagedThread::GetCurrent();
        auto *cache = DoubleToStringCache::FromCoreType(thread->GetDoubleToStringCache());
        auto *etsCoro = EtsCoroutine::GetCurrent();
        [[maybe_unused]] auto [str, result] = cache->GetOrCacheImpl(etsCoro, value);
#ifndef NDEBUG
        // don't always check to increase pressure
        if (needCheck) {
            ASSERT(!str->IsUtf16());
            auto res = str->GetMutf8();

            intrinsics::helpers::FpToStringDecimalRadix(value,
                                                        [&res](std::string_view expected) { ASSERT(expected == res); });

            auto resValue = PandaStringToD(res);
            auto eps = std::numeric_limits<double>::epsilon() * 2 * std::abs(value);
            ASSERT(std::abs(resValue - value) < eps);
        }
#endif
    }

    EtsCoroutine *GetMainCoro()
    {
        return mainCoro_;
    }

    std::mt19937 &GetEngine()
    {
        return engine_;
    }

    template <typename T>
    static void CheckCacheElementMembers()
    {
        // We can call "classLinker->GetClass" only with MutatorLock or with disabled GC.
        // So just for testing is necessary add "MutatorLock"
        // NOTE: In the main place of use (in initialization VM), during execution method
        // "EtsToStringCacheElement<T>::GetClass" GC is not started
        PandaVM::GetCurrent()->GetMutatorLock()->WriteLock();
        auto *klass = detail::EtsToStringCacheElement<T>::GetClass(EtsCoroutine::GetCurrent());
        std::vector<MirrorFieldInfo> members {
            MirrorFieldInfo("string", detail::EtsToStringCacheElement<T>::GetStringOffset()),
            MirrorFieldInfo("number", detail::EtsToStringCacheElement<T>::GetNumberOffset())};
        MirrorFieldInfo::CompareMemberOffsets(klass, members);
        PandaVM::GetCurrent()->GetMutatorLock()->Unlock();
    }

private:
    std::mt19937 engine_;
    EtsCoroutine *mainCoro_ {};
};

TEST_F(EtsToStringCacheTest, BitcastTestCached)
{
    auto &engine = GetEngine();
    auto coro = GetMainCoro();
    auto *thread = ManagedThread::GetCurrent();
    std::uniform_int_distribution<uint32_t> dis(0, std::numeric_limits<uint32_t>::max());
    coro->ManagedCodeBegin();
    auto etsCoro = EtsCoroutine::GetCurrent();
    ASSERT(coro == etsCoro);
    auto *cache = DoubleToStringCache::FromCoreType(thread->GetDoubleToStringCache());

    for (uint32_t i = 0; i < TEST_ARRAY_SIZE; i++) {
        auto longValue = (static_cast<uint64_t>(dis(engine)) << BITS_PER_UINT32) | dis(engine);
        auto value = bit_cast<double>(longValue);
        auto *str = cache->GetOrCache(etsCoro, value);
        ASSERT(!str->IsUtf16());
        auto res = str->GetMutf8();

        bool correct;
        auto eps = std::numeric_limits<double>::epsilon() * std::abs(value);
        double resValue = 0;
        if (std::isnan(value)) {
            correct = res == "NaN";
        } else {
            std::istringstream iss {std::string(res)};
            iss >> resValue;
            correct = std::abs(resValue - value) <= eps;
        }

        if (!correct) {
            std::cerr << std::setprecision(intrinsics::helpers::DOUBLE_MAX_PRECISION) << "Error:\n"
                      << "long: " << std::hex << longValue << "\n"
                      << "double: " << value << "\n"
                      << "str: " << res << "\n"
                      << "delta: " << std::abs(resValue - value) << "\n"
                      << "eps: " << eps << std::endl;
            UNREACHABLE();
        }
    }

    coro->ManagedCodeEnd();
}

TEST_F(EtsToStringCacheTest, BitcastTestRaw)
{
    auto &engine = GetEngine();
    std::uniform_int_distribution<uint32_t> dis(0, std::numeric_limits<uint32_t>::max());

    for (uint32_t i = 0; i < TEST_ARRAY_SIZE; i++) {
        auto longValue = (static_cast<uint64_t>(dis(engine)) << BITS_PER_UINT32) | dis(engine);
        auto value = bit_cast<double>(longValue);
        std::string res;
        intrinsics::helpers::FpToStringDecimalRadix(value, [&res](std::string_view expected) { res = expected; });

        bool correct;
        auto eps = std::numeric_limits<double>::epsilon() * std::abs(value);
        double resValue = 0;
        if (std::isnan(value)) {
            correct = res == "NaN";
        } else {
            std::istringstream iss {std::string(res)};
            iss >> resValue;
            correct = std::abs(resValue - value) <= eps;
        }

        if (!correct) {
            std::cerr << std::setprecision(intrinsics::helpers::DOUBLE_MAX_PRECISION) << "Error:\n"
                      << "long: " << std::hex << longValue << "\n"
                      << "double: " << value << "\n"
                      << "str: " << res << "\n"
                      << "delta: " << std::abs(resValue - value) << "\n"
                      << "eps: " << eps << std::endl;
            UNREACHABLE();
        }
    }
}

[[maybe_unused]] static bool SymEq(float x, float y)
{
    if (std::isnan(x)) {
        return std::isnan(y);
    }
    auto delta = std::abs(x - y);
    auto eps = std::abs(x) * (2 * std::numeric_limits<float>::epsilon());
    return delta <= eps;
}

TEST_F(EtsToStringCacheTest, BitcastTestFloat)
{
    auto &engine = GetEngine();
    std::uniform_int_distribution<uint32_t> dis(0, std::numeric_limits<uint32_t>::max());

    for (uint32_t i = 0; i < TEST_ARRAY_SIZE; i++) {
        auto intValue = dis(engine);
        auto value = bit_cast<float>(intValue);
        if (std::isnan(value)) {
            continue;
        }
        {
            std::string resFloat;
            intrinsics::helpers::FpToStringDecimalRadix(value, [&resFloat](std::string_view str) { resFloat = str; });
            float parsedFromFloat = 0;
            std::istringstream iss {resFloat};
            iss >> parsedFromFloat;
            ASSERT_DO(SymEq(value, parsedFromFloat), std::cerr << "Error:\n"
                                                               << "float value: " << value << "\n"
                                                               << "float str: " << resFloat << "\n"
                                                               << "float parsed: " << parsedFromFloat << "\n");
        }
        {
            auto doubleValue = static_cast<double>(value);
            std::string resDouble;
            intrinsics::helpers::FpToStringDecimalRadix(doubleValue,
                                                        [&resDouble](std::string_view str) { resDouble = str; });
            float parsedFromDouble = 0;
            std::istringstream iss {resDouble};
            iss >> parsedFromDouble;
            ASSERT(SymEq(value, parsedFromDouble));
            ASSERT_DO(SymEq(value, parsedFromDouble), std::cerr << "Error:\n"
                                                                << "float value: " << value << "\n"
                                                                << "double str: " << resDouble << "\n"
                                                                << "float parsed: " << parsedFromDouble << "\n");
        }
    }
}

TEST_F(EtsToStringCacheTest, ToStringCacheElementLayout)
{
    CheckCacheElementMembers<EtsDouble>();
    CheckCacheElementMembers<EtsFloat>();
    CheckCacheElementMembers<EtsLong>();
}

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class EtsToStringCacheCLITest : public EtsToStringCacheTest, public testing::WithParamInterface<bool> {
public:
    EtsToStringCacheCLITest() : EtsToStringCacheTest(nullptr, GetParam()) {}
};

TEST_P(EtsToStringCacheCLITest, TestCacheUsage)
{
    auto *thread = ManagedThread::GetCurrent();
    auto *doubleCache = DoubleToStringCache::FromCoreType(thread->GetDoubleToStringCache());
    auto *floatCache = FloatToStringCache::FromCoreType(thread->GetFloatToStringCache());
    auto *longCache = LongToStringCache::FromCoreType(thread->GetLongToStringCache());
    if (!GetParam()) {  // cache disabled
        ASSERT_EQ(doubleCache, nullptr);
        ASSERT_EQ(floatCache, nullptr);
        ASSERT_EQ(longCache, nullptr);
    } else {  // cache enabled
        ASSERT_NE(doubleCache, nullptr);
        ASSERT_NE(floatCache, nullptr);
        ASSERT_NE(longCache, nullptr);
    }
}

INSTANTIATE_TEST_SUITE_P(EtsToStringCacheCLITestSuite, EtsToStringCacheCLITest, testing::Values(true, false));

}  // namespace ark::ets::test

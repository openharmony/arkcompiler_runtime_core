#include <gtest/gtest.h>

#include "plugins/ets/tests/mock/calling_methods_test_helper.h"

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)

namespace panda::ets::test {

static const char *TEST_BIN_FILE_NAME = "RegisterNativeAllocationTest.abc";

class CallingMethodsTestGeneral : public CallingMethodsTestBase {
public:
    CallingMethodsTestGeneral() : CallingMethodsTestBase(TEST_BIN_FILE_NAME) {}
};

class RegisterNativeAllocationTest : public CallingMethodsTestGeneral {
    void SetUp() override
    {
        std::vector<EtsVMOption> options_vector;

        options_vector = {{EtsOptionType::EtsGcType, "g1-gc"},
                          {EtsOptionType::EtsRunGcInPlace, nullptr},
                          {EtsOptionType::EtsNoJit, nullptr},
                          {EtsOptionType::EtsBootFile, std::getenv("PANDA_STD_LIB")}};

        if (test_bin_file_name_ != nullptr) {
            options_vector.push_back({EtsOptionType::EtsBootFile, test_bin_file_name_});
        }

        EtsVMInitArgs vm_args;
        vm_args.version = ETS_NAPI_VERSION_1_0;
        vm_args.options = options_vector.data();
        vm_args.nOptions = static_cast<ets_int>(options_vector.size());

        ASSERT_TRUE(ETS_CreateVM(&vm_, &env_, &vm_args) == ETS_OK) << "Cannot create ETS VM";
    }
};

TEST_F(RegisterNativeAllocationTest, testNativeAllocation)
{
    mem::MemStatsType *mem_stats = Thread::GetCurrent()->GetVM()->GetMemStats();

    ets_class test_class = env_->FindClass("NativeAllocationTest");
    ASSERT_NE(test_class, nullptr);

    ets_method alloc_method = env_->GetStaticp_method(test_class, "allocate_object", ":I");
    ASSERT_NE(alloc_method, nullptr);
    ASSERT_EQ(env_->CallStaticIntMethod(test_class, alloc_method), 0);

    size_t heap_freed_before_method;
    {
        panda::ets::napi::ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env_));
        heap_freed_before_method = mem_stats->GetFreedHeap();
    }

    ets_method main_method = env_->GetStaticp_method(test_class, "main_method", ":I");
    ASSERT_NE(main_method, nullptr);
    ASSERT_EQ(env_->CallStaticIntMethod(test_class, main_method), 0);

    size_t heap_freed_after_method;
    {
        panda::ets::napi::ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env_));
        heap_freed_after_method = mem_stats->GetFreedHeap();
    }

    ASSERT_GT(heap_freed_after_method, heap_freed_before_method);
}

}  // namespace panda::ets::test

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
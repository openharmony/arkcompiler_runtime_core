/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>

#include <sstream>

#include "gtest/gtest.h"
#include "runtime/include/runtime.h"
#include "runtime/include/managed_thread.h"
#include "runtime/include/mtmanaged_thread.h"
#include "libarkbase/macros.h"
#include "libarkbase/os/mem.h"

namespace ark::test {

static const size_t MIN_PERMS_LENGTH = 3;
static const char PERM_NONE = '-';
static const uint32_t SECOND_PERM_INDEX = 2;

struct StackVmaInfo {
    uintptr_t start;
    uintptr_t end;
    std::string perms;
};

static bool IsVmaProtNone(const StackVmaInfo &vma)
{
    return vma.perms.size() >= MIN_PERMS_LENGTH && vma.perms[0] == PERM_NONE && vma.perms[1] == PERM_NONE &&
           vma.perms[SECOND_PERM_INDEX] == PERM_NONE;
}

static std::vector<StackVmaInfo> ParseStackVmas()
{
    std::vector<StackVmaInfo> stackVmas;
    std::ifstream mapsFile("/proc/self/maps");
    if (!mapsFile.is_open()) {
        return stackVmas;
    }

    std::string line;
    while (std::getline(mapsFile, line)) {
        if (line.find("[stack]") != std::string::npos) {
            StackVmaInfo vma;
            char dash;
            std::stringstream ss(line);
            ss >> std::hex >> vma.start >> dash >> vma.end >> vma.perms;
            if (dash == '-' && !vma.perms.empty()) {
                stackVmas.push_back(vma);
            }
        }
    }
    return stackVmas;
}

static std::vector<StackVmaInfo> ParseVmasInRange(uintptr_t rangeStart, uintptr_t rangeEnd)
{
    std::vector<StackVmaInfo> result;
    std::ifstream mapsFile("/proc/self/maps");
    if (!mapsFile.is_open()) {
        return result;
    }

    std::string line;
    while (std::getline(mapsFile, line)) {
        StackVmaInfo vma;
        char dash;
        std::stringstream ss(line);
        ss >> std::hex >> vma.start >> dash >> vma.end >> vma.perms;
        if (dash != '-' || vma.perms.empty()) {
            continue;
        }
        if (vma.start < rangeEnd && vma.end > rangeStart) {
            result.push_back(vma);
        }
    }
    return result;
}

class StackOverflowTest : public testing::Test {
public:
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    MTManagedThread *thread_ = nullptr;

    StackOverflowTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        options.SetCompilerEnableJit(false);
        options.SetGcType("epsilon");
        Logger::InitializeStdLogging(Logger::Level::ERROR, 0);
        Runtime::Create(options);

        thread_ = MTManagedThread::GetCurrent();
        if (thread_ == nullptr) {
            thread_ = MTManagedThread::Create(Runtime::GetCurrent(), Runtime::GetCurrent()->GetPandaVM());
            thread_->InitBuffers();
        } else {
            thread_->InitForStackOverflowCheck(ManagedThread::STACK_OVERFLOW_RESERVED_SIZE,
                                               ManagedThread::STACK_OVERFLOW_PROTECTED_SIZE);
        }
    }

    ~StackOverflowTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(StackOverflowTest);
    NO_MOVE_SEMANTIC(StackOverflowTest);
};

TEST_F(StackOverflowTest, StackMappingIsSingleVma)
{
    auto vmas = ParseStackVmas();
    ASSERT_FALSE(vmas.empty()) << "No [stack] VMA found in /proc/self/maps";

    ASSERT_EQ(vmas.size(), 1U) << "Expected single [stack] VMA, but found " << vmas.size()
                               << ". Stack mappings should not be split by mprotect/madvise operations.";

    for (const auto &vma : vmas) {
        LOG(INFO, RUNTIME) << "Stack VMA: " << std::hex << "[" << vma.start << "-" << vma.end
                           << "] permissions: " << vma.perms;
    }
}

TEST_F(StackOverflowTest, ProtectNativeStackSetsProtection)
{
    ASSERT_NE(thread_, nullptr);

    uintptr_t protectedBegin = thread_->GetNativeStackBegin();
    size_t protectedSize = thread_->GetNativeStackProtectedSize();

    ASSERT_GT(protectedSize, 0U);

    LOG(INFO, RUNTIME) << "Protected stack region: [" << std::hex << protectedBegin << "-"
                       << (protectedBegin + protectedSize) << "]";

    thread_->ManagedCodeBegin();

    EXPECT_DEATH_IF_SUPPORTED(
        {
            volatile uint8_t *ptr = reinterpret_cast<volatile uint8_t *>(protectedBegin);
            ptr[0] = 1;
        },
        "");

    thread_->ManagedCodeEnd();
}

TEST_F(StackOverflowTest, DisableEnableStackOverflowCheck)
{
    ASSERT_NE(thread_, nullptr);

    ASSERT_GT(thread_->GetNativeStackProtectedSize(), 0U);

    thread_->ManagedCodeBegin();

    uintptr_t protectedBegin = thread_->GetNativeStackBegin();
    thread_->DisableStackOverflowCheck();

    volatile uint8_t *ptr = reinterpret_cast<volatile uint8_t *>(protectedBegin);
    ptr[0] = 1;

    thread_->EnableStackOverflowCheck();

    EXPECT_DEATH_IF_SUPPORTED({ ptr[0] = 2; }, "");

    thread_->ManagedCodeEnd();
}

TEST_F(StackOverflowTest, NoGapBetweenSystemGuardAndProtectedZone)
{
    ASSERT_NE(thread_, nullptr);
    ASSERT_GT(thread_->GetNativeStackProtectedSize(), 0U);

    uintptr_t protectedBegin = thread_->GetNativeStackBegin();
    size_t protectedSize = thread_->GetNativeStackProtectedSize();
    uintptr_t protectedEnd = protectedBegin + protectedSize;

    void *stackBase = nullptr;
    size_t stackSize = 0;
    size_t guardSize = 0;
    ASSERT_TRUE(thread_->RetrieveStackInfo(stackBase, stackSize, guardSize));

    uintptr_t searchStart = ToUintPtr(stackBase);
    if (searchStart > protectedBegin) {
        searchStart = protectedBegin;
    }
    if (guardSize > 0 && searchStart >= guardSize) {
        searchStart -= guardSize;
    }

    auto vmas = ParseVmasInRange(searchStart, protectedEnd);
    ASSERT_FALSE(vmas.empty()) << "No VMAs found in stack range";

    std::sort(vmas.begin(), vmas.end(), [](const StackVmaInfo &a, const StackVmaInfo &b) { return a.start < b.start; });

    std::vector<StackVmaInfo> protNoneVmas;
    for (const auto &vma : vmas) {
        if (IsVmaProtNone(vma)) {
            protNoneVmas.push_back(vma);
        }
    }

    ASSERT_FALSE(protNoneVmas.empty()) << "No PROT_NONE VMAs found in stack range";

    for (size_t i = 1; i < protNoneVmas.size(); i++) {
        EXPECT_EQ(protNoneVmas[i - 1].end, protNoneVmas[i].start)
            << "Found " << std::dec << (protNoneVmas[i].start - protNoneVmas[i - 1].end)
            << " bytes gap between PROT_NONE regions [" << std::hex << protNoneVmas[i - 1].end << ", "
            << protNoneVmas[i].start << "). This indicates a hole between system guard and runtime protected zone.";
    }
}

}  // namespace ark::test

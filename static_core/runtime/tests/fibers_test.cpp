/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include <cstdlib>
#include <memory>
#include <vector>

#include "gtest/gtest.h"

#include "libpandabase/macros.h"
#include "runtime/fibers/fiber_context.h"

namespace panda::fibers::test {

/// The fixture
class FibersTest : public testing::Test {
public:
    size_t GetEntryExecCounter()
    {
        return entry_exec_counter_;
    }

    void IncEntryExecCounter()
    {
        ++entry_exec_counter_;
    }

private:
    size_t entry_exec_counter_ = 0;
};

/// A fiber instance: provides stack, registers the EP
class Fiber final {
public:
    NO_COPY_SEMANTIC(Fiber);
    NO_MOVE_SEMANTIC(Fiber);

    static constexpr size_t STACK_SIZE = 4096 * 32;
    static constexpr uint32_t MAGIC = 0xC001BEAF;

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    explicit Fiber(FibersTest &fixture, Fiber *parent = nullptr, fibers::FiberEntry entry = nullptr)
        : fixture_(fixture), parent_(parent), magic_(MAGIC)
    {
        stack_ = new uint8_t[STACK_SIZE];
        fibers::GetCurrentContext(&ctx_);
        if (entry != nullptr) {
            fibers::UpdateContext(&ctx_, entry, this, stack_, STACK_SIZE);
        }
    }

    ~Fiber()
    {
        delete[] stack_;
    }

    fibers::FiberContext *GetContextPtr()
    {
        return &ctx_;
    }

    Fiber *GetParent()
    {
        return parent_;
    }

    uint32_t GetMagic()
    {
        return magic_;
    }

    void RegisterEntryExecution()
    {
        fixture_.IncEntryExecCounter();
    }

private:
    FibersTest &fixture_;
    fibers::FiberContext ctx_;
    uint8_t *stack_ = nullptr;
    Fiber *parent_ = nullptr;

    volatile uint32_t magic_;
};

/// Regular fiber EP: switches to parent fiber on return
extern "C" void Entry(void *current_fiber)
{
    ASSERT_TRUE(current_fiber != nullptr);
    auto *f_cur = reinterpret_cast<Fiber *>(current_fiber);
    ASSERT_EQ(f_cur->GetMagic(), Fiber::MAGIC);
    // increment the EP execution counter
    f_cur->RegisterEntryExecution();
    // EOT: switch to parent (otherwise fibers lib will call abort())
    ASSERT_TRUE(f_cur->GetParent() != nullptr);
    fibers::SwitchContext(f_cur->GetContextPtr(), f_cur->GetParent()->GetContextPtr());
}

/// Empty fiber EP: checks what happens on a simple return from a fiber
extern "C" void EmptyEntry([[maybe_unused]] void *current_fiber)
{
    // NOLINTNEXTLINE(readability-redundant-control-flow)
    return;
}

/// The EP that switches back to parent in a loop
extern "C" void LoopedSwitchEntry(void *current_fiber)
{
    ASSERT_TRUE(current_fiber != nullptr);
    auto *f_cur = reinterpret_cast<Fiber *>(current_fiber);
    ASSERT_EQ(f_cur->GetMagic(), Fiber::MAGIC);

    // some non-optimized counters...
    volatile size_t counter_int = 0;
    volatile double counter_dbl = 0;
    while (true) {
        // ...and their modification...
        ++counter_int;
        counter_dbl = static_cast<double>(counter_int);

        f_cur->RegisterEntryExecution();
        ASSERT_TRUE(f_cur->GetParent() != nullptr);
        fibers::SwitchContext(f_cur->GetContextPtr(), f_cur->GetParent()->GetContextPtr());

        // ...and the check for the counters to stay correct after the switch
        ASSERT_DOUBLE_EQ(counter_dbl, static_cast<double>(counter_int));
    }
}

/* Tests*/

/// Create fiber, switch to it, execute till its end, switch back
TEST_F(FibersTest, SwitchExecuteSwitchBack)
{
    Fiber f_init(*this);
    Fiber f1(*this, &f_init, Entry);
    fibers::SwitchContext(f_init.GetContextPtr(), f1.GetContextPtr());

    ASSERT_EQ(GetEntryExecCounter(), 1);
}

/**
 * Create several fibers, organizing them in a chain using the "parent" field.
 * Switch to the last one, wait till the whole chain is executed
 */
TEST_F(FibersTest, ChainSwitch)
{
    Fiber f_init(*this);
    Fiber f1(*this, &f_init, Entry);
    Fiber f2(*this, &f1, Entry);
    Fiber f3(*this, &f2, Entry);
    fibers::SwitchContext(f_init.GetContextPtr(), f3.GetContextPtr());

    ASSERT_EQ(GetEntryExecCounter(), 3);
}

/// Create the child fiber, then switch context back and forth several times in a loop
TEST_F(FibersTest, LoopedSwitch)
{
    constexpr size_t SWITCHES = 10;

    Fiber f_init(*this);
    Fiber f_target(*this, &f_init, LoopedSwitchEntry);

    // some unoptimized counters
    volatile size_t counter_int = 0;
    volatile double counter_dbl = 0;
    for (size_t i = 0; i < SWITCHES; ++i) {
        counter_int = i;
        counter_dbl = static_cast<double>(i);

        // do something with the context before the next switch
        double n1 = 0;
        double n2 = 0;
        // NOLINTNEXTLINE(cert-err34-c, cppcoreguidelines-pro-type-vararg)
        sscanf("1.23 4.56", "%lf %lf", &n1, &n2);

        fibers::SwitchContext(f_init.GetContextPtr(), f_target.GetContextPtr());

        // check that no corruption occurred
        ASSERT_DOUBLE_EQ(n1, 1.23);
        ASSERT_DOUBLE_EQ(n2, 4.56);

        // counters should not be corrupted after a switch
        ASSERT_EQ(counter_int, i);
        ASSERT_DOUBLE_EQ(counter_dbl, static_cast<double>(i));
    }

    ASSERT_EQ(GetEntryExecCounter(), SWITCHES);
}

using FibersDeathTest = FibersTest;
/**
 * Death test. Creates an orphaned fiber that will silently return from its entry function.
 * Should cause the program to be abort()-ed
 */
TEST_F(FibersDeathTest, AbortOnFiberReturn)
{
    Fiber f_init(*this);
    Fiber f_aborts(*this, nullptr, EmptyEntry);
    EXPECT_EXIT(fibers::SwitchContext(f_init.GetContextPtr(), f_aborts.GetContextPtr()),
                testing::KilledBySignal(SIGABRT), ".*");
}

}  // namespace panda::fibers::test

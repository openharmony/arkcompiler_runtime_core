/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include <climits>

#include <gtest/gtest.h>
#include <random>
#include <sstream>

#include "libpandabase/os/filesystem.h"
#include "mem/base_mem_stats.h"
#include "mem/code_allocator.h"
#include "mem/pool_manager.h"
#include "os/filesystem.h"
#include "target/asm_printer.h"

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static std::string g_outputDir = "asm_output";

// Debug print to stdout
#if ENABLE_DEBUG_STDOUT_PRINT
#define STDOUT_PRINT
#endif  // ENABLE_DEBUG_STDOUT_PRINT

namespace ark::compiler {

template <Arch ARCH>
class PrinterTest : public ::testing::Test {
public:
    PrinterTest()
    {
        // NOLINTNEXTLINE(readability-magic-numbers)
        ark::mem::MemConfig::Initialize(64_MB, 64_MB, 64_MB, 32_MB, 0U, 0U);
#ifdef STDOUT_PRINT
        curr_stream_ = &std::cout;
#else
        currStream_ = new std::stringstream();
#endif
        PoolManager::Initialize();
        allocator_ = new ArenaAllocator(SpaceType::SPACE_TYPE_COMPILER);
        // Create printer
        encoder_ = Encoder::Create(allocator_, ARCH, true);
        encoder_->InitMasm();
        regfile_ = RegistersDescription::Create(allocator_, ARCH);
        bool pabi = false;
        bool osr = false;
        bool dyn = false;
        bool print = true;
#ifdef PANDA_COMPILER_TARGET_AARCH32
        if constexpr (ARCH == Arch::AARCH32) {
            dirSuffix_ = "aarch32";
            auto enc = reinterpret_cast<aarch32::Aarch32Assembly *>(encoder_);
            enc->SetStream(currStream_);
            callconv_ = CallingConvention::Create(allocator_, enc, regfile_, ARCH, pabi, osr, dyn, print);
            enc->GetEncoder()->SetRegfile(regfile_);
        }
#endif
#ifdef PANDA_COMPILER_TARGET_AARCH64
        if constexpr (ARCH == Arch::AARCH64) {
            dirSuffix_ = "aarch64";
            auto enc = reinterpret_cast<aarch64::Aarch64Assembly *>(encoder_);
            enc->SetStream(currStream_);
            callconv_ = CallingConvention::Create(allocator_, enc, regfile_, ARCH, pabi, osr, dyn, print);
            enc->GetEncoder()->SetRegfile(regfile_);
        }
#endif
#ifdef PANDA_COMPILER_TARGET_X86_64
        if constexpr (ARCH == Arch::X86_64) {
            dirSuffix_ = "amd64";
            auto enc = reinterpret_cast<amd64::Amd64Assembly *>(encoder_);
            enc->SetStream(currStream_);
            callconv_ = CallingConvention::Create(allocator_, enc, regfile_, ARCH, pabi, osr, dyn, print);
            enc->GetEncoder()->SetRegfile(regfile_);
        }
#endif
        memStats_ = new BaseMemStats();
        codeAlloc_ = new (std::nothrow) CodeAllocator(memStats_);
        // Create dir if it is not exist
        auto execPath = ark::os::file::File::GetExecutablePath();
        ASSERT(execPath);
        execPath_ = execPath.Value();
        os::CreateDirectories(execPath_ + "/" + g_outputDir);
        os::CreateDirectories(GetDir());
    }
    ~PrinterTest() override
    {
        Logger::Destroy();
        encoder_->~Encoder();
        delete allocator_;
        delete codeAlloc_;
        delete memStats_;
        PoolManager::Finalize();
        ark::mem::MemConfig::Finalize();
        delete currStream_;
    }

    NO_MOVE_SEMANTIC(PrinterTest);
    NO_COPY_SEMANTIC(PrinterTest);

    CodeAllocator *GetCodeAllocator()
    {
        return codeAlloc_;
    }

    std::string GetDir()
    {
        return execPath_ + "/" + g_outputDir + "/" + dirSuffix_ + "/";
    }

    void ResetCodeAllocator(void *ptr, size_t size)
    {
        os::mem::MapRange<std::byte> memRange(static_cast<std::byte *>(ptr), size);
        memRange.MakeReadWrite();
        delete codeAlloc_;
        codeAlloc_ = new (std::nothrow) CodeAllocator(memStats_);
    }

    ArenaAllocator *GetAllocator()
    {
        return allocator_;
    }

    Encoder *GetEncoder()
    {
        return encoder_;
    }

    RegistersDescription *GetRegfile()
    {
        return regfile_;
    }

    CallingConvention *GetCallconv()
    {
        return callconv_;
    }

    size_t GetCursor()
    {
        return currCursor_;
    }

    // Warning! Do not use multiply times with different types!
    Reg GetParameter(TypeInfo type, size_t id = 0U)
    {
        ASSERT(id < 4U);
        if (type.IsFloat()) {
            return Reg(id, type);
        }
        if constexpr (ARCH == Arch::AARCH32) {
            // special offset for double-reg param
            if (id == 1U && type.GetSize() == DOUBLE_WORD_SIZE) {
                return Target::Current().GetParamReg(2U, type);
            }
        }
        return Target::Current().GetParamReg(id, type);
    }

    void PreWork()
    {
        // Curor need to encode multiply tests due one execution
        currCursor_ = 0;
        encoder_->SetCursorOffset(0U);

        [[maybe_unused]] std::string funcName = "test_" + GetTestName();
#ifdef PANDA_COMPILER_TARGET_AARCH32
        if constexpr (ARCH == Arch::AARCH32) {
            auto enc = reinterpret_cast<aarch32::Aarch32Assembly *>(encoder_);
            enc->EmitFunctionName(reinterpret_cast<const void *>(funcName.c_str()));
        }
#endif
#ifdef PANDA_COMPILER_TARGET_AARCH64
        if constexpr (ARCH == Arch::AARCH64) {
            auto enc = reinterpret_cast<aarch64::Aarch64Assembly *>(encoder_);
            enc->EmitFunctionName(reinterpret_cast<const void *>(funcName.c_str()));
        }
#endif
#ifdef PANDA_COMPILER_TARGET_X86_64
        if constexpr (ARCH == Arch::X86_64) {
            auto enc = reinterpret_cast<amd64::Amd64Assembly *>(encoder_);
            enc->EmitFunctionName(reinterpret_cast<const void *>(funcName.c_str()));
        }
#endif
        callconv_->GeneratePrologue(FrameInfo::FullPrologue());
    }

    void PostWork()
    {
        auto param = Target::Current().GetParamReg(0U);
        auto returnReg = Target::Current().GetReturnReg();
        if (param.GetId() != returnReg.GetId()) {
            GetEncoder()->EncodeMov(returnReg, param);
        }
        callconv_->GenerateEpilogue(FrameInfo::FullPrologue(), []() {});
        encoder_->Finalize();
    }

#ifdef STDOUT_PRINT
    std::ostream *GetStream()
#else
    std::stringstream *GetStream()
#endif
    {
        return currStream_;
    }

    void SetTestName(std::string name)
    {
        testName_ = std::move(name);
    }

    std::string GetTestName()
    {
        return testName_;
    }

    void FinalizeTest()
    {
        // Make them separate!
        std::string filename = GetTestName() + ".S";
        std::ofstream asmFile;
        asmFile.open(GetDir() + "/" + filename);
        // Test must generate asembly-flie
        ASSERT(asmFile.is_open());
        // Compile by assembler
#ifndef STDOUT_PRINT
        asmFile << GetStream()->str();
#endif
        asmFile.close();
    }

private:
    ArenaAllocator *allocator_ {nullptr};
    Encoder *encoder_ {nullptr};
    RegistersDescription *regfile_ {nullptr};
    // Callconv for printing
    CallingConvention *cc_ {nullptr};
    // Callconv for masm initialization
    CallingConvention *callconv_ {nullptr};
    CodeAllocator *codeAlloc_ {nullptr};
    BaseMemStats *memStats_ {nullptr};
    size_t currCursor_ {0U};
#ifdef STDOUT_PRINT
    std::ostream *curr_stream_;
#else
    std::stringstream *currStream_;
#endif
    std::string testName_;
    std::string execPath_;
    std::string dirSuffix_;
};

#ifdef PANDA_COMPILER_TARGET_AARCH32
using PrinterAarch32Test = PrinterTest<Arch::AARCH32>;
#endif

#ifdef PANDA_COMPILER_TARGET_AARCH64
using PrinterAarch64Test = PrinterTest<Arch::AARCH64>;
#endif

#ifdef PANDA_COMPILER_TARGET_X86_64
using PrinterAmd64Test = PrinterTest<Arch::X86_64>;
#endif

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define SINGLE_PARAM_TEST_TEMPLATE(test_name, encode_func) \
    template <typename T, Arch arch>                       \
    bool test_name(PrinterTest<arch> *test)                \
    {                                                      \
        test->PreWork();                                   \
        auto param = test->GetParameter(TypeInfo(T(0)));   \
        test->GetEncoder()->encode_func(param, param);     \
        test->PostWork();                                  \
        return test->GetEncoder()->GetResult();            \
    }

SINGLE_PARAM_TEST_TEMPLATE(TestMov, EncodeMov)
SINGLE_PARAM_TEST_TEMPLATE(TestNeg, EncodeNeg)
SINGLE_PARAM_TEST_TEMPLATE(TestAbs, EncodeAbs)
SINGLE_PARAM_TEST_TEMPLATE(TestNot, EncodeNot)
// SINGLE_PARAM_TEST_TEMPLATE(TestSqrt, EncodeSqrt)

#undef SINGLE_PARAM_TEST_TEMPLATE

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DOUBLE_PARAM_TEST_TEMPLATE(test_name, encode_func)       \
    template <typename T, Arch arch>                             \
    bool test_name(PrinterTest<arch> *test)                      \
    {                                                            \
        test->PreWork();                                         \
        auto param1 = test->GetParameter(TypeInfo(T(0)), 0);     \
        auto param2 = test->GetParameter(TypeInfo(T(0)), 1);     \
        test->GetEncoder()->encode_func(param1, param1, param2); \
        test->PostWork();                                        \
        return test->GetEncoder()->GetResult();                  \
    }

DOUBLE_PARAM_TEST_TEMPLATE(TestAdd, EncodeAdd)
DOUBLE_PARAM_TEST_TEMPLATE(TestSub, EncodeSub)
DOUBLE_PARAM_TEST_TEMPLATE(TestMul, EncodeMul)
DOUBLE_PARAM_TEST_TEMPLATE(TestShl, EncodeShl)
DOUBLE_PARAM_TEST_TEMPLATE(TestShr, EncodeShr)
DOUBLE_PARAM_TEST_TEMPLATE(TestAShr, EncodeAShr)
DOUBLE_PARAM_TEST_TEMPLATE(TestAnd, EncodeAnd)
DOUBLE_PARAM_TEST_TEMPLATE(TestOr, EncodeOr)
DOUBLE_PARAM_TEST_TEMPLATE(TestXor, EncodeXor)

#undef DOUBLE_PARAM_TEST_TEMPLATE

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ONE_TEST_BODY(test_class, test_method, test_name, arch) \
    TEST_F(test_class, test_method)                             \
    {                                                           \
        bool success = true;                                    \
        SetTestName(#test_name "_8");                           \
        success &= test_method<uint8_t, Arch::arch>(this);      \
        SetTestName(#test_name "_16");                          \
        success &= test_method<uint16_t, Arch::arch>(this);     \
        SetTestName(#test_name "_32");                          \
        success &= test_method<uint32_t, Arch::arch>(this);     \
        SetTestName(#test_name "_64");                          \
        success &= test_method<uint64_t, Arch::arch>(this);     \
        SetTestName(#test_name);                                \
        FinalizeTest();                                         \
        EXPECT_TRUE(success);                                   \
    }

#ifdef PANDA_COMPILER_TARGET_AARCH64
ONE_TEST_BODY(PrinterAarch64Test, TestMov, mov, AARCH64)
ONE_TEST_BODY(PrinterAarch64Test, TestNeg, neg, AARCH64)
ONE_TEST_BODY(PrinterAarch64Test, TestAbs, abs, AARCH64)
ONE_TEST_BODY(PrinterAarch64Test, TestNot, not, AARCH64)
ONE_TEST_BODY(PrinterAarch64Test, TestAdd, add, AARCH64)
ONE_TEST_BODY(PrinterAarch64Test, TestSub, sub, AARCH64)
ONE_TEST_BODY(PrinterAarch64Test, TestMul, mul, AARCH64)
ONE_TEST_BODY(PrinterAarch64Test, TestShl, shl, AARCH64)
ONE_TEST_BODY(PrinterAarch64Test, TestShr, shr, AARCH64)
ONE_TEST_BODY(PrinterAarch64Test, TestAShr, ashr, AARCH64)
ONE_TEST_BODY(PrinterAarch64Test, TestAnd, and, AARCH64)
ONE_TEST_BODY(PrinterAarch64Test, TestOr, or, AARCH64)
ONE_TEST_BODY(PrinterAarch64Test, TestXor, xor, AARCH64)
#endif

#ifdef PANDA_COMPILER_TARGET_AARCH32
ONE_TEST_BODY(PrinterAarch32Test, TestMov, mov, AARCH32)
ONE_TEST_BODY(PrinterAarch32Test, TestNeg, neg, AARCH32)
ONE_TEST_BODY(PrinterAarch32Test, TestAbs, abs, AARCH32)
ONE_TEST_BODY(PrinterAarch32Test, TestNot, not, AARCH32)
ONE_TEST_BODY(PrinterAarch32Test, TestAdd, add, AARCH32)
ONE_TEST_BODY(PrinterAarch32Test, TestSub, sub, AARCH32)
ONE_TEST_BODY(PrinterAarch32Test, TestMul, mul, AARCH32)
ONE_TEST_BODY(PrinterAarch32Test, TestShl, shl, AARCH32)
ONE_TEST_BODY(PrinterAarch32Test, TestShr, shr, AARCH32)
ONE_TEST_BODY(PrinterAarch32Test, TestAShr, ashr, AARCH32)
ONE_TEST_BODY(PrinterAarch32Test, TestAnd, and, AARCH32)
ONE_TEST_BODY(PrinterAarch32Test, TestOr, or, AARCH32)
ONE_TEST_BODY(PrinterAarch32Test, TestXor, xor, AARCH32)
#endif

#ifdef PANDA_COMPILER_TARGET_X86_64
ONE_TEST_BODY(PrinterAmd64Test, TestMov, mov, X86_64)
ONE_TEST_BODY(PrinterAmd64Test, TestNeg, neg, X86_64)
ONE_TEST_BODY(PrinterAmd64Test, TestAbs, abs, X86_64)
ONE_TEST_BODY(PrinterAmd64Test, TestNot, not, X86_64)
ONE_TEST_BODY(PrinterAmd64Test, TestAdd, add, X86_64)
ONE_TEST_BODY(PrinterAmd64Test, TestSub, sub, X86_64)
ONE_TEST_BODY(PrinterAmd64Test, TestMul, mul, X86_64)
ONE_TEST_BODY(PrinterAmd64Test, TestShl, shl, X86_64)
ONE_TEST_BODY(PrinterAmd64Test, TestShr, shr, X86_64)
ONE_TEST_BODY(PrinterAmd64Test, TestAShr, ashr, X86_64)
ONE_TEST_BODY(PrinterAmd64Test, TestAnd, and, X86_64)
ONE_TEST_BODY(PrinterAmd64Test, TestOr, or, X86_64)
ONE_TEST_BODY(PrinterAmd64Test, TestXor, xor, X86_64)
#endif

}  // namespace ark::compiler

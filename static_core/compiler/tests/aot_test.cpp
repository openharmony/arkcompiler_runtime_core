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

#include "unit_test.h"
#include "aot/aot_manager.h"
#include "aot/aot_builder/aot_builder.h"
#include "aot/compiled_method.h"
#include "compiler/code_info/code_info_builder.h"
#include "libarkbase/os/exec.h"
#include "libarkbase/os/filesystem.h"
#include "assembly-parser.h"
#include <elf.h>
#include "libarkbase/utils/string_helpers.h"
#include "libarkbase/events/events.h"
#include "mem/gc/gc_types.h"
#include "runtime/include/file_manager.h"
#include "zip_archive.h"

#include <array>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <regex>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

using ark::panda_file::File;

namespace ark::compiler {
class AotTest : public AsmTest {
public:
    AotTest()
    {
        std::string exePath = GetExecPath();
        auto pos = exePath.rfind('/');
        paocPath_ = exePath.substr(0U, pos) + "/../bin/ark_aot";
        aotdumpPath_ = exePath.substr(0U, pos) + "/../bin/ark_aotdump";
    }

    ~AotTest() override = default;

    NO_MOVE_SEMANTIC(AotTest);
    NO_COPY_SEMANTIC(AotTest);

    const char *GetPaocFile() const
    {
        return paocPath_.c_str();
    }

    const char *GetAotdumpFile() const
    {
        return aotdumpPath_.c_str();
    }

    std::string GetPaocDirectory() const
    {
        auto pos = paocPath_.rfind('/');
        return paocPath_.substr(0U, pos);
    }

    const char *GetArchAsArgString() const
    {
        switch (targetArch_) {
            case Arch::AARCH32:
                return "arm";
            case Arch::AARCH64:
                return "arm64";
            case Arch::X86:
                return "x86";
            case Arch::X86_64:
                return "x86_64";
            default:
                UNREACHABLE();
        }
    }

    void RunAotdump(const std::string &aotFilename)
    {
        TmpFile tmpfile("aotdump.tmp");

        auto res = os::exec::Exec(GetAotdumpFile(), "--show-code=disasm", "--output-file", tmpfile.GetFileName(),
                                  aotFilename.c_str());
        ASSERT_TRUE(res) << "aotdump failed with error: " << res.Error().ToString();
        ASSERT_EQ(res.Value(), 0U) << "aotdump return error code: " << res.Value();
    }

    void CreateTestZip(const char *archivename, const char *pandaFilename, std::vector<uint8_t> &pfData)
    {
        pandasm::Parser parser;
        auto source = R"(
            .function void f1() {
                return.void
            }
        )";
        auto res = parser.Parse(source, pandaFilename);
        ASSERT_TRUE(res);
        auto pf = pandasm::AsmEmitter::Emit(res.Value());
        ASSERT_NE(pf, nullptr);
        const auto headerPtr = reinterpret_cast<const uint8_t *>(pf->GetHeader());
        pfData.assign(headerPtr, headerPtr + pf->GetHeader()->fileSize);
        ASSERT_EQ(CreateOrAddFileIntoZip(archivename, pandaFilename, &pfData, APPEND_STATUS_CREATE, Z_NO_COMPRESSION),
                  0);
    }

    void SpliceMemfdToDisk(int memfd, const char *diskPath)
    {
        lseek(memfd, 0, SEEK_SET);
        int diskFd = open(diskPath, O_RDWR | O_CREAT | O_TRUNC, 0644);
        ASSERT_GE(diskFd, 0);
        char buf[4096];
        ssize_t n;
        while ((n = read(memfd, buf, sizeof(buf))) > 0) {
            ASSERT_EQ(write(diskFd, buf, n), n);
        }
        fsync(diskFd);
        close(diskFd);
    }

private:
    Arch targetArch_ = Arch::AARCH64;
    std::string paocPath_;
    std::string aotdumpPath_;
};

// NOLINTBEGIN(readability-magic-numbers)
#ifdef PANDA_COMPILER_TARGET_AARCH64
TEST_F(AotTest, PaocBootPandaFiles)
{
    // Test basic functionality only in host mode.
    if (RUNTIME_ARCH != Arch::X86_64) {
        return;
    }
    TmpFile pandaFname("test.pf");
    TmpFile aotFname("./test.an");
    static const std::string LOCATION = "/data/local/tmp";
    static const std::string PANDA_FILE_PATH = LOCATION + "/" + pandaFname.GetFileName();

    auto source = R"(
        .function void dummy() {
            return.void
        }
    )";

    {
        pandasm::Parser parser;
        auto res = parser.Parse(source);
        ASSERT_TRUE(res);
        ASSERT_TRUE(pandasm::AsmEmitter::Emit(pandaFname.GetFileName(), res.Value()));
    }

    // Correct path to arkstdlib.abc
    {
        auto pandastdlibPath = GetPaocDirectory() + "/../pandastdlib/arkstdlib.abc";
        auto res = os::exec::Exec(GetPaocFile(), "--paoc-panda-files", pandaFname.GetFileName(), "--paoc-output",
                                  aotFname.GetFileName(), "--paoc-location", LOCATION.c_str(), "--compiler-cross-arch",
                                  GetArchAsArgString(), "--boot-panda-files", pandastdlibPath.c_str());
        ASSERT_TRUE(res) << "paoc failed with error: " << res.Error().ToString();
        ASSERT_EQ(res.Value(), 0U) << "Aot compiler failed with code " << res.Value();
        RunAotdump(aotFname.GetFileName());
    }
}

TEST_F(AotTest, PaocLocation)
{
    // Test basic functionality only in host mode.
    if (RUNTIME_ARCH != Arch::X86_64) {
        return;
    }
    TmpFile pandaFname("test.pf");
    TmpFile aotFname("./test.an");
    static const std::string LOCATION = "/data/local/tmp";
    static const std::string PANDA_FILE_PATH = LOCATION + "/" + pandaFname.GetFileName();

    auto source = R"(
        .function u32 add(u64 a0, u64 a1) {
            add a0, a1
            return
        }
    )";
    {
        pandasm::Parser parser;
        auto res = parser.Parse(source);
        ASSERT_TRUE(res);
        ASSERT_TRUE(pandasm::AsmEmitter::Emit(pandaFname.GetFileName(), res.Value()));
    }

    {
        auto pandastdlibPath = GetPaocDirectory() + "/../pandastdlib/arkstdlib.abc";
        auto res = os::exec::Exec(GetPaocFile(), "--paoc-panda-files", pandaFname.GetFileName(), "--paoc-output",
                                  aotFname.GetFileName(), "--paoc-location", LOCATION.c_str(),
                                  "--compiler-cross-arch=x86_64", "--gc-type=epsilon", "--paoc-use-cha=false");
        ASSERT_TRUE(res) << "paoc failed with error: " << res.Error().ToString();
        ASSERT_EQ(res.Value(), 0U) << "Aot compiler failed with code " << res.Value();
    }

    AotManager aotManager;
    {
        auto res = aotManager.AddFile(aotFname.GetFileName(), nullptr, static_cast<uint32_t>(mem::GCType::EPSILON_GC));
        ASSERT_TRUE(res) << res.Error();
    }

    auto aotFile = aotManager.GetFile(aotFname.GetFileName());
    ASSERT_TRUE(aotFile);
    ASSERT_EQ(aotFile->GetFilesCount(), 1U);
    ASSERT_TRUE(aotFile->FindPandaFile(PANDA_FILE_PATH));
}
#endif  // PANDA_COMPILER_TARGET_AARCH64

class AotTestBuildAndCheck : public AsmTest {
public:
    void BuildAot(const char *tmpfilePn, mem::GCType gcType)
    {
        AotBuilder aotBuilder;
        aotBuilder.SetArch(RUNTIME_ARCH);
        auto runtime = Runtime::GetCurrent();
        aotBuilder.SetGcType(static_cast<uint32_t>(gcType));
        RuntimeInterfaceMock iruntime;
        aotBuilder.SetRuntime(&iruntime);

        aotBuilder.StartFile(tmpFilePf_, 0x12345678U);

        auto thread = MTManagedThread::GetCurrent();
        if (thread != nullptr) {
            thread->ManagedCodeBegin();
        }
        auto etx = runtime->GetClassLinker()->GetExtension(runtime->GetLanguageContext(runtime->GetRuntimeType()));
        klass_ = etx->CreateClass(reinterpret_cast<const uint8_t *>(className_.data()), 0, 0,
                                  AlignUp(sizeof(Class), OBJECT_POINTER_SIZE));
        if (thread != nullptr) {
            thread->ManagedCodeEnd();
        }

        klass_->SetFileId(panda_file::File::EntityId(13U));
        aotBuilder.StartClass(*klass_);

        Method method1(klass_, nullptr, File::EntityId(method1Id_), File::EntityId(), 0U, 1U, nullptr);
        {
            CodeInfoBuilder codeBuilder(RUNTIME_ARCH, GetAllocator());
            ArenaVector<uint8_t> data(GetAllocator()->Adapter());
            codeBuilder.Encode(&data);
            CompiledMethod compiledMethod1(RUNTIME_ARCH, &method1, 0U);
            compiledMethod1.SetCode(
                Span(reinterpret_cast<const uint8_t *>(methodName_.data()), methodName_.size() + 1U));
            compiledMethod1.SetCodeInfo(Span(data).ToConst());
            aotBuilder.AddMethod(compiledMethod1);
        }

        Method method2(klass_, nullptr, File::EntityId(method2Id_), File::EntityId(), 0U, 1U, nullptr);
        {
            CodeInfoBuilder codeBuilder(RUNTIME_ARCH, GetAllocator());
            ArenaVector<uint8_t> data(GetAllocator()->Adapter());
            codeBuilder.Encode(&data);
            CompiledMethod compiledMethod2(RUNTIME_ARCH, &method2, 1U);
            compiledMethod2.SetCode(Span(reinterpret_cast<const uint8_t *>(x86Add_.data()), x86Add_.size()));
            compiledMethod2.SetCodeInfo(Span(data).ToConst());
            aotBuilder.AddMethod(compiledMethod2);
        }

        klass_->SetMethods(Span<Method>(&method1, &method2), 2U, 0U);
        aotBuilder.EndClass();
        uint32_t hash = GetHash32String(reinterpret_cast<const uint8_t *>(className_.data()));
        aotBuilder.InsertEntityPairHeader(hash, 13U);
        aotBuilder.InsertClassHashTableSize(1U);
        aotBuilder.EndFile();

        aotBuilder.Write(cmdline_, tmpfilePn);
    }

    void DoChecks(AotManager *aotManager, const char *tmpfilePn)
    {
        auto aotFile = aotManager->GetFile(tmpfilePn);
        ASSERT_TRUE(aotFile);
        ASSERT_TRUE(strcmp(cmdline_, aotFile->GetCommandLine()) == 0U);
        ASSERT_TRUE(strcmp(tmpfilePn, aotFile->GetFileName()) == 0U);
        ASSERT_EQ(aotFile->GetFilesCount(), 1U);

        auto pfile = aotManager->FindPandaFile(tmpFilePf_);
        ASSERT_NE(pfile, nullptr);
        auto cls = pfile->GetClass(13U);
        ASSERT_TRUE(cls.IsValid());

        {
            auto code = cls.FindMethodCodeEntry(0U);
            ASSERT_FALSE(code == nullptr);
            ASSERT_EQ(methodName_, reinterpret_cast<const char *>(code));
        }

        {
            auto code = cls.FindMethodCodeEntry(1U);
            ASSERT_FALSE(code == nullptr);
            ASSERT_EQ(std::memcmp(x86Add_.data(), code, x86Add_.size()), 0U);
#ifdef PANDA_TARGET_AMD64
            auto funcAdd = reinterpret_cast<int (*)(int, int)>(const_cast<void *>(code));
            ASSERT_EQ(funcAdd(2U, 3U), 5U);
#endif
        }
    }

    void CheckGnuHashAndEntrySymbols(const char *tmpfilePn)
    {
        auto data = ReadFile(tmpfilePn);
        ASSERT_GT(data.size(), EI_CLASS);
        // Dispatch to the matching ELF type layout.
        switch (data[EI_CLASS]) {
            case ELFCLASS32:
                CheckGnuHashAndEntrySymbolsImpl<Elf32_Ehdr, Elf32_Shdr, Elf32_Dyn, Elf32_Sym>(data);
                break;
            case ELFCLASS64:
                CheckGnuHashAndEntrySymbolsImpl<Elf64_Ehdr, Elf64_Shdr, Elf64_Dyn, Elf64_Sym>(data);
                break;
            default:
                FAIL() << "Unexpected ELF class " << static_cast<int>(data[EI_CLASS]);
        }
    }

    void CheckLinkAotCodeForBoot(AotManager *aotManager, const char *tmpfilePn)
    {
        auto aotFile = aotManager->GetFile(tmpfilePn);
        ASSERT_TRUE(aotFile);
        ASSERT_TRUE(strcmp(cmdline_, aotFile->GetCommandLine()) == 0U);
        ASSERT_TRUE(strcmp(tmpfilePn, aotFile->GetFileName()) == 0U);
        ASSERT_EQ(aotFile->GetFilesCount(), 1U);

        const AotPandaFile *aotPfile = aotManager->FindPandaFile(tmpFilePf_);
        ASSERT_NE(aotPfile, nullptr);

        Runtime *runtime = Runtime::GetCurrent();
        ClassLinker *classLinker = runtime->GetClassLinker();
        panda_file::SourceLang lang = plugins::RuntimeTypeToLang(runtime->GetRuntimeType());
        ClassLinkerContext *bootContext = classLinker->GetExtension(lang)->GetBootContext();

        bootContext->InsertClass(klass_);

        bootContext->EnumerateClasses([aotPfile, this](Class *klass) -> bool {
            if (klass->GetFileId().GetOffset() != 13U) {
                return true;
            }

            EXPECT_EQ(klass, klass_);
            Span<Method> methods = klass->GetMethods();
            EXPECT_EQ(methods.size(), 2U);

            compiler::AotClass aotClass = aotPfile->GetClass(klass->GetFileId().GetOffset());
            EXPECT_TRUE(aotClass.IsValid());
            auto entry0 = aotClass.FindMethodCodeEntry(0U);
            EXPECT_FALSE(entry0 == nullptr);
            EXPECT_EQ(methodName_, reinterpret_cast<const char *>(entry0));

            auto entry1 = aotClass.FindMethodCodeEntry(1U);
            EXPECT_FALSE(entry1 == nullptr);
            EXPECT_EQ(std::memcmp(x86Add_.data(), entry1, x86Add_.size()), 0U);
#ifdef PANDA_TARGET_AMD64
            auto funcAdd = reinterpret_cast<int (*)(int, int)>(const_cast<void *>(entry1));
            EXPECT_EQ(funcAdd(2U, 3U), 5U);
#endif
            return true;
        });

        bootContext->RemoveClass(klass_);
    };

    void CheckLinkAotCodeForBootAotClassInvalid(AotManager *aotManager, const char *tmpfilePn)
    {
        auto aotFile = aotManager->GetFile(tmpfilePn);
        ASSERT_TRUE(aotFile);
        ASSERT_TRUE(strcmp(cmdline_, aotFile->GetCommandLine()) == 0U);
        ASSERT_TRUE(strcmp(tmpfilePn, aotFile->GetFileName()) == 0U);
        ASSERT_EQ(aotFile->GetFilesCount(), 1U);

        const AotPandaFile *aotPfile = aotManager->FindPandaFile(tmpFilePf_);
        ASSERT_NE(aotPfile, nullptr);

        Runtime *runtime = Runtime::GetCurrent();
        ClassLinker *classLinker = runtime->GetClassLinker();
        panda_file::SourceLang lang = plugins::RuntimeTypeToLang(runtime->GetRuntimeType());
        ClassLinkerContext *bootContext = classLinker->GetExtension(lang)->GetBootContext();

        auto thread = MTManagedThread::GetCurrent();
        if (thread != nullptr) {
            thread->ManagedCodeBegin();
        }
        auto etx = runtime->GetClassLinker()->GetExtension(runtime->GetLanguageContext(runtime->GetRuntimeType()));
        Class *klass = etx->CreateClass(reinterpret_cast<const uint8_t *>("Bar"), 0, 0,
                                        AlignUp(sizeof(Class), OBJECT_POINTER_SIZE));
        if (thread != nullptr) {
            thread->ManagedCodeEnd();
        }

        klass->SetFileId(panda_file::File::EntityId(14U));

        uint32_t methodId = 44;
        Method method(klass, nullptr, File::EntityId(methodId), File::EntityId(), 0U, 1U, nullptr);
        klass->SetMethods(Span<Method>(&method, 1U), 1U, 0U);

        bootContext->InsertClass(klass);
        classLinker->TryReLinkAotCodeForBoot(nullptr, aotPfile, lang);

        compiler::AotClass aotClass = aotPfile->GetClass(klass->GetFileId().GetOffset());
        ASSERT_FALSE(aotClass.IsValid());
        ASSERT_EQ(method.GetCompiledEntryPoint(), nullptr);

        bootContext->RemoveClass(klass);
    };

private:
    static std::vector<uint8_t> ReadFile(const char *fileName)
    {
        std::ifstream file(fileName, std::ios::binary | std::ios::ate);
        EXPECT_TRUE(file.is_open());
        if (!file.is_open()) {
            return {};
        }
        auto size = file.tellg();
        EXPECT_GE(size, 0);
        if (size < 0) {
            return {};
        }
        std::vector<uint8_t> data(static_cast<size_t>(size));
        file.seekg(0);
        file.read(reinterpret_cast<char *>(data.data()), static_cast<std::streamsize>(data.size()));
        EXPECT_TRUE(file.good());
        return data;
    }

    template <class Ehdr, class Shdr, class Dyn, class Sym>
    void CheckGnuHashAndEntrySymbolsImpl(const std::vector<uint8_t> &data)
    {
        // Parse section headers and the section name string table.
        ASSERT_GE(data.size(), sizeof(Ehdr));
        auto *ehdr = reinterpret_cast<const Ehdr *>(data.data());
        ASSERT_EQ(ehdr->e_ident[EI_MAG0], ELFMAG0);
        ASSERT_EQ(ehdr->e_ident[EI_MAG1], ELFMAG1);
        ASSERT_EQ(ehdr->e_ident[EI_MAG2], ELFMAG2);
        ASSERT_EQ(ehdr->e_ident[EI_MAG3], ELFMAG3);
        ASSERT_LT(ehdr->e_shstrndx, ehdr->e_shnum);
        ASSERT_LE(ehdr->e_shoff + ehdr->e_shnum * sizeof(Shdr), data.size());

        auto *sections = reinterpret_cast<const Shdr *>(data.data() + ehdr->e_shoff);
        auto *shstr = GetSectionData<char>(data, sections[ehdr->e_shstrndx]);
        ASSERT_NE(shstr, nullptr);
        auto shstrSize = sections[ehdr->e_shstrndx].sh_size;

        auto getSectionName = [shstr, shstrSize](const Shdr &section) {
            if (section.sh_name >= shstrSize) {
                return static_cast<const char *>(nullptr);
            }
            return shstr + section.sh_name;
        };

        const Shdr *gnuHash = nullptr;
        const Shdr *sysvHash = nullptr;
        const Shdr *dynstr = nullptr;
        const Shdr *dynsym = nullptr;
        const Shdr *dynamic = nullptr;
        // Locate sections used by the dynamic linker.
        for (size_t i = 0; i < ehdr->e_shnum; i++) {
            auto *name = getSectionName(sections[i]);
            ASSERT_NE(name, nullptr);
            if (strcmp(name, ".gnu.hash") == 0) {
                gnuHash = &sections[i];
            } else if (strcmp(name, ".hash") == 0) {
                sysvHash = &sections[i];
            } else if (strcmp(name, ".dynstr") == 0) {
                dynstr = &sections[i];
            } else if (strcmp(name, ".dynsym") == 0) {
                dynsym = &sections[i];
            } else if (strcmp(name, ".dynamic") == 0) {
                dynamic = &sections[i];
            }
        }

        ASSERT_NE(gnuHash, nullptr);
        EXPECT_EQ(gnuHash->sh_type, SHT_GNU_HASH);
        EXPECT_EQ(sysvHash, nullptr);
        ASSERT_NE(dynstr, nullptr);
        ASSERT_NE(dynsym, nullptr);
        ASSERT_NE(dynamic, nullptr);

        bool hasGnuHashDynamicTag = false;
        bool hasSysvHashDynamicTag = false;
        // Check that the dynamic section points to GNU hash, not SysV hash.
        auto *dynamicData = GetSectionData<Dyn>(data, *dynamic);
        ASSERT_NE(dynamicData, nullptr);
        ASSERT_EQ(dynamic->sh_size % sizeof(Dyn), 0U);
        for (size_t i = 0; i < dynamic->sh_size / sizeof(Dyn); i++) {
            hasGnuHashDynamicTag |= dynamicData[i].d_tag == DT_GNU_HASH;
            hasSysvHashDynamicTag |= dynamicData[i].d_tag == DT_HASH;
        }
        EXPECT_TRUE(hasGnuHashDynamicTag);
        EXPECT_FALSE(hasSysvHashDynamicTag);

        auto *strings = GetSectionData<char>(data, *dynstr);
        auto *symbols = GetSectionData<Sym>(data, *dynsym);
        ASSERT_NE(strings, nullptr);
        ASSERT_NE(symbols, nullptr);
        ASSERT_EQ(dynsym->sh_size % sizeof(Sym), 0U);
        ASSERT_GE(dynsym->sh_size / sizeof(Sym), entrySymbols_.size() + 1U);

        // Entry symbols must be the first non-null dynamic symbols.
        for (size_t i = 0; i < entrySymbols_.size(); i++) {
            auto &symbol = symbols[i + 1U];
            ASSERT_LT(symbol.st_name, dynstr->sh_size);
            EXPECT_STREQ(strings + symbol.st_name, entrySymbols_[i]);
        }
    }

    template <class T, class Shdr>
    static const T *GetSectionData(const std::vector<uint8_t> &data, const Shdr &section)
    {
        EXPECT_LE(section.sh_offset + section.sh_size, data.size());
        if (section.sh_offset + section.sh_size > data.size()) {
            return nullptr;
        }
        return reinterpret_cast<const T *>(data.data() + section.sh_offset);
    }

private:
    static constexpr std::array<const char *, 4U> entrySymbols_ {"code", "code_end", "aot", "aot_end"};
    const char *tmpFilePf_ = "test.pf";
    const char *cmdline_ = "cmdline";
    uint32_t method1Id_ = 42;
    uint32_t method2Id_ = 43;
    const std::string className_ {"Foo"};
    std::string methodName_ {className_ + "::method"};
    std::array<uint8_t, 4U> x86Add_ = {
        0x8dU, 0x04U, 0x37U,  // lea    eax,[rdi+rdi*1]
        0xc3U                 // ret
    };
    Class *klass_ {nullptr};
};

TEST_F(AotTestBuildAndCheck, BuildAndLoad)
{
    if (RUNTIME_ARCH == Arch::AARCH32) {
        // NOTE(compiler): for some reason dlopen cannot open aot file in qemu-arm
        return;
    }
    uint32_t tid = os::thread::GetCurrentThreadId();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::string tmpfile = helpers::string::Format("/tmp/tmpfile_%04x.pn", tid);
    const char *tmpfilePn = tmpfile.c_str();

    BuildAot(tmpfilePn, mem::GCType::STW_GC);
    CheckGnuHashAndEntrySymbols(tmpfilePn);
    AotManager aotManager;
    {
        auto res = aotManager.AddFile(tmpfilePn, nullptr, static_cast<uint32_t>(mem::GCType::STW_GC));
        ASSERT_TRUE(res) << res.Error();
    }
    DoChecks(&aotManager, tmpfilePn);
}

TEST_F(AotTestBuildAndCheck, BuildAndLoadAn)
{
    if (RUNTIME_ARCH == Arch::AARCH32) {
        // NOTE(compiler): for some reason dlopen cannot open aot file in qemu-arm
        return;
    }
    uint32_t tid = os::thread::GetCurrentThreadId();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::string tmpfile = helpers::string::Format("test.an", tid);
    const char *tmpfilePn = tmpfile.c_str();

    auto runtime = Runtime::GetCurrent();
    auto gcType = Runtime::GetGCType(runtime->GetOptions(), plugins::RuntimeTypeToLang(runtime->GetRuntimeType()));
    BuildAot(tmpfilePn, gcType);
    {
        auto res = FileManager::LoadAnFile(tmpfilePn);
        ASSERT_TRUE(res) << "Fail to load an file";
    }
    auto aotManager = Runtime::GetCurrent()->GetClassLinker()->GetAotManager();
    DoChecks(aotManager, tmpfilePn);
}

TEST_F(AotTestBuildAndCheck, LinkAotCodeForBoot)
{
    if (RUNTIME_ARCH == Arch::AARCH32) {
        // NOTE(compiler): for some reason dlopen cannot open aot file in qemu-arm
        return;
    }
    uint32_t tid = os::thread::GetCurrentThreadId();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::string tmpfile = helpers::string::Format("test.an", tid);
    const char *tmpfilePn = tmpfile.c_str();

    auto runtime = Runtime::GetCurrent();
    auto gcType = Runtime::GetGCType(runtime->GetOptions(), plugins::RuntimeTypeToLang(runtime->GetRuntimeType()));
    BuildAot(tmpfilePn, gcType);
    {
        auto res = FileManager::LoadAnFile(tmpfilePn);
        ASSERT_TRUE(res) << "Fail to load an file";
    }
    auto aotManager = Runtime::GetCurrent()->GetClassLinker()->GetAotManager();
    CheckLinkAotCodeForBoot(aotManager, tmpfilePn);
}

TEST_F(AotTestBuildAndCheck, LinkAotCodeForBootAotClassInvalid)
{
    if (RUNTIME_ARCH == Arch::AARCH32) {
        return;
    }
    uint32_t tid = os::thread::GetCurrentThreadId();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::string tmpfile = helpers::string::Format("test.an", tid);
    const char *tmpfilePn = tmpfile.c_str();

    auto runtime = Runtime::GetCurrent();
    auto gcType = Runtime::GetGCType(runtime->GetOptions(), plugins::RuntimeTypeToLang(runtime->GetRuntimeType()));
    BuildAot(tmpfilePn, gcType);
    {
        auto res = FileManager::LoadAnFile(tmpfilePn);
        ASSERT_TRUE(res) << "Fail to load an file";
    }
    auto aotManager = Runtime::GetCurrent()->GetClassLinker()->GetAotManager();
    CheckLinkAotCodeForBootAotClassInvalid(aotManager, tmpfilePn);
}

static void PaocSpecifyMethodsEmit(const TmpFile &pandaFname)
{
    auto source = R"(
        .record A {}
        .record B {}

        .function i32 A.f1() {
            ldai 10
            return
        }

        .function i32 B.f1() {
            ldai 20
            return
        }

        .function i32 A.f2() {
            ldai 10
            return
        }

        .function i32 B.f2() {
            ldai 20
            return
        }

        .function i32 B.f_overloaded() {
            ldai 20
            return
        }

        .function i32 B.f_overloaded(i32 a0) {
            lda a0
            return
        }

        .function i32 main() {
            ldai 0
            return
        }
    )";
    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    ASSERT_TRUE(pandasm::AsmEmitter::Emit(pandaFname.GetFileName(), res.Value()));
}

TEST_F(AotTest, PaocSpecifyMethods)
{
#ifndef PANDA_EVENTS_ENABLED
    GTEST_SKIP();
#endif

    // Test basic functionality only in host mode.
    if (RUNTIME_ARCH != Arch::X86_64) {
        return;
    }
    TmpFile pandaFname("test.pf");
    TmpFile paocOutputName("events-out.csv");

    PaocSpecifyMethodsEmit(pandaFname);

    {
        // paoc will try compiling all the methods from the panda-file that matches `--compiler-regex`
        auto res =
            os::exec::Exec(GetPaocFile(), "--paoc-panda-files", pandaFname.GetFileName(), "--compiler-regex", "B::f1",
                           "--paoc-mode=jit", "--events-output=csv", "--events-file", paocOutputName.GetFileName());
        ASSERT_TRUE(res) << "paoc failed with error: " << res.Error().ToString();
        ASSERT_EQ(res.Value(), 0U);

        std::ifstream infile(paocOutputName.GetFileName());
        std::regex rgx("Compilation,B::f1.*,COMPILED");
        size_t found = 0;
        for (std::string line; std::getline(infile, line);) {
            if (line.rfind("Compilation", 0U) == 0U) {
                ASSERT_TRUE(std::regex_match(line, rgx));
                found++;
            }
        }
        ASSERT_EQ(found, 1U);
    }
    {
        // Test `--compiler-regex-with-signature`:
        auto res = os::exec::Exec(GetPaocFile(), "--paoc-panda-files", pandaFname.GetFileName(),
                                  "--compiler-regex-with-signature", ".*B::f_overloaded\\(\\)", "--paoc-mode=jit",
                                  "--events-output=csv", "--events-file", paocOutputName.GetFileName());
        ASSERT_TRUE(res) << "paoc failed with error: " << res.Error().ToString();
        ASSERT_EQ(res.Value(), 0U);

        std::ifstream infile(paocOutputName.GetFileName());
        std::regex rgx("Compilation,B::f_overloaded.*,COMPILED");
        size_t found = 0;
        for (std::string line; std::getline(infile, line);) {
            if (line.rfind("Compilation", 0U) == 0U) {
                ASSERT_TRUE(std::regex_match(line, rgx));
                found++;
            }
        }
        ASSERT_EQ(found, 1U);
    }
}

static void PaocMultipleFilesEmitFirst(const TmpFile &pandaFname)
{
    auto source = R"(
        .function f64 main() {
            fldai.64 3.1415926
            return.64
        }
    )";

    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    ASSERT_TRUE(pandasm::AsmEmitter::Emit(pandaFname.GetFileName(), res.Value()));
}

static void PaocMultipleFilesEmitSecond(const TmpFile &pandaFname)
{
    auto source = R"(
        .record MyMath {
        }

        .function f64 MyMath.getPi() <static> {
            fldai.64 3.1415926
            return.64
        }
    )";

    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    ASSERT_TRUE(pandasm::AsmEmitter::Emit(pandaFname.GetFileName(), res.Value()));
}

TEST_F(AotTest, PaocMultipleFiles)
{
    if (RUNTIME_ARCH != Arch::X86_64) {
        GTEST_SKIP();
    }

    TmpFile aotFname("./test.an");
    TmpFile pandaFname1("test1.pf");
    TmpFile pandaFname2("test2.pf");

    PaocMultipleFilesEmitFirst(pandaFname1);
    PaocMultipleFilesEmitSecond(pandaFname2);

    {
        std::stringstream pandaFiles;
        pandaFiles << pandaFname1.GetFileName() << ',' << pandaFname2.GetFileName();
        auto res = os::exec::Exec(GetPaocFile(), "--paoc-panda-files", pandaFiles.str().c_str(), "--paoc-output",
                                  aotFname.GetFileName(), "--gc-type=epsilon", "--paoc-use-cha=false");
        ASSERT_TRUE(res) << "paoc failed with error: " << res.Error().ToString();
        ASSERT_EQ(res.Value(), 0U);
    }

    {
        AotManager aotManager;
        auto res = aotManager.AddFile(aotFname.GetFileName(), nullptr, static_cast<uint32_t>(mem::GCType::EPSILON_GC));
        ASSERT_TRUE(res) << res.Error();

        auto aotFile = aotManager.GetFile(aotFname.GetFileName());
        ASSERT_TRUE(aotFile);
        ASSERT_EQ(aotFile->GetFilesCount(), 2U);
    }
    RunAotdump(aotFname.GetFileName());
}

TEST_F(AotTest, PaocGcType)
{
    if (RUNTIME_ARCH != Arch::X86_64) {
        GTEST_SKIP();
    }

    TmpFile aotFname("./test.pn");
    TmpFile pandaFname("test.pf");

    {
        auto source = R"(
            .function f64 main() {
                fldai.64 3.1415926
                return.64
            }
        )";

        pandasm::Parser parser;
        auto res = parser.Parse(source);
        ASSERT_TRUE(res);
        ASSERT_TRUE(pandasm::AsmEmitter::Emit(pandaFname.GetFileName(), res.Value()));
    }

    {
        auto res = os::exec::Exec(GetPaocFile(), "--paoc-panda-files", pandaFname.GetFileName(), "--paoc-output",
                                  aotFname.GetFileName(), "--gc-type=epsilon", "--paoc-use-cha=false");
        ASSERT_TRUE(res) << "paoc failed with error: " << res.Error().ToString();
        ASSERT_EQ(res.Value(), 0U);
    }

    {
        // Wrong gc-type
        AotManager aotManager;
        auto res = aotManager.AddFile(aotFname.GetFileName(), nullptr, static_cast<uint32_t>(mem::GCType::STW_GC));
        ASSERT_FALSE(res) << res.Error();
        std::string expectedString = "Wrong AotHeader gc-type: epsilon vs stw";
        ASSERT_NE(res.Error().find(expectedString), std::string::npos);
    }

    {
        AotManager aotManager;
        auto res = aotManager.AddFile(aotFname.GetFileName(), nullptr, static_cast<uint32_t>(mem::GCType::EPSILON_GC));
        ASSERT_TRUE(res) << res.Error();

        auto aotFile = aotManager.GetFile(aotFname.GetFileName());
        ASSERT_TRUE(aotFile);
        ASSERT_EQ(aotFile->GetFilesCount(), 1U);
    }
    RunAotdump(aotFname.GetFileName());
}

TEST_F(AotTest, TryLoadAnFileFromLocationRejectsEscapedPrefix)
{
    if (RUNTIME_ARCH != Arch::X86_64) {
        GTEST_SKIP();
    }

    std::string anFileLocation = "/tmp";
    PandaString abcFilePrefix = "/../test";
    std::string pandaFileLocation = "test.pf";
    auto loaded = FileManager::TryLoadAnFileFromLocation(anFileLocation, abcFilePrefix, pandaFileLocation);
    ASSERT_FALSE(loaded);
}

TEST_F(AotTest, TryLoadAnFileFromLocationLoadsNormalPath)
{
    if (RUNTIME_ARCH != Arch::X86_64) {
        GTEST_SKIP();
    }

    TmpFile aotFname("./test.an");
    TmpFile pandaFname("./test.pf");

    auto source = R"(
        .function f64 main() {
            fldai.64 1.0
            return.64
        }
    )";

    {
        pandasm::Parser parser;
        auto res = parser.Parse(source);
        ASSERT_TRUE(res);
        ASSERT_TRUE(pandasm::AsmEmitter::Emit(pandaFname.GetFileName(), res.Value()));
    }

    {
        auto runtime = Runtime::GetCurrent();
        auto gcType = Runtime::GetGCType(runtime->GetOptions(), plugins::RuntimeTypeToLang(runtime->GetRuntimeType()));
        auto gcTypeName = "--gc-type=epsilon";
        if (gcType == mem::GCType::STW_GC) {
            gcTypeName = "--gc-type=stw";
        } else if (gcType == mem::GCType::G1_GC) {
            gcTypeName = "--gc-type=g1-gc";
        } else {
            ASSERT_TRUE(gcType == mem::GCType::EPSILON_GC || gcType == mem::GCType::EPSILON_G1_GC)
                << "Invalid GC type\n";
        }
        auto res = os::exec::Exec(GetPaocFile(), "--paoc-panda-files", pandaFname.GetFileName(), "--paoc-output",
                                  aotFname.GetFileName(), gcTypeName, "--paoc-use-cha=false");
        ASSERT_TRUE(res) << "paoc failed with error: " << res.Error().ToString();
        ASSERT_EQ(res.Value(), 0U);
    }

    std::string pandaFileLocation = os::GetAbsolutePath(pandaFname.GetFileName());
    ASSERT_FALSE(pandaFileLocation.empty());
    auto posStart = pandaFileLocation.find_last_of('/');
    auto posEnd = pandaFileLocation.find_last_of('.');
    ASSERT_NE(posStart, std::string::npos);
    ASSERT_NE(posEnd, std::string::npos);

    std::string anFileLocation = pandaFileLocation.substr(0, posStart);
    PandaString abcFilePrefix = PandaString(pandaFileLocation.substr(posStart, posEnd - posStart));
    auto loaded = FileManager::TryLoadAnFileFromLocation(anFileLocation, abcFilePrefix, pandaFileLocation);
    ASSERT_TRUE(loaded);

    auto aotManager = Runtime::GetCurrent()->GetClassLinker()->GetAotManager();
    ASSERT_TRUE(aotManager->GetFile(aotFname.GetFileName()) != nullptr);
}

TEST_F(AotTest, TryLoadAnFileFromLocationInvalidDirectoryRejected)
{
    if (RUNTIME_ARCH != Arch::X86_64) {
        GTEST_SKIP();
    }

    uint32_t tid = os::thread::GetCurrentThreadId();
    auto anFileLocation = helpers::string::Format("/tmp/an_missing_dir_%u", tid);
    PandaString abcFilePrefix = "/test";
    std::string pandaFileLocation = "test.pf";
    auto loaded = FileManager::TryLoadAnFileFromLocation(anFileLocation, abcFilePrefix, pandaFileLocation);
    ASSERT_FALSE(loaded);
}

TEST_F(AotTest, FileManagerLoadAbc)
{
    if (RUNTIME_ARCH != Arch::X86_64) {
        GTEST_SKIP();
    }

    TmpFile aotFname("./test.an");
    TmpFile pandaFname("./test.pf");

    {
        auto source = R"(
            .function f64 main() {
                fldai.64 3.1415926
                return.64
            }
        )";

        pandasm::Parser parser;
        auto res = parser.Parse(source);
        ASSERT_TRUE(res);
        ASSERT_TRUE(pandasm::AsmEmitter::Emit(pandaFname.GetFileName(), res.Value()));
    }

    {
        auto runtime = Runtime::GetCurrent();
        auto gcType = Runtime::GetGCType(runtime->GetOptions(), plugins::RuntimeTypeToLang(runtime->GetRuntimeType()));
        auto gcTypeName = "--gc-type=epsilon";
        if (gcType == mem::GCType::STW_GC) {
            gcTypeName = "--gc-type=stw";
        } else if (gcType == mem::GCType::G1_GC) {
            gcTypeName = "--gc-type=g1-gc";
        } else {
            ASSERT_TRUE(gcType == mem::GCType::EPSILON_GC || gcType == mem::GCType::EPSILON_G1_GC)
                << "Invalid GC type\n";
        }
        auto res = os::exec::Exec(GetPaocFile(), "--paoc-panda-files", pandaFname.GetFileName(), "--paoc-output",
                                  aotFname.GetFileName(), gcTypeName, "--paoc-use-cha=false");
        ASSERT_TRUE(res) << "paoc failed with error: " << res.Error().ToString();
        ASSERT_EQ(res.Value(), 0U);
    }

    {
        auto res = FileManager::LoadAbcFile(pandaFname.GetFileName(), panda_file::File::READ_ONLY);
        ASSERT_TRUE(res);
        auto aotManager = Runtime::GetCurrent()->GetClassLinker()->GetAotManager();
        auto aotFile = aotManager->GetFile(aotFname.GetFileName());
        ASSERT_TRUE(aotFile);
        ASSERT_EQ(aotFile->GetFilesCount(), 1U);
    }
    RunAotdump(aotFname.GetFileName());
}

static void PaocClustersWriteJson(const TmpFile &paocClusters)
{
    std::ofstream(paocClusters.GetFileName()) <<
        R"(
    {
        "clusters_map" :
        {
            "A::count" : ["unroll_enable"],
            "B::count2" : ["unroll_disable"],
            "_GLOBAL::main" : ["inline_disable", 1]
        },

        "compiler_options" :
        {
            "unroll_disable" :
            {
                "compiler-loop-unroll" : "false"
            },

            "unroll_enable" :
            {
                "compiler-loop-unroll" : "true",
                "compiler-loop-unroll-factor" : 42,
                "compiler-loop-unroll-inst-limit" : 850
            },

            "inline_disable" :
            {
                "compiler-inlining" : "false"
            }
        }
    }
    )";
}

static void PaocClustersEmit(const TmpFile &pandaFname)
{
    auto source = R"(
        .record A {}
        .record B {}

        .function i32 A.count() <static> {
            movi v1, 5
            ldai 0
        main_loop:
            jeq v1, main_ret
            addi 1
            jmp main_loop
        main_ret:
            return
        }

        .function i32 B.count() <static> {
            movi v1, 5
            ldai 0
        main_loop:
            jeq v1, main_ret
            addi 1
            jmp main_loop
        main_ret:
            return
        }

        .function i32 B.count2() <static> {
            movi v1, 5
            ldai 0
        main_loop:
            jeq v1, main_ret
            addi 1
            jmp main_loop
        main_ret:
            return
        }

        .function i32 main() {
            call.short A.count
            sta v0
            call.short B.count
            add2 v0
            call.short B.count2
            add2 v0
            return
        }
    )";

    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    ASSERT_TRUE(pandasm::AsmEmitter::Emit(pandaFname.GetFileName(), res.Value()));
}

TEST_F(AotTest, PaocClusters)
{
    // Test basic functionality only in host mode.
    if (RUNTIME_ARCH != Arch::X86_64) {
        return;
    }

    TmpFile paocClusters("clusters.json");
    PaocClustersWriteJson(paocClusters);

    TmpFile pandaFname("test.pf");
    PaocClustersEmit(pandaFname);

    {
        TmpFile compilerEvents("events.csv");
        auto res =
            os::exec::Exec(GetPaocFile(), "--paoc-panda-files", pandaFname.GetFileName(), "--paoc-clusters",
                           paocClusters.GetFileName(), "--compiler-loop-unroll-factor=7",
                           "--compiler-enable-events=true", "--compiler-events-path", compilerEvents.GetFileName());
        ASSERT_TRUE(res) << "paoc failed with error: " << res.Error().ToString();
        ASSERT_EQ(res.Value(), 0U);

        bool firstFound = false;
        bool secondFound = false;
        std::ifstream eventsFile(compilerEvents.GetFileName());

        std::regex rgxUnrollAppliedCluster("A::count,loop-unroll,.*,unroll_factor:42,.*");
        std::regex rgxUnrollRestoredDefault("B::count,loop-unroll,.*,unroll_factor:7,.*");

        for (std::string line; std::getline(eventsFile, line);) {
            if (line.rfind("loop-unroll") != std::string::npos) {
                if (!firstFound) {
                    // Check that the cluster is applied:
                    ASSERT_TRUE(std::regex_match(line, rgxUnrollAppliedCluster));
                    firstFound = true;
                    continue;
                }
                ASSERT_FALSE(secondFound);
                // Check that the option is restored:
                ASSERT_TRUE(std::regex_match(line, rgxUnrollRestoredDefault));
                secondFound = true;
            }
        }
        ASSERT_TRUE(firstFound && secondFound);
    }
}

static void PandaFilesEmitFirst(const TmpFile &pandaFname)
{
    auto source = R"(
        .record Z {}
        .function i32 Z.zoo() <static> {
            ldai 45
            return
        }
    )";

    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    ASSERT_TRUE(pandasm::AsmEmitter::Emit(pandaFname.GetFileName(), res.Value()));
}

static void PandaFilesEmitSecond(const TmpFile &pandaFname)
{
    auto source = R"(
        .record Z <external>
        .function i32 Z.zoo() <external, static>
        .record X {}
        .function i32 X.main() {
            call.short Z.zoo
            return
        }
    )";

    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    ASSERT_TRUE(pandasm::AsmEmitter::Emit(pandaFname.GetFileName(), res.Value()));
}

TEST_F(AotTest, PandaFiles)
{
#ifndef PANDA_EVENTS_ENABLED
    GTEST_SKIP();
#endif

    if (RUNTIME_ARCH != Arch::X86_64) {
        GTEST_SKIP();
    }

    TmpFile aotFname("./test.an");
    TmpFile pandaFname1("test1.pf");
    TmpFile pandaFname2("test2.pf");
    TmpFile paocOutputName("events-out.csv");

    PandaFilesEmitFirst(pandaFname1);
    PandaFilesEmitSecond(pandaFname2);

    {
        std::stringstream pandaFiles;
        pandaFiles << pandaFname1.GetFileName() << ',' << pandaFname2.GetFileName();
        auto res = os::exec::Exec(GetPaocFile(), "--paoc-panda-files", pandaFname2.GetFileName(), "--panda-files",
                                  pandaFname1.GetFileName(), "--events-output=csv", "--events-file",
                                  paocOutputName.GetFileName());
        ASSERT_TRUE(res) << "paoc failed with error: " << res.Error().ToString();
        ASSERT_EQ(res.Value(), 0U);

        std::ifstream infile(paocOutputName.GetFileName());
        // Inlining attempt proofs that Z::zoo was available to inline
        std::regex rgx("Inline,.*Z::zoo.*");
        bool inlineAttempt = false;
        for (std::string line; std::getline(infile, line);) {
            inlineAttempt |= std::regex_match(line, rgx);
        }
        ASSERT_TRUE(inlineAttempt);
    }
}

TEST_F(AotTest, PandaZipFile)
{
    if (RUNTIME_ARCH != Arch::X86_64) {
        GTEST_SKIP();
    }

    std::vector<uint8_t> pfData {};
    {
        pandasm::Parser p;

        auto source = R"(
            .record Z {}
            .function i32 Z.zoo() <static> {
            ldai 45
            return
            }
        )";

        std::string srcFilename = "src.pa";
        auto res = p.Parse(source, srcFilename);
        ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);

        auto pf = pandasm::AsmEmitter::Emit(res.Value());
        ASSERT_NE(pf, nullptr);

        const auto headerPtr = reinterpret_cast<const uint8_t *>(pf->GetHeader());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        pfData.assign(headerPtr, headerPtr + pf->GetHeader()->fileSize);
    }

    const char *archivename = "__TestPaocOpenZipPandaFile__.zip";
    const char *pandaFilename = "test.abc";
    const char *filename = "__TestPaocOpenZipPandaFile__.zip/test.abc";

    int ret = CreateOrAddFileIntoZip(archivename, pandaFilename, &pfData, APPEND_STATUS_CREATE, Z_NO_COMPRESSION);
    if (ret != 0) {
        ASSERT_EQ(1, 0) << "CreateOrAddFileIntoZip failed!";
        return;
    }

    {
        auto res = os::exec::Exec(GetPaocFile(), "--paoc-panda-files", filename, "--paoc-zip-panda-file", pandaFilename,
                                  "--paoc-location", "./");
        ASSERT_TRUE(res) << "paoc failed with error: " << res.Error().ToString();
        ASSERT_EQ(res.Value(), 0U);
    }
}

static void EmitAndLoadPandaFile(ClassLinker *classLinker, const char *source, const TmpFile *pandaFname)
{
    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    std::unique_ptr<const panda_file::File> pf = nullptr;
    if (pandaFname != nullptr) {
        ASSERT_TRUE(pandasm::AsmEmitter::Emit(pandaFname->GetFileName(), res.Value()));
        pf = panda_file::File::Open(pandaFname->GetFileName());
    } else {
        pf = pandasm::AsmEmitter::Emit(res.Value());
    }
    ASSERT_TRUE(pf);
    classLinker->AddPandaFile(std::move(pf), nullptr);
}

static void UpdateClassContext()
{
    auto classLinker = Runtime::GetCurrent()->GetClassLinker();
    // Build full context string from all files (like runtime does)
    auto fullContextStr = classLinker->GetClassContextForAot(true);

    // Split context like runtime does - find last delimiter, everything after is app context
    auto aotManager = classLinker->GetAotManager();
    auto lastDelimIdx = fullContextStr.find_last_of(compiler::AotClassContextCollector::DELIMETER);
    if (lastDelimIdx == std::string::npos) {
        // Only one file - treat as app context
        aotManager->SetAppClassContext(fullContextStr, true);
        aotManager->SetBootClassContext("", true);
    } else {
        // Split: everything after last delimiter is app context
        aotManager->SetAppClassContext(fullContextStr.substr(lastDelimIdx + 1), true);
        aotManager->SetBootClassContext(fullContextStr.substr(0, lastDelimIdx), true);
    }
}

static void CreateFilesForTestWithInMemoryFile(const TmpFile &pandaFname1, const TmpFile &pandaFname2)
{
    auto classLinker = Runtime::GetCurrent()->GetClassLinker();

    {
        auto source = R"(
            .record B {}

            .function i32 B.f1() {
                ldai 20
                return
            }
        )";
        EmitAndLoadPandaFile(classLinker, source, &pandaFname1);
    }

    // Add the in-memory file, it shall not change indices
    {
        auto source = R"(
            .record M {}

            .function i32 M.f1() {
                ldai 10
                return
            }
        )";
        EmitAndLoadPandaFile(classLinker, source, nullptr);
    }

    {
        auto source = R"(
            .record A {}

            .function i32 A.f1() {
                ldai 10
                return
            }
        )";
        EmitAndLoadPandaFile(classLinker, source, &pandaFname2);
    }
}

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class AotCompilerTest : public ::testing::Test {
public:
    AotCompilerTest()
    {
        RuntimeOptions options;
        auto execPath = ark::os::file::File::GetExecutablePath();
        pandaStdLib_ = ark::os::GetAbsolutePath(execPath.Value() + "/../pandastdlib/arkstdlib.abc");
        options.SetBootPandaFiles({pandaStdLib_});
        options.SetLoadRuntimes({"core"});
        options.SetArkAot(true);
        options.SetHeapSizeLimit(50_MB);  // NOLINT(readability-magic-numbers)
        options.SetGcType("epsilon");
        Runtime::Create(options);
    }

    ~AotCompilerTest() override
    {
        Runtime::Destroy();
    }

    auto GetStdLibPath()
    {
        return pandaStdLib_;
    }

private:
    std::string pandaStdLib_;
};

TEST_F(AotCompilerTest, NoMemoryFilesInAnFile)
{
    if (RUNTIME_ARCH != Arch::X86_64) {
        GTEST_SKIP();
    }

    TmpFile pandaFname1("test1.abc");
    TmpFile pandaFname2("test2.abc");

    auto classLinker = Runtime::GetCurrent()->GetClassLinker();
    auto aotManager = classLinker->GetAotManager();

    // Fisrt file is stdlib (index 0)
    int index = 0;
    ASSERT_EQ(aotManager->GetPandaFileSnapshotIndex(GetStdLibPath()), index++);

    CreateFilesForTestWithInMemoryFile(pandaFname1, pandaFname2);

    ASSERT_EQ(aotManager->GetPandaFileSnapshotIndex(ark::os::GetAbsolutePath(pandaFname1.GetFileName())), index++);
    ASSERT_EQ(aotManager->GetPandaFileSnapshotIndex(ark::os::GetAbsolutePath(pandaFname2.GetFileName())), index++);

    UpdateClassContext();

    // Verify that the in-memory file does not appear in the class context strings
    auto finalContext = aotManager->GetBootClassContext() + ":" + aotManager->GetAppClassContext();
    size_t start = 0;
    size_t end;
    while ((end = finalContext.find(':', start)) != std::string::npos) {
        size_t hashPos = finalContext.find('*', start);
        if (hashPos != std::string::npos && hashPos < end) {
            auto filename = finalContext.substr(start, hashPos - start);
            ASSERT_FALSE(filename.empty()) << "Empty filename found in final context string";
        }
        start = end + 1;
    }
    // Check the last entry
    size_t hashPos = finalContext.find('*', start);
    if (hashPos != std::string::npos) {
        auto filename = finalContext.substr(start, hashPos - start);
        ASSERT_FALSE(filename.empty()) << "Empty filename found in final context string";
    }
}
// Test: When --load-in-boot=false (default, e.g. appspawn), boot files should NOT be split into app context.
// This verifies the fix for the issue where appspawn (with --load-in-boot=false and --panda-files empty)
// incorrectly moved the last boot ABC into appClassContext_.
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class BootContextSplitTest : public ::testing::TestWithParam<bool> {
public:
    BootContextSplitTest()
    {
        RuntimeOptions options;
        auto execPath = ark::os::file::File::GetExecutablePath();
        pandaStdLib_ = ark::os::GetAbsolutePath(execPath.Value() + "/../pandastdlib/arkstdlib.abc");
        options.SetBootPandaFiles({pandaStdLib_});
        options.SetLoadRuntimes({"core"});
        options.SetArkAot(true);
        options.SetHeapSizeLimit(50_MB);  // NOLINT(readability-magic-numbers)
        options.SetGcType("epsilon");
        // --panda-files is empty (not set), simulating appspawn or standalone with --load-in-boot
        options.SetLoadInBoot(GetParam());
        Runtime::Create(options);
    }

    ~BootContextSplitTest() override
    {
        Runtime::Destroy();
    }

private:
    std::string pandaStdLib_;
};

// When --load-in-boot=false (appspawn scenario): all boot files stay in boot context, app context is empty.
// When --load-in-boot=true (standalone scenario): the last boot file is split into app context.
TEST_P(BootContextSplitTest, BootContextNotSplitWithoutLoadInBoot)
{
    auto *aotManager = Runtime::GetCurrent()->GetClassLinker()->GetAotManager();
    auto bootCtx = aotManager->GetBootClassContext();
    auto appCtx = aotManager->GetAppClassContext();

    if (GetParam()) {
        // --load-in-boot=true: last boot file should be moved to app context.
        // With only one boot file (arkstdlib.abc), boot context becomes empty after the split.
        ASSERT_FALSE(appCtx.empty()) << "With --load-in-boot=true, app context should contain the last boot file";
    } else {
        // --load-in-boot=false: no splitting, app context should be empty
        ASSERT_TRUE(appCtx.empty()) << "With --load-in-boot=false, app context should be empty, but got: " << appCtx;
        ASSERT_FALSE(bootCtx.empty()) << "All boot files should remain in boot context";
    }
}

INSTANTIATE_TEST_SUITE_P(LoadInBootGuard, BootContextSplitTest, ::testing::Values(false, true));

// Many classes and methods to ensure compilation takes measurable time
static constexpr auto MULTI_METHODS_SOURCE = R"(
    .record A {}
    .record B {}
    .record C {}
    .record D {}

    .function i32 A.method1() {
        ldai 1
        return
    }
    .function i32 A.method2() {
        ldai 2
        return
    }
    .function i32 A.method3() {
        ldai 3
        return
    }
    .function i32 B.method1() {
        ldai 4
        return
    }
    .function i32 B.method2() {
        ldai 5
        return
    }
    .function i32 B.method3() {
        ldai 6
        return
    }
    .function i32 C.method1() {
        ldai 7
        return
    }
    .function i32 C.method2() {
        ldai 8
        return
    }
    .function i32 C.method3() {
        ldai 9
        return
    }
    .function i32 D.method1() {
        ldai 10
        return
    }
    .function i32 D.method2() {
        ldai 11
        return
    }
    .function i32 D.method3() {
        ldai 12
        return
    }
)";

static void PaocCompilationLimitEmit(const TmpFile &pandaFname)
{
    pandasm::Parser parser;
    auto res = parser.Parse(MULTI_METHODS_SOURCE);
    ASSERT_TRUE(res);
    ASSERT_TRUE(pandasm::AsmEmitter::Emit(pandaFname.GetFileName(), res.Value()));
}

static size_t CountCompiledMethodsInEvents(const char *eventsFile)
{
    std::ifstream infile(eventsFile);
    std::regex rgx("Compilation,.*,COMPILED");
    size_t count = 0;
    for (std::string line; std::getline(infile, line);) {
        if (std::regex_match(line, rgx)) {
            count++;
        }
    }
    return count;
}

TEST_F(AotTest, PaocCompilationLimitNoLimit)
{
#ifndef PANDA_EVENTS_ENABLED
    GTEST_SKIP();
#endif
    if (RUNTIME_ARCH == Arch::AARCH32) {
        GTEST_SKIP() << "AOT isn't supported on Aarch32";
    }

    TmpFile pandaFname("compilation_limit_test.abc");
    TmpFile aotFname("./compilation_limit_test.an");
    TmpFile eventsFile("compilation_limit_events.csv");

    PaocCompilationLimitEmit(pandaFname);

    // With no time limit (default 0), all methods should be compiled
    auto res = os::exec::Exec(GetPaocFile(), "--paoc-panda-files", pandaFname.GetFileName(), "--paoc-output",
                              aotFname.GetFileName(), "--paoc-compilation-limit=0", "--events-output=csv",
                              "--events-file", eventsFile.GetFileName());
    ASSERT_TRUE(res) << "paoc failed with error: " << res.Error().ToString();
    ASSERT_EQ(res.Value(), 0U);

    constexpr size_t totalMethods = 12;
    size_t compiled = CountCompiledMethodsInEvents(eventsFile.GetFileName());
    ASSERT_EQ(compiled, totalMethods) << "All methods should be compiled when no time limit is set";

    RunAotdump(aotFname.GetFileName());
}

TEST_F(AotTest, PaocCompilationLimitWithTimeout)
{
#ifndef PANDA_EVENTS_ENABLED
    GTEST_SKIP();
#endif
    if (RUNTIME_ARCH == Arch::AARCH32) {
        GTEST_SKIP() << "AOT isn't supported on Aarch32";
    }

    TmpFile pandaFname("compilation_limit_timeout_test.abc");
    TmpFile aotFname("./compilation_limit_timeout_test.an");
    TmpFile eventsFile("compilation_limit_events.csv");

    PaocCompilationLimitEmit(pandaFname);

    // With 1ms time limit, compilation should stop early (or at least succeed without error)
    auto res = os::exec::Exec(GetPaocFile(), "--paoc-panda-files", pandaFname.GetFileName(), "--paoc-output",
                              aotFname.GetFileName(), "--paoc-compilation-limit=1", "--events-output=csv",
                              "--events-file", eventsFile.GetFileName(), "--compiler-ignore-failures=false");
    ASSERT_TRUE(res) << "paoc failed with error: " << res.Error().ToString();
    // Time-limited compilation should return success (0)
    ASSERT_EQ(res.Value(), 0U) << "Time-limited compilation should not return an error";

    // Check that compilation stopped due to set time limit
    std::ifstream infile(eventsFile.GetFileName());
    size_t count = 0;
    for (std::string line; std::getline(infile, line);) {
        if (line.find("Compilation time limit (1ms) exceeded, stopping compilation") != std::string::npos) {
            count++;
        }
    }
    ASSERT_EQ(count, 1U);
}

TEST_F(AotTest, PaocCompilationLimitPartialAotValid)
{
#ifndef PANDA_EVENTS_ENABLED
    GTEST_SKIP();
#endif
    if (RUNTIME_ARCH == Arch::AARCH32) {
        GTEST_SKIP() << "AOT isn't supported on Aarch32";
    }

    TmpFile pandaFname("compilation_limit_partial_test.abc");
    TmpFile aotFname("./compilation_limit_partial_test.an");
    TmpFile eventsFile("compilation_limit_partial_events.csv");

    PaocCompilationLimitEmit(pandaFname);

    // Use enough time limit that allows some but potentially not all methods to compile.
    auto res = os::exec::Exec(GetPaocFile(), "--paoc-panda-files", pandaFname.GetFileName(), "--paoc-output",
                              aotFname.GetFileName(), "--paoc-compilation-limit=1000", "--events-output=csv",
                              "--events-file", eventsFile.GetFileName());
    ASSERT_TRUE(res) << "paoc failed with error: " << res.Error().ToString();
    ASSERT_EQ(res.Value(), 0U);

    size_t compiled = CountCompiledMethodsInEvents(eventsFile.GetFileName());
    ASSERT_GT(compiled, 0U) << "At least some methods should be compiled within 1000ms";
}

TEST_F(AotTest, PaocCompilationLimitLargeTimeout)
{
#ifndef PANDA_EVENTS_ENABLED
    GTEST_SKIP();
#endif
    if (RUNTIME_ARCH == Arch::AARCH32) {
        GTEST_SKIP() << "AOT isn't supported on Aarch32";
    }

    TmpFile pandaFname("compilation_limit_large_test.abc");
    TmpFile aotFname("./compilation_limit_large_test.an");
    TmpFile eventsFile("compilation_limit_large_events.csv");

    PaocCompilationLimitEmit(pandaFname);

    // With a very large time limit, all methods should compile (same as no limit)
    auto res = os::exec::Exec(GetPaocFile(), "--paoc-panda-files", pandaFname.GetFileName(), "--paoc-output",
                              aotFname.GetFileName(), "--paoc-compilation-limit=60000", "--events-output=csv",
                              "--events-file", eventsFile.GetFileName(), "--compiler-ignore-failures=false");
    ASSERT_TRUE(res) << "paoc failed with error: " << res.Error().ToString();
    ASSERT_EQ(res.Value(), 0U);

    constexpr size_t totalMethods = 12;
    size_t compiled = CountCompiledMethodsInEvents(eventsFile.GetFileName());
    ASSERT_EQ(compiled, totalMethods) << "All methods should be compiled with a large time limit";
}

// NOLINTEND(readability-magic-numbers)

TEST_F(AotTest, PaocWriteToAnFd)
{
    if (RUNTIME_ARCH == Arch::AARCH32) {
        GTEST_SKIP() << "AOT isn't supported on Aarch32";
    }
    TmpFile pandaFname("test_anfd.pf");
    TmpFile aotFname("./test_anfd.an");

    auto source = R"(
        .function void f1() {
            return.void
        }
        .function void f2() {
            return.void
        }
    )";
    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    ASSERT_TRUE(pandasm::AsmEmitter::Emit(pandaFname.GetFileName(), res.Value()));

    // Open a file descriptor for the output .an file
    int anFd = open(aotFname.GetFileName(), O_RDWR | O_CREAT | O_TRUNC, 0644);
    ASSERT_GE(anFd, 0);

    // Run paoc with --paoc-an-fd
    auto execRes = os::exec::Exec(GetPaocFile(), "--paoc-panda-files", pandaFname.GetFileName(), "--paoc-output",
                                  aotFname.GetFileName(), "--paoc-an-fd", std::to_string(anFd).c_str());
    close(anFd);
    ASSERT_TRUE(execRes) << "paoc failed with error: " << execRes.Error().ToString();
    ASSERT_EQ(execRes.Value(), 0U) << "paoc returned error code: " << execRes.Value();

    // Verify the .an file was written correctly (should be a valid ELF)
    auto res2 = os::exec::Exec(GetAotdumpFile(), "--show-code=disasm", aotFname.GetFileName());
    ASSERT_TRUE(res2) << "aotdump failed: " << res2.Error().ToString();
    ASSERT_EQ(res2.Value(), 0U) << "aotdump return error: " << res2.Value();

    unlink(aotFname.GetFileName());
}

TEST_F(AotTest, PaocWriteToInvalidFd)
{
    if (RUNTIME_ARCH == Arch::AARCH32) {
        GTEST_SKIP() << "AOT isn't supported on Aarch32";
    }
    TmpFile pandaFname("test_invalid_fd.pf");
    TmpFile aotFname("./test_invalid_fd.an");

    auto source = R"(
        .function void f1() {
            return.void
        }
    )";
    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    ASSERT_TRUE(pandasm::AsmEmitter::Emit(pandaFname.GetFileName(), res.Value()));

    // Open the .pf file read-only and pass as --paoc-an-fd.
    // The fd is valid (lseek succeeds) but not writable (ftruncate/write fail),
    // so paoc should detect the write failure and return non-zero.
    int readOnlyFd = open(pandaFname.GetFileName(), O_RDONLY);
    ASSERT_GE(readOnlyFd, 0);

    auto execRes = os::exec::Exec(GetPaocFile(), "--paoc-panda-files", pandaFname.GetFileName(), "--paoc-output",
                                  aotFname.GetFileName(), "--paoc-an-fd", std::to_string(readOnlyFd).c_str());
    close(readOnlyFd);
    ASSERT_TRUE(execRes) << "paoc execution failed: " << execRes.Error().ToString();
    // Write to a read-only fd should fail — paoc must return non-zero exit code
    ASSERT_NE(execRes.Value(), 0U) << "paoc should fail when --paoc-an-fd is a read-only fd";

    unlink(aotFname.GetFileName());
}

TEST_F(AotTest, PaocWriteToAnFdAndPathProduceSameOutput)
{
    if (RUNTIME_ARCH == Arch::AARCH32) {
        GTEST_SKIP() << "AOT isn't supported on Aarch32";
    }
    TmpFile pandaFname("test_anfd_cmp.pf");
    TmpFile aotFnameFd("./test_anfd_cmp_fd.an");
    TmpFile aotFnamePath("./test_anfd_cmp_path.an");

    auto source = R"(
        .function void f1() {
            return.void
        }
    )";
    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    ASSERT_TRUE(pandasm::AsmEmitter::Emit(pandaFname.GetFileName(), res.Value()));

    // Compile with fd
    int anFd = open(aotFnameFd.GetFileName(), O_RDWR | O_CREAT | O_TRUNC, 0644);
    ASSERT_GE(anFd, 0);
    auto execRes1 = os::exec::Exec(GetPaocFile(), "--paoc-panda-files", pandaFname.GetFileName(), "--paoc-output",
                                   aotFnameFd.GetFileName(), "--paoc-an-fd", std::to_string(anFd).c_str());
    close(anFd);
    ASSERT_TRUE(execRes1);
    ASSERT_EQ(execRes1.Value(), 0U);

    // Compile with path (default)
    auto execRes2 = os::exec::Exec(GetPaocFile(), "--paoc-panda-files", pandaFname.GetFileName(), "--paoc-output",
                                   aotFnamePath.GetFileName());
    ASSERT_TRUE(execRes2);
    ASSERT_EQ(execRes2.Value(), 0U);

    // Compare file sizes (ELF content should be functionally equivalent)
    struct stat stFd;
    struct stat stPath;
    ASSERT_EQ(stat(aotFnameFd.GetFileName(), &stFd), 0);
    ASSERT_EQ(stat(aotFnamePath.GetFileName(), &stPath), 0);
    ASSERT_EQ(stFd.st_size, stPath.st_size) << "FD and path output sizes differ";

    unlink(aotFnameFd.GetFileName());
    unlink(aotFnamePath.GetFileName());
}

TEST_F(AotTest, PaocLoadZipFromHapFd)
{
    if (RUNTIME_ARCH == Arch::AARCH32) {
        GTEST_SKIP() << "AOT isn't supported on Aarch32";
    }
    // Create a zip archive containing a panda file, simulating a HAP
    std::vector<uint8_t> pfData;
    {
        pandasm::Parser parser;
        auto source = R"(
            .function void f1() {
                return.void
            }
        )";
        auto res = parser.Parse(source, "test.abc");
        ASSERT_TRUE(res);
        auto pf = pandasm::AsmEmitter::Emit(res.Value());
        ASSERT_NE(pf, nullptr);
        const auto headerPtr = reinterpret_cast<const uint8_t *>(pf->GetHeader());
        pfData.assign(headerPtr, headerPtr + pf->GetHeader()->fileSize);
    }

    const char *archivename = "__TestPaocHapFd__.zip";
    const char *pandaFilename = "test.abc";
    int ret = CreateOrAddFileIntoZip(archivename, pandaFilename, &pfData, APPEND_STATUS_CREATE, Z_NO_COMPRESSION);
    ASSERT_EQ(ret, 0);

    // Open the zip file and pass its fd to paoc via --paoc-hap-fd
    int hapFd = open(archivename, O_RDONLY);
    ASSERT_GE(hapFd, 0);

    std::string hapFdArg = "--paoc-hap-fd=" + std::to_string(hapFd);
    std::string zipPath = std::string(archivename) + "/" + pandaFilename;
    auto execRes = os::exec::Exec(GetPaocFile(), "--paoc-panda-files", zipPath.c_str(), "--paoc-zip-panda-file",
                                  pandaFilename, "--paoc-location", "./", hapFdArg.c_str());
    close(hapFd);
    ASSERT_TRUE(execRes) << "paoc failed with error: " << execRes.Error().ToString();
    ASSERT_EQ(execRes.Value(), 0U) << "paoc returned error with hapFd";

    unlink(archivename);
}

TEST_F(AotTest, PaocFullFdPipeline)
{
    if (RUNTIME_ARCH == Arch::AARCH32) {
        GTEST_SKIP() << "AOT isn't supported on Aarch32";
    }
    std::vector<uint8_t> pfData;
    const char *archivename = "__TestFullFdPipeline__.zip";
    const char *pandaFilename = "test.abc";
    CreateTestZip(archivename, pandaFilename, pfData);

    int hapFd = open(archivename, O_RDONLY);
    ASSERT_GE(hapFd, 0);
    int anFd = memfd_create("an_file", MFD_ALLOW_SEALING);
    ASSERT_GE(anFd, 0);

    std::string hapFdArg = "--paoc-hap-fd=" + std::to_string(hapFd);
    std::string anFdArg = "--paoc-an-fd=" + std::to_string(anFd);
    TmpFile aotFname("./test_full_fd.an");
    std::string zipPath = std::string(archivename) + "/" + pandaFilename;
    auto execRes = os::exec::Exec(GetPaocFile(), "--paoc-panda-files", zipPath.c_str(), "--paoc-zip-panda-file",
                                  pandaFilename, "--paoc-location", "./", "--paoc-output", aotFname.GetFileName(),
                                  hapFdArg.c_str(), anFdArg.c_str());
    close(hapFd);
    ASSERT_TRUE(execRes) << "paoc failed with error: " << execRes.Error().ToString();
    ASSERT_EQ(execRes.Value(), 0U) << "paoc returned error in full fd pipeline";

    SpliceMemfdToDisk(anFd, aotFname.GetFileName());
    close(anFd);
    RunAotdump(aotFname.GetFileName());
    unlink(archivename);
}

}  // namespace ark::compiler

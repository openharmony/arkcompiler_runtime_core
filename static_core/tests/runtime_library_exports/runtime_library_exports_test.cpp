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

#include <dlfcn.h>
#include <sys/stat.h>
#include <cstdlib>
#include <string>
#include <gtest/gtest.h>

namespace ark::test {
struct ExportCase {
    const char *library;
    const char *symbol;
};

class RuntimeLibraryExportsTest : public testing::TestWithParam<ExportCase> {
protected:
    void SetUp() override
    {
        const char *directory = std::getenv("RUNTIME_EXPORTS_LIB_DIR");
        ASSERT_NE(directory, nullptr) << "Set RUNTIME_EXPORTS_LIB_DIR to the absolute directory of tested SOs";
        ASSERT_EQ(directory[0], '/') << "An absolute directory is required";
        path_ = std::string(directory) + "/" + GetParam().library;
        handle_ = dlopen(path_.c_str(), RTLD_NOW | RTLD_LOCAL);
        ASSERT_NE(handle_, nullptr) << path_ << ": " << dlerror();
    }

    void TearDown() override
    {
        if (handle_ != nullptr) {
            EXPECT_EQ(dlclose(handle_), 0);
        }
    }

    bool IsOwnedByLibrary(void *symbol)
    {
        Dl_info info {};
        if (dladdr(symbol, &info) == 0 || info.dli_fname == nullptr) {
            ADD_FAILURE() << "Cannot determine symbol owner";
            return false;
        }
        struct stat expected {};
        struct stat actual {};
        if (stat(path_.c_str(), &expected) != 0 || stat(info.dli_fname, &actual) != 0) {
            ADD_FAILURE() << "Cannot stat symbol owner: " << info.dli_fname;
            return false;
        }
        return expected.st_dev == actual.st_dev && expected.st_ino == actual.st_ino;
    }

    void *handle_ {nullptr};
    std::string path_;
};

class RequiredExportsTest : public RuntimeLibraryExportsTest {};
class CfiExportsTest : public RuntimeLibraryExportsTest {};
class HiddenExportsTest : public RuntimeLibraryExportsTest {};

TEST_P(RequiredExportsTest, DefinedByRequestedLibrary)
{
    dlerror();
    void *symbol = dlsym(handle_, GetParam().symbol);
    const char *error = dlerror();
    ASSERT_EQ(error, nullptr) << path_ << ": " << GetParam().symbol << ": " << (error ? error : "");
    ASSERT_NE(symbol, nullptr) << GetParam().symbol;
    EXPECT_TRUE(IsOwnedByLibrary(symbol)) << GetParam().symbol << " resolved from a dependency";
}

TEST_P(CfiExportsTest, CheckEntryDefinedByRequestedLibrary)
{
    dlerror();
    void *symbol = dlsym(handle_, GetParam().symbol);
    const char *error = dlerror();
    ASSERT_EQ(error, nullptr) << path_ << ": " << (error ? error : "");
    ASSERT_NE(symbol, nullptr);
    EXPECT_TRUE(IsOwnedByLibrary(symbol)) << "__cfi_check resolved from a dependency";
}

TEST_P(HiddenExportsTest, NotExportedByRequestedLibrary)
{
    dlerror();
    void *symbol = dlsym(handle_, GetParam().symbol);
    const char *error = dlerror();
    if (error == nullptr && symbol != nullptr) {
        // A dependency may export this name; only the tested SO must hide it.
        EXPECT_FALSE(IsOwnedByLibrary(symbol)) << path_ << " exports " << GetParam().symbol;
    }
}

// Independent contract samples, not generated from the version scripts.
// ANI is currently an empty shim; do not assert ANI_CreateVM belongs to it.
INSTANTIATE_TEST_SUITE_P(
    PublicApi, RequiredExportsTest,
    testing::Values(
        ExportCase {"libani_helpers.z.so", "_ZN5arkts13ani_signature16SignatureBuilder10AddBooleanEv"},
        ExportCase {"libani_helpers.z.so", "_ZN5arkts19concurrency_helpers11GetWorkerIdEP9__ani_env"},
        ExportCase {"libani_helpers.z.so", "CreateExternalArrayBuffer"},
        ExportCase {"libani_helpers.z.so", "CreateFinalizableArrayBuffer"},
        ExportCase {"libani_helpers.z.so", "DetachArrayBuffer"},
        ExportCase {"libani_helpers.z.so", "IsDetachedArrayBuffer"},
        ExportCase {"libani_helpers.z.so", "arkts_ani_scope_open"},
        ExportCase {"libani_helpers.z.so", "arkts_ani_scope_close_n"},
        ExportCase {"libani_helpers.z.so", "arkts_napi_scope_open"},
        ExportCase {"libani_helpers.z.so", "arkts_napi_scope_close_n"},
        ExportCase {"libani_helpers.z.so", "arkts_esvalue_unwrap"},
        ExportCase {"libani_helpers.z.so", "hybridgref_create_from_ani"},
        ExportCase {"libani_helpers.z.so", "hybridgref_create_from_napi"},
        ExportCase {"libani_helpers.z.so", "hybridgref_delete_from_ani"},
        ExportCase {"libani_helpers.z.so", "hybridgref_delete_from_napi"},
        ExportCase {"libani_helpers.z.so", "hybridgref_get_esvalue"},
        ExportCase {"libani_helpers.z.so", "hybridgref_get_napi_value"},
        ExportCase {"libarkaotmanager.so", "CallStaticPltResolver"},
        ExportCase {"libarkencoder.so", "_ZN3ark8compiler7Encoder6CreateEPNS_15ArenaAllocatorTILb0EEENS_4ArchEbb"},
        ExportCase {"libarkencoder.so", "_ZN3ark8compiler7Encoder11CreateLabelEv"},
        ExportCase {"libarkencoder.so", "_ZN3ark8compiler7Encoder9BindLabelEm"},
        ExportCase {"libarkencoder.so", "_ZTVN3ark8compiler7EncoderE"},
        ExportCase {"libarktscompiler.so", "_ZN3ark8compiler9g_optionsE"},
        ExportCase {"libarktscompiler.so", "_ZN3ark8compiler8RegAllocEPNS0_5GraphE"},
        ExportCase {"libarktscompiler.so", "_ZN3ark8compiler25SwapOperandsConditionCodeENS0_13ConditionCodeE"}));

// clang-format off
INSTANTIATE_TEST_SUITE_P(
    CrossDsoCfi, CfiExportsTest,
    testing::Values(
        ExportCase {"libani.z.so", "__cfi_check"},
        ExportCase {"libani_helpers.z.so", "__cfi_check"},
        ExportCase {"libarkaotmanager.so", "__cfi_check"},
        ExportCase {"libarkencoder.so", "__cfi_check"},
        ExportCase {"libarktscompiler.so", "__cfi_check"}));

INSTANTIATE_TEST_SUITE_P(
    InitializationSymbols, HiddenExportsTest,
    testing::Values(
        ExportCase {"libani.z.so", "_init"},
        ExportCase {"libani.z.so", "_fini"},
        ExportCase {"libani_helpers.z.so", "_init"},
        ExportCase {"libani_helpers.z.so", "_fini"},
        ExportCase {"libarkaotmanager.so", "_init"},
        ExportCase {"libarkaotmanager.so", "_fini"},
        ExportCase {"libarkencoder.so", "_init"},
        ExportCase {"libarkencoder.so", "_fini"},
        ExportCase {"libarktscompiler.so", "_init"},
        ExportCase {"libarktscompiler.so", "_fini"}));

// Pair these negative checks with the libarkencoder RequiredExportsTest cases above.
// dlsym may find the dependency's definition; it must not belong to the compiler.
INSTANTIATE_TEST_SUITE_P(
    DuplicateEncoderSymbols, HiddenExportsTest,
    testing::Values(
        ExportCase {"libarktscompiler.so", "_ZN3ark8compiler7Encoder6CreateEPNS_15ArenaAllocatorTILb0EEENS_4ArchEbb"},
        ExportCase {"libarktscompiler.so", "_ZN3ark8compiler7Encoder11CreateLabelEv"},
        ExportCase {"libarktscompiler.so", "_ZN3ark8compiler7Encoder9BindLabelEm"},
        ExportCase {"libarktscompiler.so", "_ZTVN3ark8compiler7EncoderE"}));

// These implementation copies were exported by the unfiltered libraries.
// ANI is an empty shim and does not contain securec implementations.
INSTANTIATE_TEST_SUITE_P(
    PrivateSecurecSymbols, HiddenExportsTest,
    testing::Values(
        ExportCase {"libani_helpers.z.so", "memcpy_s"},
        ExportCase {"libani_helpers.z.so", "memset_s"},
        ExportCase {"libarkaotmanager.so", "memcpy_s"},
        ExportCase {"libarkaotmanager.so", "memset_s"},
        ExportCase {"libarkencoder.so", "memcpy_s"},
        ExportCase {"libarkencoder.so", "memset_s"},
        ExportCase {"libarktscompiler.so", "memcpy_s"},
        ExportCase {"libarktscompiler.so", "memset_s"}));
// clang-format on

}  // namespace ark::test

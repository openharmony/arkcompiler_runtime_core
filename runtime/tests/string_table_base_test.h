/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef STRING_TABLE_BASE_TEST_H
#define STRING_TABLE_BASE_TEST_H

#include "gtest/gtest.h"
#include "runtime/include/coretypes/string.h"
#include "runtime/include/runtime.h"
#include "runtime/include/thread.h"
#include "runtime/include/gc_task.h"
#include "runtime/include/panda_vm.h"
#include "runtime/handle_base-inl.h"
#include "runtime/mem/refstorage/global_object_storage.h"
#include "runtime/include/thread_scopes.h"

#include "libpandafile/file.h"
#include "libpandafile/file_item_container.h"
#include "libpandafile/file_writer.h"

#include <limits>

namespace panda::mem::test {
class StringTableBaseTest : public testing::Test {
public:
    StringTableBaseTest() {}

    ~StringTableBaseTest() override
    {
        Runtime::Destroy();
    }

    static coretypes::String *AllocUtf8String(std::vector<uint8_t> data, bool is_movable = true)
    {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
        return coretypes::String::CreateFromMUtf8(data.data(), utf::MUtf8ToUtf16Size(data.data()), ctx,
                                                  Runtime::GetCurrent()->GetPandaVM(), is_movable);
    }

    void SetupRuntime(const std::string &gc_type)
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        if (gc_type == "g1-gc") {
            options.SetYoungSpaceSize(1_MB);
            options.SetHeapSizeLimit(3_MB);
        } else {
            options.SetYoungSpaceSize(18_MB);
            options.SetHeapSizeLimit(36_MB);
        }

        options.SetGcType(gc_type);
        options.SetCompilerEnableJit(false);
        Runtime::Create(options);

        thread_ = panda::MTManagedThread::GetCurrent();
    }

    void RunStringTableTests()
    {
        EmptyTable();
        InternCompressedUtf8AndString();
        InternUncompressedUtf8AndString();
        InternTheSameUtf16String();
        InternManyStrings();
        SweepObjectInTable();
        InternTooLongString();
    }

    void EmptyTable()
    {
        ScopedManagedCodeThread s(thread_);
        auto table = StringTable();
        ASSERT_EQ(table.Size(), 0);
    }

    void InternCompressedUtf8AndString()
    {
        ScopedManagedCodeThread s(thread_);
        auto table = StringTable();
        std::vector<uint8_t> data {0x01, 0x02, 0x03, 0x00};
        auto *string = AllocUtf8String(data);
        auto *interned_str1 =
            table.GetOrInternString(data.data(), data.size() - 1,
                                    Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY));
        auto *interned_str2 = table.GetOrInternString(
            string, Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY));
        ASSERT_EQ(interned_str1, interned_str2);
        ASSERT_EQ(table.Size(), 1);
    }

    void InternUncompressedUtf8AndString()
    {
        ScopedManagedCodeThread s(thread_);
        auto table = StringTable();
        std::vector<uint8_t> data {0xc2, 0xa7, 0x34, 0x00};
        auto *string = AllocUtf8String(data);
        auto *interned_str1 = table.GetOrInternString(
            data.data(), 2, Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY));
        auto *interned_str2 = table.GetOrInternString(
            string, Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY));
        ASSERT_EQ(interned_str1, interned_str2);
        ASSERT_EQ(table.Size(), 1);
    }

    void InternTheSameUtf16String()
    {
        ScopedManagedCodeThread s(thread_);
        auto table = StringTable();
        std::vector<uint16_t> data {0xffc3, 0x33, 0x00};

        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
        auto *first_string =
            coretypes::String::CreateFromUtf16(data.data(), data.size(), ctx, Runtime::GetCurrent()->GetPandaVM());
        auto *second_string =
            coretypes::String::CreateFromUtf16(data.data(), data.size(), ctx, Runtime::GetCurrent()->GetPandaVM());

        auto *interned_str1 = table.GetOrInternString(first_string, ctx);
        auto *interned_str2 = table.GetOrInternString(second_string, ctx);
        ASSERT_EQ(interned_str1, interned_str2);
        ASSERT_EQ(table.Size(), 1);
    }

    void InternManyStrings()
    {
        ScopedManagedCodeThread s(thread_);
        static constexpr size_t iterations = 50;
        auto table = StringTable();
        std::vector<uint8_t> data {0x00};
        const unsigned number_of_letters = 25;

        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
        for (size_t i = 0; i < iterations; i++) {
            data.insert(data.begin(), (('a' + i) % number_of_letters) + 1);
            [[maybe_unused]] auto *first_pointer = table.GetOrInternString(AllocUtf8String(data), ctx);
            [[maybe_unused]] auto *second_pointer =
                table.GetOrInternString(data.data(), utf::MUtf8ToUtf16Size(data.data()), ctx);
            auto *third_pointer = table.GetOrInternString(AllocUtf8String(data), ctx);
            ASSERT_EQ(first_pointer, second_pointer);
            ASSERT_EQ(second_pointer, third_pointer);
        }
        ASSERT_EQ(table.Size(), iterations);
    }

    void SweepObjectInTable()
    {
        ScopedManagedCodeThread s(thread_);
        auto table = thread_->GetVM()->GetStringTable();
        auto table_init_size = table->Size();
        std::vector<uint8_t> data1 {0x01, 0x00};
        std::vector<uint8_t> data2 {0x02, 0x00};
        std::vector<uint8_t> data3 {0x03, 0x00};
        const unsigned EXPECTED_TABLE_SIZE = 2;

        auto storage = thread_->GetVM()->GetGlobalObjectStorage();

        auto *s1 = AllocUtf8String(data1);
        auto ref1 = storage->Add(s1, Reference::ObjectType::GLOBAL);
        auto *s2 = AllocUtf8String(data2);
        auto ref2 = storage->Add(s2, Reference::ObjectType::GLOBAL);
        auto *s3 = AllocUtf8String(data3);
        auto ref3 = storage->Add(s3, Reference::ObjectType::GLOBAL);

        auto panda_class_context = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
        table->GetOrInternString(reinterpret_cast<coretypes::String *>(storage->Get(ref1)), panda_class_context);
        table->GetOrInternString(reinterpret_cast<coretypes::String *>(storage->Get(ref2)), panda_class_context);
        table->GetOrInternString(reinterpret_cast<coretypes::String *>(storage->Get(ref3)), panda_class_context);

        storage->Remove(ref2);

        thread_->GetVM()->GetGC()->WaitForGCInManaged(panda::GCTask(panda::GCTaskCause::EXPLICIT_CAUSE));
        // genGC collect all heap for EXPLICIT_CAUSE
        ASSERT_EQ(table->Size(), table_init_size + EXPECTED_TABLE_SIZE);
    }

    void InternTooLongString()
    {
        ScopedManagedCodeThread s(thread_);
        auto table = StringTable();
        auto *runtime = Runtime::GetCurrent();
        auto panda_class_context = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);

        auto *thread = ManagedThread::GetCurrent();

        [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);

        PandaVector<VMHandle<ObjectHeader>> objects;

        std::vector<uint8_t> string_data(10000U, 0x30);
        string_data.push_back(0x00);

        auto fillHeap = [&string_data, &thread, &objects](bool is_movable) {
            while (true) {
                auto *obj = AllocUtf8String(string_data, is_movable);
                if (obj == nullptr) {
                    thread->ClearException();
                    break;
                }
                objects.emplace_back(thread, obj);
            }
        };

        {
            fillHeap(true);
            auto *res = table.GetOrInternString(string_data.data(), string_data.size() - 1, panda_class_context);
            ASSERT_EQ(res, nullptr);
            ManagedThread::GetCurrent()->ClearException();
        }

        {
            panda_file::ItemContainer container;
            panda_file::MemoryWriter writer;

            auto *string_item = container.GetOrCreateStringItem(reinterpret_cast<char *>(string_data.data()));

            container.Write(&writer);
            auto data = writer.GetData();

            auto id = panda_file::File::EntityId(string_item->GetOffset());

            os::mem::ConstBytePtr ptr(reinterpret_cast<std::byte *>(data.data()), data.size(),
                                      [](std::byte *, size_t) noexcept {});

            auto pf = panda_file::File::OpenFromMemory(std::move(ptr));

            fillHeap(false);
            auto *res = table.GetOrInternInternalString(*pf.get(), id, panda_class_context);
            ASSERT_EQ(res, nullptr);
            ManagedThread::GetCurrent()->ClearException();
        }
    }

protected:
    panda::MTManagedThread *thread_;
};
}  // namespace panda::mem::test

#endif  // STRING_TABLE_BASE_TEST_H
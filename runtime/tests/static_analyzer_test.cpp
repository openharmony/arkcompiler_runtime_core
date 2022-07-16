/*
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

#include <gtest/gtest.h>

#include "assembly-parser.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/mem/vm_handle.h"
#include "runtime/handle_scope-inl.h"
#include "runtime/include/coretypes/array.h"
#include "runtime/include/coretypes/string.h"
#include "runtime/mem/object_helpers-inl.h"

namespace panda::mem {
inline std::string separator()
{
#ifdef _WIN32
    return "\\";
#else
    return "/";
#endif
}

class StaticAnalyzerTest : public testing::Test {
public:
    StaticAnalyzerTest()
    {
        RuntimeOptions options;
        options.SetBootClassSpaces({"core"});
        options.SetRuntimeType("core");
        options.SetGcType("epsilon");
        options.SetGcTriggerType("debug-never");
        auto exec_path = panda::os::file::File::GetExecutablePath();
        std::string panda_std_lib =
            exec_path.Value() + separator() + ".." + separator() + "pandastdlib" + separator() + "pandastdlib.bin";
        options.SetBootPandaFiles({panda_std_lib});

        Runtime::Create(options);
    }

    ~StaticAnalyzerTest()
    {
        Runtime::Destroy();
    }

    coretypes::String *AllocString()
    {
        Runtime *runtime = Runtime::GetCurrent();
        LanguageContext ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
        ScopedManagedCodeThread s(MTManagedThread::GetCurrent());
        return coretypes::String::CreateEmptyString(ctx, runtime->GetPandaVM());
    }

    coretypes::Array *AllocStringArray(size_t length)
    {
        Runtime *runtime = Runtime::GetCurrent();
        LanguageContext ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
        SpaceType space_type = SpaceType::SPACE_TYPE_OBJECT;
        auto *klass = runtime->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::ARRAY_STRING);
        ScopedManagedCodeThread s(MTManagedThread::GetCurrent());
        return coretypes::Array::Create(klass, length, space_type);
    }
};

TEST_F(StaticAnalyzerTest, TestArray)
{
    coretypes::Array *array = AllocStringArray(2);
    ASSERT_NE(nullptr, array);
    ObjectHeader *expected = AllocString();
    ASSERT_NE(nullptr, expected);
    array->Set(0U, expected);  // SUPPRESS_CSA
    // SUPPRESS_CSA_NEXTLINE
    array->Set(1U, expected);

    size_t count = 0;
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    auto handler = [array, &count, expected](ObjectHeader *obj, ObjectHeader *ref, uint32_t offset, bool is_volatile) {
        ++count;
        EXPECT_EQ(array, obj);
        EXPECT_EQ(expected, ref);
        EXPECT_EQ(ref, ObjectAccessor::GetObject<true>(obj, offset));
        EXPECT_FALSE(is_volatile);
        return true;
    };
    GCStaticObjectHelpers::TraverseAllObjectsWithInfo(array, handler);  // SUPPRESS_CSA(alpha.core.WasteObjHeader)
    ASSERT_EQ(2U, count);
}
}  // namespace panda::mem

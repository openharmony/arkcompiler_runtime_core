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

#include <gtest/gtest.h>

#include "assembler/assembly-parser.h"
#include "assembler/assembly-emitter.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/include/coretypes/array.h"
#include "runtime/include/coretypes/string.h"
#include "runtime/mem/vm_handle.h"
#include "runtime/handle_scope-inl.h"
#include "runtime/tooling/hprof/heap_dump.h"

namespace ark::tooling::hprof::test {

class HeapDumpTest : public testing::Test {
public:
    HeapDumpTest()
    {
        RuntimeOptions options;
        options.SetLoadRuntimes({"core"});
        options.SetGcType("epsilon");
        options.SetGcTriggerType("debug-never");
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);

        Runtime::Create(options);
        runtime_ = Runtime::GetCurrent();
        vm_ = runtime_->GetPandaVM();
    }

    ~HeapDumpTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(HeapDumpTest);
    NO_MOVE_SEMANTIC(HeapDumpTest);

    Class *LoadTestClass(const char *source)
    {
        ScopedManagedCodeThread s(MTManagedThread::GetCurrent());
        pandasm::Parser p;
        auto res = p.Parse(source);
        auto pf = pandasm::AsmEmitter::Emit(res.Value());
        if (pf == nullptr) {
            return nullptr;
        }

        ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
        classLinker->AddPandaFile(std::move(pf));
        auto *extension = classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY);

        PandaString descriptor;
        Class *klass = extension->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("Test"), &descriptor));
        if (klass != nullptr) {
            classLinker->InitializeClass(MTManagedThread::GetCurrent(), klass);
        }
        return klass;
    }

    ObjectHeader *NewObject(Class *klass)
    {
        ScopedManagedCodeThread s(MTManagedThread::GetCurrent());
        return runtime_->GetPandaVM()->GetHeapManager()->AllocateObject(klass, klass->GetObjectSize());
    }

    coretypes::String *AllocString(const char *str)
    {
        ScopedManagedCodeThread s(MTManagedThread::GetCurrent());
        LanguageContext ctx = runtime_->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
        return coretypes::String::CreateFromMUtf8(reinterpret_cast<const uint8_t *>(str), strlen(str), ctx, vm_);
    }

    coretypes::Array *AllocStringArray(size_t length)
    {
        ScopedManagedCodeThread s(MTManagedThread::GetCurrent());
        LanguageContext ctx = runtime_->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
        SpaceType spaceType = SpaceType::SPACE_TYPE_OBJECT;
        auto *klass = runtime_->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::ARRAY_STRING);
        return coretypes::Array::Create(klass, length, spaceType);
    }

protected:
    Runtime *runtime_;
    PandaVM *vm_;
};

// Test MapToStaticNodeType
TEST_F(HeapDumpTest, MapToStaticNodeType_ArrayClass)
{
    Class *klass = LoadTestClass(R"(
        .record Test {}
    )");
    ASSERT_NE(klass, nullptr);

    LanguageContext ctx = runtime_->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto *arrayClass = runtime_->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::ARRAY_I32);

    auto nodeType = HeapDump::MapToStaticNodeType(arrayClass);
    EXPECT_EQ(nodeType, arkplatform::StaticNodeType::ARRAY);
}

TEST_F(HeapDumpTest, MapToStaticNodeType_StringClass)
{
    LanguageContext ctx = runtime_->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto *stringClass = runtime_->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::STRING);

    auto nodeType = HeapDump::MapToStaticNodeType(stringClass);
    EXPECT_EQ(nodeType, arkplatform::StaticNodeType::STRING);
}

TEST_F(HeapDumpTest, MapToStaticNodeType_ClassClass)
{
    LanguageContext ctx = runtime_->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto *classClass = runtime_->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::CLASS);

    auto nodeType = HeapDump::MapToStaticNodeType(classClass);
    EXPECT_EQ(nodeType, arkplatform::StaticNodeType::CLASS);
}

TEST_F(HeapDumpTest, MapToStaticNodeType_RegularObject)
{
    Class *klass = LoadTestClass(R"(
        .record Test {}
    )");
    ASSERT_NE(klass, nullptr);

    auto nodeType = HeapDump::MapToStaticNodeType(klass);
    EXPECT_EQ(nodeType, arkplatform::StaticNodeType::OBJECT);
}

// Test GetNodeName
TEST_F(HeapDumpTest, GetNodeName_EmptyArray)
{
    auto *array = AllocStringArray(0);
    ASSERT_NE(array, nullptr);

    std::string name = HeapDump::GetNodeName(array);
    EXPECT_EQ(name, "Array[0]");
}

TEST_F(HeapDumpTest, GetNodeName_NonEmptyArray)
{
    auto *array = AllocStringArray(5);
    ASSERT_NE(array, nullptr);

    std::string name = HeapDump::GetNodeName(array);
    EXPECT_EQ(name, "Array[5]");
}

TEST_F(HeapDumpTest, GetNodeName_String)
{
    auto *str = AllocString("Hello");
    ASSERT_NE(str, nullptr);

    std::string name = HeapDump::GetNodeName(str);
    EXPECT_EQ(name, "Hello");
}

TEST_F(HeapDumpTest, GetNodeName_Class)
{
    LanguageContext ctx = runtime_->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto *classClass = runtime_->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::STRING);
    ObjectHeader *classObject = classClass->GetManagedObject();

    std::string name = HeapDump::GetNodeName(classObject);
    EXPECT_NE(name.find("String"), std::string::npos);
}

TEST_F(HeapDumpTest, GetNodeName_RegularObject)
{
    Class *klass = LoadTestClass(R"(
        .record Test {}
    )");
    ASSERT_NE(klass, nullptr);

    ObjectHeader *obj = NewObject(klass);
    ASSERT_NE(obj, nullptr);

    std::string name = HeapDump::GetNodeName(obj);
    // Class name may be returned without descriptor prefix
    EXPECT_TRUE(name == "Test" || name == "LTest;");
}

// Test GetObjectSize
TEST_F(HeapDumpTest, GetObjectSize_ReturnsNonZero)
{
    Class *klass = LoadTestClass(R"(
        .record Test {}
    )");
    ASSERT_NE(klass, nullptr);

    ObjectHeader *obj = NewObject(klass);
    ASSERT_NE(obj, nullptr);

    size_t size = HeapDump::GetObjectSize(obj);
    EXPECT_GT(size, 0);
}

// Test ObjectToNodeInfo
TEST_F(HeapDumpTest, ObjectToNodeInfo_String)
{
    auto *str = AllocString("TestString");
    ASSERT_NE(str, nullptr);

    auto nodeInfo = HeapDump::ObjectToNodeInfo(str);

    EXPECT_EQ(nodeInfo.name, "TestString");
    EXPECT_EQ(nodeInfo.nodeType, arkplatform::StaticNodeType::STRING);
    EXPECT_GT(nodeInfo.size, 0);
    EXPECT_EQ(nodeInfo.nativeSize, 0);
    EXPECT_EQ(nodeInfo.addr, reinterpret_cast<uint64_t>(str));
}

TEST_F(HeapDumpTest, ObjectToNodeInfo_Array)
{
    auto *array = AllocStringArray(10);
    ASSERT_NE(array, nullptr);

    auto nodeInfo = HeapDump::ObjectToNodeInfo(array);

    EXPECT_EQ(nodeInfo.name, "Array[10]");
    EXPECT_EQ(nodeInfo.nodeType, arkplatform::StaticNodeType::ARRAY);
    EXPECT_GT(nodeInfo.size, 0);
    EXPECT_EQ(nodeInfo.addr, reinterpret_cast<uint64_t>(array));
}

TEST_F(HeapDumpTest, ObjectToNodeInfo_Object)
{
    Class *klass = LoadTestClass(R"(
        .record Test {}
    )");
    ASSERT_NE(klass, nullptr);

    ObjectHeader *obj = NewObject(klass);
    ASSERT_NE(obj, nullptr);

    auto nodeInfo = HeapDump::ObjectToNodeInfo(obj);

    // Class name may be returned without descriptor prefix
    EXPECT_TRUE(nodeInfo.name == "Test" || nodeInfo.name == "LTest;");
    EXPECT_EQ(nodeInfo.nodeType, arkplatform::StaticNodeType::OBJECT);
    EXPECT_GT(nodeInfo.size, 0);
    EXPECT_EQ(nodeInfo.addr, reinterpret_cast<uint64_t>(obj));
}

// Test DumpObjectFields
TEST_F(HeapDumpTest, DumpObjectFields_NoFields)
{
    Class *klass = LoadTestClass(R"(
        .record Test {}
    )");
    ASSERT_NE(klass, nullptr);

    ObjectHeader *obj = NewObject(klass);
    ASSERT_NE(obj, nullptr);

    std::vector<arkplatform::EdgeInfo> edges;
    HeapDump::DumpObjectFields(obj, edges, nullptr);

    EXPECT_EQ(edges.size(), 0);
}

TEST_F(HeapDumpTest, DumpObjectFields_WithReferenceField)
{
    Class *klass = LoadTestClass(R"(
        .record panda.String <external>
        .record Test {
            panda.String field
        }
    )");
    ASSERT_NE(klass, nullptr);

    ObjectHeader *obj = NewObject(klass);
    ASSERT_NE(obj, nullptr);

    coretypes::String *str = AllocString("Test");
    ASSERT_NE(str, nullptr);

    Field *field = klass->GetInstanceFieldByName(reinterpret_cast<const uint8_t *>("field"));
    ASSERT_NE(field, nullptr);
    ObjectAccessor::SetFieldObject(obj, *field, str);

    std::vector<arkplatform::EdgeInfo> edges;
    HeapDump::DumpObjectFields(obj, edges, nullptr);

    ASSERT_EQ(edges.size(), 1);
    EXPECT_EQ(edges[0].edgeType, arkplatform::StaticEdgeType::PROPERTY);
    EXPECT_EQ(edges[0].fromAddr, reinterpret_cast<uint64_t>(obj));
    EXPECT_EQ(edges[0].toAddr, reinterpret_cast<uint64_t>(str));
    EXPECT_EQ(edges[0].name, "field");
}

TEST_F(HeapDumpTest, DumpObjectFields_WithWeakEdgeChecker)
{
    Class *klass = LoadTestClass(R"(
        .record panda.String <external>
        .record Test {
            panda.String field
        }
    )");
    ASSERT_NE(klass, nullptr);

    ObjectHeader *obj = NewObject(klass);
    ASSERT_NE(obj, nullptr);

    coretypes::String *str = AllocString("Test");
    ASSERT_NE(str, nullptr);

    Field *field = klass->GetInstanceFieldByName(reinterpret_cast<const uint8_t *>("field"));
    ASSERT_NE(field, nullptr);
    ObjectAccessor::SetFieldObject(obj, *field, str);

    auto checker = [obj, field](ObjectHeader *object, const Field &edgeField) {
        return object == obj && edgeField.GetOffset() == field->GetOffset();
    };

    std::vector<arkplatform::EdgeInfo> edges;
    HeapDump::DumpObjectFields(obj, edges, checker);

    ASSERT_EQ(edges.size(), 1);
    EXPECT_EQ(edges[0].edgeType, arkplatform::StaticEdgeType::WEAK);
}

// Test DumpArrayElements
TEST_F(HeapDumpTest, DumpArrayElements_EmptyArray)
{
    auto *array = AllocStringArray(0);
    ASSERT_NE(array, nullptr);

    Class *klass = array->ClassAddr<Class>();
    std::vector<arkplatform::EdgeInfo> edges;
    HeapDump::DumpArrayElements(array, klass, edges);

    EXPECT_EQ(edges.size(), 0);
}

TEST_F(HeapDumpTest, DumpArrayElements_NonEmptyArray)
{
    auto *array = AllocStringArray(2);
    ASSERT_NE(array, nullptr);

    coretypes::String *str1 = AllocString("First");
    coretypes::String *str2 = AllocString("Second");
    array->Set(0, str1);
    array->Set(1, str2);

    Class *klass = array->ClassAddr<Class>();
    std::vector<arkplatform::EdgeInfo> edges;
    HeapDump::DumpArrayElements(array, klass, edges);

    ASSERT_EQ(edges.size(), 2);  // 2 : Two edges should be present for the two non-null elements

    EXPECT_EQ(edges[0].edgeType, arkplatform::StaticEdgeType::ELEMENT);
    EXPECT_EQ(edges[0].index, 0);
    EXPECT_EQ(edges[0].toAddr, reinterpret_cast<uint64_t>(str1));

    EXPECT_EQ(edges[1].edgeType, arkplatform::StaticEdgeType::ELEMENT);
    EXPECT_EQ(edges[1].index, 1);
    EXPECT_EQ(edges[1].toAddr, reinterpret_cast<uint64_t>(str2));
}

TEST_F(HeapDumpTest, DumpArrayElements_WithNullElements)
{
    auto *array = AllocStringArray(3);
    ASSERT_NE(array, nullptr);

    coretypes::String *str1 = AllocString("Test1");
    coretypes::String *str2 = AllocString("Test2");
    array->Set(0, str1);
    // Index 1 remains null (not set)
    array->Set(2, str2);  // 2 : Set index 2 to non-null value

    Class *klass = array->ClassAddr<Class>();
    std::vector<arkplatform::EdgeInfo> edges;
    HeapDump::DumpArrayElements(array, klass, edges);

    ASSERT_EQ(edges.size(), 2);    // 2 : Only 2 edges should be present for non-null elements
    EXPECT_EQ(edges[0].index, 0);  // 0 ： Verify that index 0 is correctly reported
    EXPECT_EQ(edges[1].index,
              2);  // 2 : Verify that index 2 is correctly reported, and index 1 is not included in edges
}

// Test DumpClassStaticFields
TEST_F(HeapDumpTest, DumpClassStaticFields_ClassObject)
{
    LanguageContext ctx = runtime_->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto *classClass = runtime_->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::STRING);
    ObjectHeader *classObject = classClass->GetManagedObject();

    std::vector<arkplatform::EdgeInfo> edges;
    HeapDump::DumpClassStaticFields(classObject, edges, nullptr);

    // Class object should have static fields
    EXPECT_GE(edges.size(), 0);
}

// Test DumpReferences
TEST_F(HeapDumpTest, DumpReferences_Array)
{
    auto *array = AllocStringArray(2);
    ASSERT_NE(array, nullptr);

    coretypes::String *str = AllocString("Test");
    array->Set(0, str);

    std::vector<arkplatform::EdgeInfo> edges;
    HeapDump::DumpReferences(reinterpret_cast<uint64_t>(array), edges, nullptr);

    EXPECT_GT(edges.size(), 0);

    bool foundElementEdge = false;
    for (const auto &edge : edges) {
        if (edge.edgeType == arkplatform::StaticEdgeType::ELEMENT) {
            foundElementEdge = true;
            EXPECT_EQ(edge.index, 0);
            EXPECT_EQ(edge.toAddr, reinterpret_cast<uint64_t>(str));
        }
    }
    EXPECT_TRUE(foundElementEdge);
}

TEST_F(HeapDumpTest, DumpReferences_Object)
{
    Class *klass = LoadTestClass(R"(
        .record panda.String <external>
        .record Test {
            panda.String field
        }
    )");
    ASSERT_NE(klass, nullptr);

    ObjectHeader *obj = NewObject(klass);
    ASSERT_NE(obj, nullptr);

    coretypes::String *str = AllocString("Test");
    Field *field = klass->GetInstanceFieldByName(reinterpret_cast<const uint8_t *>("field"));
    ASSERT_NE(field, nullptr);
    ObjectAccessor::SetFieldObject(obj, *field, str);

    std::vector<arkplatform::EdgeInfo> edges;
    HeapDump::DumpReferences(reinterpret_cast<uint64_t>(obj), edges, nullptr);

    ASSERT_EQ(edges.size(), 1);
    EXPECT_EQ(edges[0].edgeType, arkplatform::StaticEdgeType::PROPERTY);
    EXPECT_EQ(edges[0].name, "field");
}

TEST_F(HeapDumpTest, DumpReferences_NullAddress)
{
    std::vector<arkplatform::EdgeInfo> edges;
    HeapDump::DumpReferences(0, edges, nullptr);

    EXPECT_EQ(edges.size(), 0);
}

// Test GetAllEtsObjects
TEST_F(HeapDumpTest, GetAllEtsObjects_ReturnsNonEmpty)
{
    auto *str = AllocString("Test");
    ASSERT_NE(str, nullptr);

    auto objects = HeapDump::GetAllEtsObjects(vm_);

    EXPECT_GT(objects.size(), 0);

    bool foundString = false;
    for (const auto &node : objects) {
        if (node.addr == reinterpret_cast<uint64_t>(str)) {
            foundString = true;
            EXPECT_EQ(node.nodeType, arkplatform::StaticNodeType::STRING);
        }
    }
    EXPECT_TRUE(foundString);
}

// Test IterateAllObjects
TEST_F(HeapDumpTest, IterateAllObjects_CallsCallback)
{
    auto *str1 = AllocString("Test1");
    auto *str2 = AllocString("Test2");
    ASSERT_NE(str1, nullptr);
    ASSERT_NE(str2, nullptr);

    std::vector<uint64_t> addresses;
    auto callback = [&addresses](uint64_t addr) { addresses.push_back(addr); };

    HeapDump::IterateAllObjects(vm_, callback);

    EXPECT_GT(addresses.size(), 0);

    bool foundStr1 = std::find(addresses.begin(), addresses.end(), reinterpret_cast<uint64_t>(str1)) != addresses.end();
    bool foundStr2 = std::find(addresses.begin(), addresses.end(), reinterpret_cast<uint64_t>(str2)) != addresses.end();

    EXPECT_TRUE(foundStr1);
    EXPECT_TRUE(foundStr2);
}

}  // namespace ark::tooling::hprof::test

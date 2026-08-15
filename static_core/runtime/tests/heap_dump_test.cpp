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

#include <algorithm>
#include <cmath>
#include <initializer_list>
#include <limits>
#include <utility>

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

namespace {

constexpr int8_t NEGATIVE_I8_VALUE = -8;
constexpr int8_t POSITIVE_I8_VALUE = 7;
constexpr uint8_t U8_VALUE = 250U;
constexpr int16_t NEGATIVE_I16_VALUE = -1600;
constexpr int16_t POSITIVE_I16_VALUE = 1600;
constexpr uint16_t U16_VALUE = 65000U;
constexpr int32_t NEGATIVE_I32_VALUE = -320000;
constexpr int32_t POSITIVE_I32_VALUE = 320000;
constexpr uint32_t U32_VALUE = 4000000000U;
constexpr int64_t NEGATIVE_I64_VALUE = -9000000000LL;
constexpr int64_t POSITIVE_I64_VALUE = 9000000000LL;
constexpr uint64_t U64_VALUE = 18000000000ULL;
constexpr float POSITIVE_F32_VALUE = 1.5F;
constexpr float NEGATIVE_F32_VALUE = -2.5F;
constexpr double NEGATIVE_F64_VALUE = -2.25;
constexpr double POSITIVE_F64_VALUE = 2.25;
constexpr double ARRAY_NEGATIVE_F64_VALUE = -3.5;
constexpr int32_t NUMERIC_PRIMITIVE_VALUE = 42;
constexpr int32_t NEGATIVE_NUMERIC_PRIMITIVE_VALUE = -42;

constexpr const char *PRIMITIVE_FIELDS_SOURCE = R"(
    .record Test {
        u1 enabled
        u1 disabled
        i8 i8Value
        u8 u8Value
        i16 i16Value
        u16 u16Value
        i32 i32Value
        u32 u32Value
        i64 i64Value
        u64 u64Value
        f32 f32Value
        f64 f64Value
    }
)";

struct PrimitiveEdgeExpectation {
    uint64_t fromAddr;
    const char *name;
    uint32_t index;
    arkplatform::StaticPrimitiveType primitiveType;
    const char *primitiveValue;
};

}  // namespace

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

    const arkplatform::EdgeInfo *FindEdgeByName(const std::vector<arkplatform::EdgeInfo> &edges,
                                                const std::string &name) const
    {
        auto edge = std::find_if(edges.begin(), edges.end(), [&name](const auto &item) { return item.name == name; });
        return edge == edges.end() ? nullptr : &*edge;
    }

    void AssertPrimitiveEdge(const arkplatform::EdgeInfo &edge, const PrimitiveEdgeExpectation &expected) const
    {
        EXPECT_EQ(edge.edgeType, expected.name[0] == '\0' ? arkplatform::StaticEdgeType::ELEMENT
                                                          : arkplatform::StaticEdgeType::PROPERTY);
        EXPECT_EQ(edge.fromAddr, expected.fromAddr);
        EXPECT_EQ(edge.toAddr, 0U);
        EXPECT_EQ(edge.name, expected.name);
        EXPECT_EQ(edge.index, expected.index);
        EXPECT_EQ(edge.primitiveType, expected.primitiveType);
        EXPECT_EQ(edge.primitiveValue, expected.primitiveValue);
    }

    void AssertPrimitiveFieldEdge(const std::vector<arkplatform::EdgeInfo> &edges, uint64_t fromAddr, const char *name,
                                  arkplatform::StaticPrimitiveType primitiveType, const char *primitiveValue) const
    {
        const auto *edge = FindEdgeByName(edges, name);
        ASSERT_NE(edge, nullptr) << name;
        PrimitiveEdgeExpectation expected {};
        expected.fromAddr = fromAddr;
        expected.name = name;
        expected.index = 0U;
        expected.primitiveType = primitiveType;
        expected.primitiveValue = primitiveValue;
        AssertPrimitiveEdge(*edge, expected);
    }

    template <class T>
    void SetInstancePrimitive(ObjectHeader *object, Class *klass, const char *name, T value)
    {
        auto *field = klass->GetInstanceFieldByName(reinterpret_cast<const uint8_t *>(name));
        ASSERT_NE(field, nullptr);
        object->SetFieldPrimitive<T>(*field, value);
    }

    template <class T>
    void SetStaticPrimitive(Class *klass, const char *name, T value)
    {
        auto *field = klass->GetStaticFieldByName(reinterpret_cast<const uint8_t *>(name));
        ASSERT_NE(field, nullptr);
        klass->SetFieldPrimitive<T>(*field, value);
    }

    void SetAllPrimitiveFields(ObjectHeader *object, Class *klass)
    {
        SetInstancePrimitive<bool>(object, klass, "enabled", true);
        SetInstancePrimitive<bool>(object, klass, "disabled", false);
        SetInstancePrimitive<int8_t>(object, klass, "i8Value", NEGATIVE_I8_VALUE);
        SetInstancePrimitive<uint8_t>(object, klass, "u8Value", U8_VALUE);
        SetInstancePrimitive<int16_t>(object, klass, "i16Value", NEGATIVE_I16_VALUE);
        SetInstancePrimitive<uint16_t>(object, klass, "u16Value", U16_VALUE);
        SetInstancePrimitive<int32_t>(object, klass, "i32Value", NEGATIVE_I32_VALUE);
        SetInstancePrimitive<uint32_t>(object, klass, "u32Value", U32_VALUE);
        SetInstancePrimitive<int64_t>(object, klass, "i64Value", NEGATIVE_I64_VALUE);
        SetInstancePrimitive<uint64_t>(object, klass, "u64Value", U64_VALUE);
        SetInstancePrimitive<float>(object, klass, "f32Value", POSITIVE_F32_VALUE);
        SetInstancePrimitive<double>(object, klass, "f64Value", NEGATIVE_F64_VALUE);
    }

    template <class T>
    void AssertPrimitiveArray(ClassRoot classRoot, std::initializer_list<T> values,
                              arkplatform::StaticPrimitiveType primitiveType,
                              std::initializer_list<const char *> expectedValues)
    {
        ASSERT_EQ(values.size(), expectedValues.size());
        ScopedManagedCodeThread scope(MTManagedThread::GetCurrent());
        LanguageContext ctx = runtime_->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
        auto *klass = runtime_->GetClassLinker()->GetExtension(ctx)->GetClassRoot(classRoot);
        ASSERT_NE(klass, nullptr);
        auto *array = coretypes::Array::Create(klass, values.size(), SpaceType::SPACE_TYPE_OBJECT);
        ASSERT_NE(array, nullptr);

        size_t index = 0;
        for (T value : values) {
            array->template SetPrimitive<T>(index * klass->GetComponentSize(), value);
            index++;
        }

        std::vector<arkplatform::EdgeInfo> edges;
        HeapDump::DumpArrayElements(array, klass, edges);
        ASSERT_EQ(edges.size(), expectedValues.size());
        index = 0;
        for (const char *expectedValue : expectedValues) {
            AssertPrimitiveEdge(edges[index], {reinterpret_cast<uint64_t>(array), "", static_cast<uint32_t>(index),
                                               primitiveType, expectedValue});
            index++;
        }
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

TEST_F(HeapDumpTest, DumpObjectFields_AllPrimitiveTypes)
{
    Class *klass = LoadTestClass(PRIMITIVE_FIELDS_SOURCE);
    ASSERT_NE(klass, nullptr);

    ObjectHeader *obj = NewObject(klass);
    ASSERT_NE(obj, nullptr);
    SetAllPrimitiveFields(obj, klass);

    std::vector<arkplatform::EdgeInfo> edges;
    HeapDump::DumpObjectFields(obj, edges, nullptr);

    ASSERT_EQ(edges.size(), 12U);
    const uint64_t fromAddr = reinterpret_cast<uint64_t>(obj);
    AssertPrimitiveFieldEdge(edges, fromAddr, "enabled", arkplatform::StaticPrimitiveType::BOOLEAN, "Boolean:true");
    AssertPrimitiveFieldEdge(edges, fromAddr, "disabled", arkplatform::StaticPrimitiveType::BOOLEAN, "Boolean:false");
    AssertPrimitiveFieldEdge(edges, fromAddr, "i8Value", arkplatform::StaticPrimitiveType::NUMBER, "Int:-8");
    AssertPrimitiveFieldEdge(edges, fromAddr, "u8Value", arkplatform::StaticPrimitiveType::NUMBER, "Int:250");
    AssertPrimitiveFieldEdge(edges, fromAddr, "i16Value", arkplatform::StaticPrimitiveType::NUMBER, "Int:-1600");
    AssertPrimitiveFieldEdge(edges, fromAddr, "u16Value", arkplatform::StaticPrimitiveType::NUMBER, "Int:65000");
    AssertPrimitiveFieldEdge(edges, fromAddr, "i32Value", arkplatform::StaticPrimitiveType::NUMBER, "Int:-320000");
    AssertPrimitiveFieldEdge(edges, fromAddr, "u32Value", arkplatform::StaticPrimitiveType::NUMBER, "Int:4000000000");
    AssertPrimitiveFieldEdge(edges, fromAddr, "i64Value", arkplatform::StaticPrimitiveType::NUMBER, "Int:-9000000000");
    AssertPrimitiveFieldEdge(edges, fromAddr, "u64Value", arkplatform::StaticPrimitiveType::NUMBER, "Int:18000000000");
    AssertPrimitiveFieldEdge(edges, fromAddr, "f32Value", arkplatform::StaticPrimitiveType::NUMBER, "Double:1.5");
    AssertPrimitiveFieldEdge(edges, fromAddr, "f64Value", arkplatform::StaticPrimitiveType::NUMBER, "Double:-2.25");
}

TEST_F(HeapDumpTest, DumpObjectFields_PrimitiveCaptureOptions)
{
    Class *klass = LoadTestClass(R"(
        .record Test {
            u1 enabled
            i32 count
        }
    )");
    ASSERT_NE(klass, nullptr);

    ObjectHeader *obj = NewObject(klass);
    ASSERT_NE(obj, nullptr);
    auto *enabled = klass->GetInstanceFieldByName(reinterpret_cast<const uint8_t *>("enabled"));
    auto *count = klass->GetInstanceFieldByName(reinterpret_cast<const uint8_t *>("count"));
    ASSERT_NE(enabled, nullptr);
    ASSERT_NE(count, nullptr);
    obj->SetFieldPrimitive<bool>(*enabled, true);
    obj->SetFieldPrimitive<int32_t>(*count, NUMERIC_PRIMITIVE_VALUE);

    std::vector<arkplatform::EdgeInfo> edges;
    HeapDump::DumpObjectFields(obj, edges, nullptr, false, false);
    ASSERT_EQ(edges.size(), 1U);
    AssertPrimitiveEdge(edges[0], {reinterpret_cast<uint64_t>(obj), "enabled", 0U,
                                   arkplatform::StaticPrimitiveType::BOOLEAN, "Boolean:true"});

    edges.clear();
    HeapDump::DumpObjectFields(obj, edges, nullptr, false, true);
    ASSERT_EQ(edges.size(), 2U);
    const auto *countEdge = FindEdgeByName(edges, "count");
    ASSERT_NE(countEdge, nullptr);
    AssertPrimitiveEdge(
        *countEdge, {reinterpret_cast<uint64_t>(obj), "count", 0U, arkplatform::StaticPrimitiveType::NUMBER, "Int:42"});

    edges.clear();
    HeapDump::DumpObjectFields(obj, edges, nullptr, true, true);
    EXPECT_TRUE(edges.empty());
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

TEST_F(HeapDumpTest, DumpArrayElements_AllPrimitiveTypes)
{
    ASSERT_NO_FATAL_FAILURE(AssertPrimitiveArray<bool>(ClassRoot::ARRAY_U1, {false, true},
                                                       arkplatform::StaticPrimitiveType::BOOLEAN,
                                                       {"Boolean:false", "Boolean:true"}));
    ASSERT_NO_FATAL_FAILURE(AssertPrimitiveArray<int8_t>(ClassRoot::ARRAY_I8, {NEGATIVE_I8_VALUE, POSITIVE_I8_VALUE},
                                                         arkplatform::StaticPrimitiveType::NUMBER,
                                                         {"Int:-8", "Int:7"}));
    ASSERT_NO_FATAL_FAILURE(AssertPrimitiveArray<uint8_t>(
        ClassRoot::ARRAY_U8, {1U, U8_VALUE}, arkplatform::StaticPrimitiveType::NUMBER, {"Int:1", "Int:250"}));
    ASSERT_NO_FATAL_FAILURE(
        AssertPrimitiveArray<int16_t>(ClassRoot::ARRAY_I16, {NEGATIVE_I16_VALUE, POSITIVE_I16_VALUE},
                                      arkplatform::StaticPrimitiveType::NUMBER, {"Int:-1600", "Int:1600"}));
    ASSERT_NO_FATAL_FAILURE(AssertPrimitiveArray<uint16_t>(
        ClassRoot::ARRAY_U16, {1U, U16_VALUE}, arkplatform::StaticPrimitiveType::NUMBER, {"Int:1", "Int:65000"}));
    ASSERT_NO_FATAL_FAILURE(
        AssertPrimitiveArray<int32_t>(ClassRoot::ARRAY_I32, {NEGATIVE_I32_VALUE, POSITIVE_I32_VALUE},
                                      arkplatform::StaticPrimitiveType::NUMBER, {"Int:-320000", "Int:320000"}));
    ASSERT_NO_FATAL_FAILURE(AssertPrimitiveArray<uint32_t>(
        ClassRoot::ARRAY_U32, {1U, U32_VALUE}, arkplatform::StaticPrimitiveType::NUMBER, {"Int:1", "Int:4000000000"}));
    ASSERT_NO_FATAL_FAILURE(
        AssertPrimitiveArray<int64_t>(ClassRoot::ARRAY_I64, {NEGATIVE_I64_VALUE, POSITIVE_I64_VALUE},
                                      arkplatform::StaticPrimitiveType::NUMBER, {"Int:-9000000000", "Int:9000000000"}));
    ASSERT_NO_FATAL_FAILURE(AssertPrimitiveArray<uint64_t>(ClassRoot::ARRAY_U64, {1ULL, U64_VALUE},
                                                           arkplatform::StaticPrimitiveType::NUMBER,
                                                           {"Int:1", "Int:18000000000"}));
    ASSERT_NO_FATAL_FAILURE(AssertPrimitiveArray<float>(ClassRoot::ARRAY_F32, {POSITIVE_F32_VALUE, NEGATIVE_F32_VALUE},
                                                        arkplatform::StaticPrimitiveType::NUMBER,
                                                        {"Double:1.5", "Double:-2.5"}));
    ASSERT_NO_FATAL_FAILURE(
        AssertPrimitiveArray<double>(ClassRoot::ARRAY_F64, {POSITIVE_F64_VALUE, ARRAY_NEGATIVE_F64_VALUE},
                                     arkplatform::StaticPrimitiveType::NUMBER, {"Double:2.25", "Double:-3.5"}));
}

TEST_F(HeapDumpTest, DumpArrayElements_FloatingPointNamesPreserveRoundTripPrecision)
{
    const float firstFloat = std::nextafter(1.0F, std::numeric_limits<float>::infinity());
    const float secondFloat = std::nextafter(firstFloat, std::numeric_limits<float>::infinity());
    ASSERT_NO_FATAL_FAILURE(AssertPrimitiveArray<float>(ClassRoot::ARRAY_F32, {firstFloat, secondFloat},
                                                        arkplatform::StaticPrimitiveType::NUMBER,
                                                        {"Double:1.00000012", "Double:1.00000024"}));

    const double firstDouble = std::nextafter(1.0, std::numeric_limits<double>::infinity());
    const double secondDouble = std::nextafter(firstDouble, std::numeric_limits<double>::infinity());
    ASSERT_NO_FATAL_FAILURE(AssertPrimitiveArray<double>(ClassRoot::ARRAY_F64, {firstDouble, secondDouble},
                                                         arkplatform::StaticPrimitiveType::NUMBER,
                                                         {"Double:1.0000000000000002", "Double:1.0000000000000004"}));
}

TEST_F(HeapDumpTest, DumpArrayElements_PrimitiveCaptureOptions)
{
    ScopedManagedCodeThread scope(MTManagedThread::GetCurrent());
    LanguageContext ctx = runtime_->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto *extension = runtime_->GetClassLinker()->GetExtension(ctx);
    auto *booleanClass = extension->GetClassRoot(ClassRoot::ARRAY_U1);
    ASSERT_NE(booleanClass, nullptr);
    auto *booleanArray = coretypes::Array::Create(booleanClass, 1U, SpaceType::SPACE_TYPE_OBJECT);
    ASSERT_NE(booleanArray, nullptr);
    booleanArray->SetPrimitive<bool>(0, true);

    std::vector<arkplatform::EdgeInfo> edges;
    HeapDump::DumpArrayElements(booleanArray, booleanClass, edges, false, false);
    ASSERT_EQ(edges.size(), 1U);
    AssertPrimitiveEdge(edges[0], {reinterpret_cast<uint64_t>(booleanArray), "", 0U,
                                   arkplatform::StaticPrimitiveType::BOOLEAN, "Boolean:true"});

    edges.clear();
    HeapDump::DumpArrayElements(booleanArray, booleanClass, edges, true, true);
    EXPECT_TRUE(edges.empty());

    auto *numericClass = extension->GetClassRoot(ClassRoot::ARRAY_I32);
    ASSERT_NE(numericClass, nullptr);
    auto *numericArray = coretypes::Array::Create(numericClass, 1U, SpaceType::SPACE_TYPE_OBJECT);
    ASSERT_NE(numericArray, nullptr);
    numericArray->SetPrimitive<int32_t>(0, NUMERIC_PRIMITIVE_VALUE);
    HeapDump::DumpArrayElements(numericArray, numericClass, edges, false, false);
    EXPECT_TRUE(edges.empty());

    HeapDump::DumpArrayElements(numericArray, numericClass, edges, false, true);
    ASSERT_EQ(edges.size(), 1U);
    AssertPrimitiveEdge(edges[0], {reinterpret_cast<uint64_t>(numericArray), "", 0U,
                                   arkplatform::StaticPrimitiveType::NUMBER, "Int:42"});
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
TEST_F(HeapDumpTest, DumpClassStaticFields_WithPrimitiveFields)
{
    Class *klass = LoadTestClass(R"(
        .record Test {
            u1 enabled <static>
            u1 disabled <static>
            i32 count <static>
        }
    )");
    ASSERT_NE(klass, nullptr);
    SetStaticPrimitive<bool>(klass, "enabled", true);
    SetStaticPrimitive<bool>(klass, "disabled", false);
    SetStaticPrimitive<int32_t>(klass, "count", NEGATIVE_NUMERIC_PRIMITIVE_VALUE);
    ObjectHeader *classObject = klass->GetManagedObject();
    ASSERT_NE(classObject, nullptr);

    std::vector<arkplatform::EdgeInfo> edges;
    HeapDump::DumpClassStaticFields(classObject, edges, nullptr, false, true);

    ASSERT_EQ(edges.size(), 3U);
    const auto *enabledEdge = FindEdgeByName(edges, "enabled");
    const auto *disabledEdge = FindEdgeByName(edges, "disabled");
    const auto *countEdge = FindEdgeByName(edges, "count");
    ASSERT_NE(enabledEdge, nullptr);
    ASSERT_NE(disabledEdge, nullptr);
    ASSERT_NE(countEdge, nullptr);
    const uint64_t fromAddr = reinterpret_cast<uint64_t>(classObject);
    AssertPrimitiveEdge(*enabledEdge,
                        {fromAddr, "enabled", 0U, arkplatform::StaticPrimitiveType::BOOLEAN, "Boolean:true"});
    AssertPrimitiveEdge(*disabledEdge,
                        {fromAddr, "disabled", 0U, arkplatform::StaticPrimitiveType::BOOLEAN, "Boolean:false"});
    AssertPrimitiveEdge(*countEdge, {fromAddr, "count", 0U, arkplatform::StaticPrimitiveType::NUMBER, "Int:-42"});
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

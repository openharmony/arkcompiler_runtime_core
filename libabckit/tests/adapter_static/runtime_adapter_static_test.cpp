/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "gtest/gtest.h"
#include <iostream>
#include "libabckit/src/adapter_static/runtime_adapter_static.h"
#include "assembly-parser.h"

namespace libabckit::test::adapter_static {
using namespace ark;
using namespace ark::panda_file;
using namespace ark::pandasm;
using namespace ark::compiler;
class RuntimeAdapterStaticTest : public testing::Test {
public:
    RuntimeAdapterStaticTest() {}
    ~RuntimeAdapterStaticTest() override {}

    NO_COPY_SEMANTIC(RuntimeAdapterStaticTest);
    NO_MOVE_SEMANTIC(RuntimeAdapterStaticTest);
};

static const uint8_t *GetTypeDescriptor(const std::string &name, std::string *storage)
{
    *storage = "L" + name + ";";
    std::replace(storage->begin(), storage->end(), '.', '/');
    return utf::CStringAsMutf8(storage->c_str());
}

static void MainTestFunc(const std::string &source, RuntimeInterface::IntrinsicId expectId, const std::string &name)
{
    Parser p;
    auto res = p.Parse(source);
    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE);

    auto pf = AsmEmitter::Emit(res.Value());
    EXPECT_NE(pf, nullptr);

    std::string descriptor;
    auto class_id = pf->GetClassId(GetTypeDescriptor(name, &descriptor));
    EXPECT_TRUE(class_id.IsValid());
    EXPECT_FALSE(pf->IsExternal(class_id));
    libabckit::AbckitRuntimeAdapterStatic abckitRuntimeAdapterStatic(*pf);
    panda_file::ClassDataAccessor cda(*pf, class_id);
    cda.EnumerateMethods([&](MethodDataAccessor &mda) {
        auto methodPtr = reinterpret_cast<RuntimeInterface::MethodPtr>(mda.GetMethodId().GetOffset());
        auto intrinsicId = abckitRuntimeAdapterStatic.GetIntrinsicId(methodPtr);
        EXPECT_EQ(intrinsicId, expectId);
    });
}

TEST_F(RuntimeAdapterStaticTest, RuntimeAdapterStaticTestNormal)
{
    std::string source = R"(
        .record IO {}
        .function void IO.printI64(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_IO_PRINT_I64, "IO");
}

TEST_F(RuntimeAdapterStaticTest, RuntimeAdapterStaticTestInvalid)
{
    std::string source = R"(
        .record IO {}
        .function i32 IO.printU32(i32 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "IO");
}

TEST_F(RuntimeAdapterStaticTest, MathAbsI32Test)
{
    std::string source = R"(
        .record Math {}
        .function i32 Math.absI32(i32 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_MATH_ABS_I32, "Math");

    source = R"(
        .record Math {}
        .function void Math.absI32() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Math");
}

TEST_F(RuntimeAdapterStaticTest, MathAbsI64Test)
{
    std::string source = R"(
        .record Math {}
        .function i64 Math.absI64(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_MATH_ABS_I64, "Math");

    source = R"(
        .record Math {}
        .function void Math.absI64() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Math");
}

TEST_F(RuntimeAdapterStaticTest, MathAbsF32Test)
{
    std::string source = R"(
        .record Math {}
        .function f32 Math.absF32(f32 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_MATH_ABS_F32, "Math");

    source = R"(
        .record Math {}
        .function void Math.absF32() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Math");
}

TEST_F(RuntimeAdapterStaticTest, MathAbsF64Test)
{
    std::string source = R"(
        .record Math {}
        .function f64 Math.absF64(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_MATH_ABS_F64, "Math");

    source = R"(
        .record Math {}
        .function void Math.absF64() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Math");
}

TEST_F(RuntimeAdapterStaticTest, MathMinI32Test)
{
    std::string source = R"(
        .record Math {}
        .function i32 Math.minI32(i32 a0, i32 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MIN_I32, "Math");

    source = R"(
        .record Math {}
        .function void Math.minI32() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Math");
}

TEST_F(RuntimeAdapterStaticTest, MathMinI64Test)
{
    std::string source = R"(
        .record Math {}
        .function i64 Math.minI64(i64 a0, i64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MIN_I64, "Math");

    source = R"(
        .record Math {}
        .function void Math.minI64() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Math");
}

TEST_F(RuntimeAdapterStaticTest, MathMinF32Test)
{
    std::string source = R"(
        .record Math {}
        .function f32 Math.minF32(f32 a0, f32 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MIN_F32, "Math");

    source = R"(
        .record Math {}
        .function void Math.minF32() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Math");
}

TEST_F(RuntimeAdapterStaticTest, MathMinF64Test)
{
    std::string source = R"(
        .record Math {}
        .function f64 Math.minF64(f64 a0, f64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MIN_F64, "Math");

    source = R"(
        .record Math {}
        .function void Math.minF64() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Math");
}

TEST_F(RuntimeAdapterStaticTest, MathMaxI32Test)
{
    std::string source = R"(
        .record Math {}
        .function i32 Math.maxI32(i32 a0, i32 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MAX_I32, "Math");

    source = R"(
        .record Math {}
        .function void Math.maxI32() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Math");
}

TEST_F(RuntimeAdapterStaticTest, MathMaxI64Test)
{
    std::string source = R"(
        .record Math {}
        .function i64 Math.maxI64(i64 a0, i64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MAX_I64, "Math");

    source = R"(
        .record Math {}
        .function void Math.maxI64() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Math");
}

TEST_F(RuntimeAdapterStaticTest, MathMaxF32Test)
{
    std::string source = R"(
        .record Math {}
        .function f32 Math.maxF32(f32 a0, f32 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MAX_F32, "Math");

    source = R"(
        .record Math {}
        .function void Math.maxF32() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Math");
}

TEST_F(RuntimeAdapterStaticTest, MathMaxF64Test)
{
    std::string source = R"(
        .record Math {}
        .function f64 Math.maxF64(f64 a0, f64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MAX_F64, "Math");

    source = R"(
        .record Math {}
        .function void Math.maxF64() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Math");
}

TEST_F(RuntimeAdapterStaticTest, MathSinF32Test)
{
    std::string source = R"(
        .record Math {}
        .function f32 Math.fsin(f32 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_MATH_SIN_F32, "Math");

    source = R"(
        .record Math {}
        .function void Math.fsin() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Math");
}

TEST_F(RuntimeAdapterStaticTest, MathSinF64Test)
{
    std::string source = R"(
        .record Math {}
        .function f64 Math.sin(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_MATH_SIN_F64, "Math");

    source = R"(
        .record Math {}
        .function void Math.sin() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Math");
}

TEST_F(RuntimeAdapterStaticTest, MathCosF32Test)
{
    std::string source = R"(
        .record Math {}
        .function f32 Math.fcos(f32 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_MATH_COS_F32, "Math");

    source = R"(
        .record Math {}
        .function void Math.fcos() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Math");
}

TEST_F(RuntimeAdapterStaticTest, MathCosF64Test)
{
    std::string source = R"(
        .record Math {}
        .function f64 Math.cos(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_MATH_COS_F64, "Math");

    source = R"(
        .record Math {}
        .function void Math.cos() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Math");
}

TEST_F(RuntimeAdapterStaticTest, MathPowF32Test)
{
    std::string source = R"(
        .record Math {}
        .function f32 Math.fpow(f32 a0, f32 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_MATH_POW_F32, "Math");

    source = R"(
        .record Math {}
        .function void Math.fpow() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Math");
}

TEST_F(RuntimeAdapterStaticTest, MathPowF64Test)
{
    std::string source = R"(
        .record Math {}
        .function f64 Math.pow(f64 a0, f64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_MATH_POW_F64, "Math");

    source = R"(
        .record Math {}
        .function void Math.pow() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Math");
}

TEST_F(RuntimeAdapterStaticTest, MathSqrtF32Test)
{
    std::string source = R"(
        .record Math {}
        .function f32 Math.fsqrt(f32 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_MATH_SQRT_F32, "Math");

    source = R"(
        .record Math {}
        .function void Math.fsqrt() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Math");
}

TEST_F(RuntimeAdapterStaticTest, MathSqrtF64Test)
{
    std::string source = R"(
        .record Math {}
        .function f64 Math.sqrt(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_MATH_SQRT_F64, "Math");

    source = R"(
        .record Math {}
        .function void Math.sqrt() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Math");
}

TEST_F(RuntimeAdapterStaticTest, MathCalculateDoubleTest)
{
    std::string source = R"(
        .record Math {}
        .function f64 Math.calculateDouble(u32 a0, f64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_MATH_CALCULATE_DOUBLE, "Math");

    source = R"(
        .record Math {}
        .function void Math.calculateDouble() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Math");
}

TEST_F(RuntimeAdapterStaticTest, MathCalculateFloatTest)
{
    std::string source = R"(
        .record Math {}
        .function f32 Math.calculateFloat(u32 a0, f32 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_MATH_CALCULATE_FLOAT, "Math");

    source = R"(
        .record Math {}
        .function void Math.calculateFloat() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Math");
}

TEST_F(RuntimeAdapterStaticTest, DoubleIsInfTest)
{
    std::string source = R"(
        .record Double {}
        .function u1 Double.isInfinite(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_DOUBLE_IS_INF, "Double");

    source = R"(
        .record Double {}
        .function void Double.isInfinite() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Double");
}

TEST_F(RuntimeAdapterStaticTest, FloatIsInfTest)
{
    std::string source = R"(
        .record Float {}
        .function u1 Float.isInfinite(f32 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_FLOAT_IS_INF, "Float");

    source = R"(
        .record Float {}
        .function void Float.isInfinite() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Float");
}

TEST_F(RuntimeAdapterStaticTest, IOPrintStringTest)
{
    std::string source = R"(
        .record IO {}
        .record panda.String {}
        .function void IO.printString(panda.String a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_IO_PRINT_STRING, "IO");

    source = R"(
        .record IO {}
        .function i32 IO.printString() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "IO");
}

TEST_F(RuntimeAdapterStaticTest, IOPrintF32Test)
{
    std::string source = R"(
        .record IO {}
        .function void IO.printF32(f32 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_IO_PRINT_F32, "IO");

    source = R"(
        .record IO {}
        .function i32 IO.printF32() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "IO");
}

TEST_F(RuntimeAdapterStaticTest, IOPrintF64Test)
{
    std::string source = R"(
        .record IO {}
        .function void IO.printF64(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_IO_PRINT_F64, "IO");

    source = R"(
        .record IO {}
        .function i32 IO.printF64() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "IO");
}

TEST_F(RuntimeAdapterStaticTest, IOPrintI32Test)
{
    std::string source = R"(
        .record IO {}
        .function void IO.printI32(i32 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_IO_PRINT_I32, "IO");

    source = R"(
        .record IO {}
        .function i32 IO.printI32() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "IO");
}

TEST_F(RuntimeAdapterStaticTest, IOPrintU32Test)
{
    std::string source = R"(
        .record IO {}
        .function void IO.printU32(u32 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_IO_PRINT_U32, "IO");

    source = R"(
        .record IO {}
        .function i32 IO.printU32() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "IO");
}

TEST_F(RuntimeAdapterStaticTest, IOPrintI64Test)
{
    std::string source = R"(
        .record IO {}
        .function void IO.printI64(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_IO_PRINT_I64, "IO");

    source = R"(
        .record IO {}
        .function i32 IO.printI64() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "IO");
}

TEST_F(RuntimeAdapterStaticTest, IOPrintU64Test)
{
    std::string source = R"(
        .record IO {}
        .function void IO.printU64(u64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_IO_PRINT_U64, "IO");

    source = R"(
        .record IO {}
        .function i32 IO.printU64() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "IO");
}

TEST_F(RuntimeAdapterStaticTest, SystemCompileMethodTest)
{
    std::string source = R"(
        .record System {}
        .record panda.String {}
        .function u8 System.compileMethod(panda.String a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_SYSTEM_COMPILE_METHOD, "System");

    source = R"(
        .record System {}
        .function void System.compileMethod() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "System");
}

TEST_F(RuntimeAdapterStaticTest, SystemExitTest)
{
    std::string source = R"(
        .record System {}
        .function void System.exit(i32 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_SYSTEM_EXIT, "System");

    source = R"(
        .record System {}
        .function i32 System.exit() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "System");
}

TEST_F(RuntimeAdapterStaticTest, SystemNanoTimeTest)
{
    std::string source = R"(
        .record System {}
        .function i64 System.nanoTime() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_SYSTEM_NANO_TIME, "System");

    source = R"(
        .record System {}
        .function void System.nanoTime() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "System");
}

TEST_F(RuntimeAdapterStaticTest, SystemAssertTest)
{
    std::string source = R"(
        .record System {}
        .function void System.assert(u1 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_SYSTEM_ASSERT, "System");

    source = R"(
        .record System {}
        .function i32 System.assert() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "System");
}

TEST_F(RuntimeAdapterStaticTest, SystemAssertPrintTest)
{
    std::string source = R"(
        .record System {}
        .record panda.String {}
        .function void System.assertPrint(u1 a0, panda.String a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_SYSTEM_ASSERT_PRINT, "System");

    source = R"(
        .record System {}
        .function i32 System.assertPrint() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "System");
}

TEST_F(RuntimeAdapterStaticTest, SystemScheduleCoroutineTest)
{
    std::string source = R"(
        .record System {}
        .function void System.scheduleCoroutine() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_SYSTEM_SCHEDULE_COROUTINE, "System");

    source = R"(
        .record System {}
        .function i32 System.scheduleCoroutine() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "System");
}

TEST_F(RuntimeAdapterStaticTest, SystemCoroutineGetWorkerIdTest)
{
    std::string source = R"(
        .record System {}
        .function i32 System.getCoroutineWorkerId() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_SYSTEM_COROUTINE_GET_WORKER_ID, "System");

    source = R"(
        .record System {}
        .function void System.getCoroutineWorkerId() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "System");
}

TEST_F(RuntimeAdapterStaticTest, CheckTagTest)
{
    std::string source = R"(
        .record DebugUtils {}
        .function void DebugUtils.checkTag(i64 a0, i64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_CHECK_TAG, "DebugUtils");

    source = R"(
        .record DebugUtils {}
        .function i32 DebugUtils.checkTag() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "DebugUtils");
}

TEST_F(RuntimeAdapterStaticTest, ConvertStringToI32Test)
{
    std::string source = R"(
        .record Convert {}
        .record panda.String {}
        .function i32 Convert.stringToI32(panda.String a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_CONVERT_STRING_TO_I32, "Convert");

    source = R"(
        .record Convert {}
        .function void Convert.stringToI32() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Convert");
}

TEST_F(RuntimeAdapterStaticTest, ConvertStringToU32Test)
{
    std::string source = R"(
        .record Convert {}
        .record panda.String {}
        .function u32 Convert.stringToU32(panda.String a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_CONVERT_STRING_TO_U32, "Convert");

    source = R"(
        .record Convert {}
        .function void Convert.stringToU32() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Convert");
}

TEST_F(RuntimeAdapterStaticTest, ConvertStringToI64Test)
{
    std::string source = R"(
        .record Convert {}
        .record panda.String {}
        .function i64 Convert.stringToI64(panda.String a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_CONVERT_STRING_TO_I64, "Convert");

    source = R"(
        .record Convert {}
        .function void Convert.stringToI64() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Convert");
}

TEST_F(RuntimeAdapterStaticTest, ConvertStringToU64Test)
{
    std::string source = R"(
        .record Convert {}
        .record panda.String {}
        .function u64 Convert.stringToU64(panda.String a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_CONVERT_STRING_TO_U64, "Convert");

    source = R"(
        .record Convert {}
        .function void Convert.stringToU64() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Convert");
}

TEST_F(RuntimeAdapterStaticTest, ConvertStringToF32Test)
{
    std::string source = R"(
        .record Convert {}
        .record panda.String {}
        .function f32 Convert.stringToF32(panda.String a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_CONVERT_STRING_TO_F32, "Convert");

    source = R"(
        .record Convert {}
        .function void Convert.stringToF32() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Convert");
}

TEST_F(RuntimeAdapterStaticTest, ConvertStringToF64Test)
{
    std::string source = R"(
        .record Convert {}
        .record panda.String {}
        .function f64 Convert.stringToF64(panda.String a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_CONVERT_STRING_TO_F64, "Convert");

    source = R"(
        .record Convert {}
        .function void Convert.stringToF64() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Convert");
}

TEST_F(RuntimeAdapterStaticTest, ObjectCreateNonMovableTest)
{
    std::string source = R"(
        .record Object {}
        .record panda.Class {}
        .record panda.Object {}
        .function panda.Object Object.createNonMovable(panda.Class a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_OBJECT_CREATE_NON_MOVABLE, "Object");

    source = R"(
        .record Object {}
        .function void Object.createNonMovable() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Object");
}

TEST_F(RuntimeAdapterStaticTest, ObjectMonitorEnterTest)
{
    std::string source = R"(
        .record Object {}
        .record panda.Object {}
        .function void Object.monitorEnter(panda.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_OBJECT_MONITOR_ENTER, "Object");

    source = R"(
        .record Object {}
        .function i32 Object.monitorEnter() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Object");
}

TEST_F(RuntimeAdapterStaticTest, ObjectMonitorExitTest)
{
    std::string source = R"(
        .record Object {}
        .record panda.Object {}
        .function void Object.monitorExit(panda.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_OBJECT_MONITOR_EXIT, "Object");

    source = R"(
        .record Object {}
        .function i32 Object.monitorExit() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Object");
}

TEST_F(RuntimeAdapterStaticTest, ObjectWaitTest)
{
    std::string source = R"(
        .record Object {}
        .record panda.Object {}
        .function void Object.Wait(panda.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_OBJECT_WAIT, "Object");

    source = R"(
        .record Object {}
        .function i32 Object.Wait() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Object");
}

TEST_F(RuntimeAdapterStaticTest, ObjectTimedWaitTest)
{
    std::string source = R"(
        .record Object {}
        .record panda.Object {}
        .function void Object.TimedWait(panda.Object a0, u64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_OBJECT_TIMED_WAIT, "Object");

    source = R"(
        .record Object {}
        .function i32 Object.TimedWait() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Object");
}

TEST_F(RuntimeAdapterStaticTest, ObjectTimedWaitNanosTest)
{
    std::string source = R"(
        .record Object {}
        .record panda.Object {}
        .function void Object.TimedWaitNanos(panda.Object a0, u64 a1, u64 a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_OBJECT_TIMED_WAIT_NANOS, "Object");

    source = R"(
        .record Object {}
        .function i32 Object.TimedWaitNanos() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Object");
}

TEST_F(RuntimeAdapterStaticTest, ObjectNotifyTest)
{
    std::string source = R"(
        .record Object {}
        .record panda.Object {}
        .function void Object.Notify(panda.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_OBJECT_NOTIFY, "Object");

    source = R"(
        .record Object {}
        .function i32 Object.Notify() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Object");
}

TEST_F(RuntimeAdapterStaticTest, ObjectNotifyAllTest)
{
    std::string source = R"(
        .record Object {}
        .record panda.Object {}
        .function void Object.NotifyAll(panda.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_OBJECT_NOTIFY_ALL, "Object");

    source = R"(
        .record Object {}
        .function i32 Object.NotifyAll() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Object");
}

TEST_F(RuntimeAdapterStaticTest, StdMathSinTest)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function f64 std.math.ETSGLOBAL.sin(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_SIN, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.sin() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMathCosTest)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function f64 std.math.ETSGLOBAL.cos(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_COS, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.cos() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMathPowerTest)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function f64 std.math.ETSGLOBAL.power(f64 a0, f64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_POWER, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.power() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMathSqrtTest)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function f64 std.math.ETSGLOBAL.sqrt(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_SQRT, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.sqrt() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMathAbsTest)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function f64 std.math.ETSGLOBAL.abs(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_ABS, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.abs() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMathMaxI32Test)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function i32 std.math.ETSGLOBAL.max(i32 a0, i32 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_MAX_I32, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.max() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMathMaxI64Test)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function i64 std.math.ETSGLOBAL.max(i64 a0, i64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_MAX_I64, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.max() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMathMaxF32Test)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function f32 std.math.ETSGLOBAL.max(f32 a0, f32 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_MAX_F32, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.max() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMathMaxF64Test)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function f64 std.math.ETSGLOBAL.max(f64 a0, f64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_MAX_F64, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.max() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMathMinI32Test)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function i32 std.math.ETSGLOBAL.min(i32 a0, i32 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_MIN_I32, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.min() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMathMinI64Test)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function i64 std.math.ETSGLOBAL.min(i64 a0, i64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_MIN_I64, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.min() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMathMinF32Test)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function f32 std.math.ETSGLOBAL.min(f32 a0, f32 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_MIN_F32, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.min() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMathMinF64Test)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function f64 std.math.ETSGLOBAL.min(f64 a0, f64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_MIN_F64, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.min() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMathRandomTest)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function f64 std.math.ETSGLOBAL.random() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_RANDOM, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.random() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMathAcosTest)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function f64 std.math.ETSGLOBAL.acos(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_ACOS, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.acos() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMathAcoshTest)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function f64 std.math.ETSGLOBAL.acosh(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_ACOSH, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.acosh() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMathAsinTest)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function f64 std.math.ETSGLOBAL.asin(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_ASIN, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.asin() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMathAsinhTest)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function f64 std.math.ETSGLOBAL.asinh(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_ASINH, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.asinh() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMathAtan2Test)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function f64 std.math.ETSGLOBAL.atan2(f64 a0, f64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_ATAN2, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.atan2() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMathAtanhTest)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function f64 std.math.ETSGLOBAL.atanh(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_ATANH, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.atanh() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMathAtanTest)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function f64 std.math.ETSGLOBAL.atan(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_ATAN, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.atan() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMathSinhTest)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function f64 std.math.ETSGLOBAL.sinh(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_SINH, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.sinh() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMathCoshTest)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function f64 std.math.ETSGLOBAL.cosh(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_COSH, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.cosh() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMathFloorTest)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function f64 std.math.ETSGLOBAL.floor(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_FLOOR, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.floor() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMathRoundTest)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function f64 std.math.ETSGLOBAL.round(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_ROUND, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.round() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMathTruncTest)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function f64 std.math.ETSGLOBAL.trunc(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_TRUNC, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.trunc() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMathCbrtTest)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function f64 std.math.ETSGLOBAL.cbrt(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_CBRT, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.cbrt() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMathTanTest)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function f64 std.math.ETSGLOBAL.tan(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_TAN, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.tan() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMathTanhTest)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function f64 std.math.ETSGLOBAL.tanh(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_TANH, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.tanh() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMathExpTest)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function f64 std.math.ETSGLOBAL.exp(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_EXP, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.exp() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMathLog10Test)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function f64 std.math.ETSGLOBAL.log10(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_LOG10, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.log10() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMathExpm1Test)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function f64 std.math.ETSGLOBAL.expm1(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_EXPM1, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.expm1() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMathCeilTest)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function f64 std.math.ETSGLOBAL.ceil(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_CEIL, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.ceil() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMathLogTest)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function f64 std.math.ETSGLOBAL.log(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_LOG, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.log() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMathRemTest)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function f64 std.math.ETSGLOBAL.rem(f64 a0, f64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_REM, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.rem() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMathModTest)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function f64 std.math.ETSGLOBAL.mod(f64 a0, f64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_MOD, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.mod() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMathClz64Test)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function i32 std.math.ETSGLOBAL.clz64(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_CLZ64, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.clz64() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMathClz32Test)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function i32 std.math.ETSGLOBAL.clz32(i32 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_CLZ32, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.clz32() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMathClz32DoubleTest)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function f64 std.math.ETSGLOBAL.clz32Double(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_CLZ32_DOUBLE, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.clz32Double() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMathSignbitTest)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function u1 std.math.ETSGLOBAL.signbit(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_SIGNBIT, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.signbit() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMathImulTest)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function i32 std.math.ETSGLOBAL.imul(f64 a0, f64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_IMUL, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.imul() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMathFroundTest)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function f64 std.math.ETSGLOBAL.fround(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_FROUND, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.fround() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMathHypotTest)
{
    std::string source = R"(
        .record std.math.ETSGLOBAL {}
        .function f64 std.math.ETSGLOBAL.hypot(f64 a0, f64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_HYPOT, "std.math.ETSGLOBAL");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.hypot() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreLoadLibraryTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.String {}
        .function void std.core.ETSGLOBAL.loadLibrary(std.core.String a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_LOAD_LIBRARY, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function i32 std.core.ETSGLOBAL.loadLibrary() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreLoadLibraryWithPermissionCheckTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.String {}
        .function void std.core.ETSGLOBAL.loadLibraryWithPermissionCheck(std.core.String a0, std.core.String a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_LOAD_LIBRARY_WITH_PERMISSION_CHECK,
                 "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function i32 std.core.ETSGLOBAL.loadLibraryWithPermissionCheck() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreInitializePrimitivesInClassTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.initializePrimitivesInClass() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_INITIALIZE_PRIMITIVES_IN_CLASS,
                 "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function i32 std.core.ETSGLOBAL.initializePrimitivesInClass() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreAllocGenericArrayTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .function std.core.Object[] std.core.ETSGLOBAL.__alloc_array(i32 a0, {Ustd.core.Null, std.core.Object} a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_ALLOC_GENERIC_ARRAY, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.__alloc_array() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreBoolCopyToTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.copyTo(u1[] a0, u1[] a1, i32 a2, i32 a3, i32 a4) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_BOOL_COPY_TO, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function i32 std.core.ETSGLOBAL.copyTo() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreCharCopyToTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.copyTo(u16[] a0, u16[] a1, i32 a2, i32 a3, i32 a4) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_CHAR_COPY_TO, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function i32 std.core.ETSGLOBAL.copyTo() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreByteCopyToTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.copyTo(i8[] a0, i8[] a1, i32 a2, i32 a3, i32 a4) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_BYTE_COPY_TO, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function i32 std.core.ETSGLOBAL.copyTo() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreShortCopyToTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.copyTo(i16[] a0, i16[] a1, i32 a2, i32 a3, i32 a4) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_SHORT_COPY_TO, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function i32 std.core.ETSGLOBAL.copyTo() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreIntCopyToTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.copyTo(i32[] a0, i32[] a1, i32 a2, i32 a3, i32 a4) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_INT_COPY_TO, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function i32 std.core.ETSGLOBAL.copyTo() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreLongCopyToTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.copyTo(i64[] a0, i64[] a1, i32 a2, i32 a3, i32 a4) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_LONG_COPY_TO, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function i32 std.core.ETSGLOBAL.copyTo() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreObjectCopyToTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.copyTo(std.core.Object[] a0, std.core.Object[] a1, i32 a2, i32 a3, i32 a4) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_OBJECT_COPY_TO, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function i32 std.core.ETSGLOBAL.copyTo() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreFloatCopyToTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.copyTo(f32[] a0, f32[] a1, i32 a2, i32 a3, i32 a4) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_FLOAT_COPY_TO, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function i32 std.core.ETSGLOBAL.copyTo() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreDoubleCopyToTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.copyTo(f64[] a0, f64[] a1, i32 a2, i32 a3, i32 a4) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_DOUBLE_COPY_TO, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function i32 std.core.ETSGLOBAL.copyTo() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdCorePrintStackTraceTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.printStackTrace() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_PRINT_STACK_TRACE, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function i32 std.core.ETSGLOBAL.printStackTrace() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreStackTraceLinesTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .function std.core.String[] std.core.ETSGLOBAL.stackTraceLines() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STACK_TRACE_LINES, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.stackTraceLines() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreExitTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.exit(i32 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_EXIT, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function i32 std.core.ETSGLOBAL.exit() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreGetNearestNonBootRuntimeLinkerTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.RuntimeLinker {}
        .function std.core.RuntimeLinker std.core.ETSGLOBAL.getNearestNonBootRuntimeLinker() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_GET_NEAREST_NON_BOOT_RUNTIME_LINKER,
                 "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.getNearestNonBootRuntimeLinker() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPIGetStaticFieldValueTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.String {}
        .record std.core.Type {}
        .record std.core.Object {}
        .function std.core.Object std.core.ETSGLOBAL.TypeAPIGetStaticFieldValue(std.core.Type a0, std.core.String a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_GET_STATIC_FIELD_VALUE,
                 "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.TypeAPIGetStaticFieldValue() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPISetStaticFieldValueTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.String {}
        .record std.core.Type {}
        .record std.core.Object {}
        .function void std.core.ETSGLOBAL.TypeAPISetStaticFieldValue(std.core.Type a0, std.core.String a1, std.core.Object a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_SET_STATIC_FIELD_VALUE,
                 "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function i32 std.core.ETSGLOBAL.TypeAPISetStaticFieldValue() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMethodInvokeTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.Object {}
        .record std.core.MethodType {}
        .function std.core.Object std.core.ETSGLOBAL.TypeAPIMethodInvoke(std.core.MethodType a0, std.core.Object a1, std.core.Object[] a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_METHOD_INVOKE, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.TypeAPIMethodInvoke() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdMethodInvokeConstructorTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.Object {}
        .record std.core.MethodType {}
        .function std.core.Object std.core.ETSGLOBAL.TypeAPIMethodInvokeConstructor(std.core.MethodType a0, std.core.Object[] a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_METHOD_INVOKE_CONSTRUCTOR, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.TypeAPIMethodInvokeConstructor() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPIGetFunctionObjectNameFromAnnotationTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.String {}
        .record std.core.Object {}
        .function std.core.String std.core.ETSGLOBAL.getFunctionObjectNameFromAnnotation(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_GET_FUNCTION_OBJECT_NAME_FROM_ANNOTATION,
                 "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.getFunctionObjectNameFromAnnotation() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPIGetFieldObjectTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.Object {}
        .function std.core.Object std.core.ETSGLOBAL.ValueAPIGetFieldObject(std.core.Object a0, i64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_GET_FIELD_OBJECT, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.ValueAPIGetFieldObject() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPIGetFieldBooleanTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.Object {}
        .function u1 std.core.ETSGLOBAL.ValueAPIGetFieldBoolean(std.core.Object a0, i64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_GET_FIELD_BOOLEAN, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.ValueAPIGetFieldBoolean() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPIGetFieldByteTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.Object {}
        .function i8 std.core.ETSGLOBAL.ValueAPIGetFieldByte(std.core.Object a0, i64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_GET_FIELD_BYTE, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.ValueAPIGetFieldByte() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPIGetFieldCharTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.Object {}
        .function u16 std.core.ETSGLOBAL.ValueAPIGetFieldChar(std.core.Object a0, i64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_GET_FIELD_CHAR, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.ValueAPIGetFieldChar() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPIGetFieldShortTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.Object {}
        .function i16 std.core.ETSGLOBAL.ValueAPIGetFieldShort(std.core.Object a0, i64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_GET_FIELD_SHORT, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.ValueAPIGetFieldShort() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPIGetFieldIntTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.Object {}
        .function i32 std.core.ETSGLOBAL.ValueAPIGetFieldInt(std.core.Object a0, i64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_GET_FIELD_INT, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.ValueAPIGetFieldInt() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPIGetFieldLongTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.Object {}
        .function i64 std.core.ETSGLOBAL.ValueAPIGetFieldLong(std.core.Object a0, i64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_GET_FIELD_LONG, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.ValueAPIGetFieldLong() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPIGetFieldFloatTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.Object {}
        .function f32 std.core.ETSGLOBAL.ValueAPIGetFieldFloat(std.core.Object a0, i64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_GET_FIELD_FLOAT, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.ValueAPIGetFieldFloat() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPIGetFieldDoubleTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.Object {}
        .function f64 std.core.ETSGLOBAL.ValueAPIGetFieldDouble(std.core.Object a0, i64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_GET_FIELD_DOUBLE, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.ValueAPIGetFieldDouble() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPIGetFieldByNameObjectTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.String {}
        .record std.core.Object {}
        .function std.core.Object std.core.ETSGLOBAL.ValueAPIGetFieldByNameObject(std.core.Object a0, std.core.String a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_GET_FIELD_BY_NAME_OBJECT,
                 "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.ValueAPIGetFieldByNameObject() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPIGetFieldByNameBooleanTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.String {}
        .record std.core.Object {}
        .function u1 std.core.ETSGLOBAL.ValueAPIGetFieldByNameBoolean(std.core.Object a0, std.core.String a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_GET_FIELD_BY_NAME_BOOLEAN,
                 "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.ValueAPIGetFieldByNameBoolean() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPIGetFieldByNameByteTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.String {}
        .record std.core.Object {}
        .function i8 std.core.ETSGLOBAL.ValueAPIGetFieldByNameByte(std.core.Object a0, std.core.String a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_GET_FIELD_BY_NAME_BYTE,
                 "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.ValueAPIGetFieldByNameByte() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPIGetFieldByNameCharTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.String {}
        .record std.core.Object {}
        .function u16 std.core.ETSGLOBAL.ValueAPIGetFieldByNameChar(std.core.Object a0, std.core.String a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_GET_FIELD_BY_NAME_CHAR,
                 "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.ValueAPIGetFieldByNameChar() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPIGetFieldByNameShortTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.String {}
        .record std.core.Object {}
        .function i16 std.core.ETSGLOBAL.ValueAPIGetFieldByNameShort(std.core.Object a0, std.core.String a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_GET_FIELD_BY_NAME_SHORT,
                 "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.ValueAPIGetFieldByNameShort() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPIGetFieldByNameIntTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.String {}
        .record std.core.Object {}
        .function i32 std.core.ETSGLOBAL.ValueAPIGetFieldByNameInt(std.core.Object a0, std.core.String a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_GET_FIELD_BY_NAME_INT,
                 "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.ValueAPIGetFieldByNameInt() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPIGetFieldByNameLongTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.String {}
        .record std.core.Object {}
        .function i64 std.core.ETSGLOBAL.ValueAPIGetFieldByNameLong(std.core.Object a0, std.core.String a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_GET_FIELD_BY_NAME_LONG,
                 "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.ValueAPIGetFieldByNameLong() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPIGetFieldByNameFloatTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.String {}
        .record std.core.Object {}
        .function f32 std.core.ETSGLOBAL.ValueAPIGetFieldByNameFloat(std.core.Object a0, std.core.String a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_GET_FIELD_BY_NAME_FLOAT,
                 "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.ValueAPIGetFieldByNameFloat() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPIGetFieldByNameDoubleTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.String {}
        .record std.core.Object {}
        .function f64 std.core.ETSGLOBAL.ValueAPIGetFieldByNameDouble(std.core.Object a0, std.core.String a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_GET_FIELD_BY_NAME_DOUBLE,
                 "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.ValueAPIGetFieldByNameDouble() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPISetFieldObjectTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.Object {}
        .function void std.core.ETSGLOBAL.ValueAPISetFieldObject(std.core.Object a0, i64 a1, std.core.Object a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_SET_FIELD_OBJECT, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function i32 std.core.ETSGLOBAL.ValueAPISetFieldObject() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPISetFieldBooleanTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.Object {}
        .function void std.core.ETSGLOBAL.ValueAPISetFieldBoolean(std.core.Object a0, i64 a1, u1 a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_SET_FIELD_BOOLEAN, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function i32 std.core.ETSGLOBAL.ValueAPISetFieldBoolean() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPISetFieldByteTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.Object {}
        .function void std.core.ETSGLOBAL.ValueAPISetFieldByte(std.core.Object a0, i64 a1, i8 a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_SET_FIELD_BYTE, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function i32 std.core.ETSGLOBAL.ValueAPISetFieldByte() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPISetFieldCharTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.Object {}
        .function void std.core.ETSGLOBAL.ValueAPISetFieldChar(std.core.Object a0, i64 a1, u16 a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_SET_FIELD_CHAR, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function i32 std.core.ETSGLOBAL.ValueAPISetFieldChar() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPISetFieldShortTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.Object {}
        .function void std.core.ETSGLOBAL.ValueAPISetFieldShort(std.core.Object a0, i64 a1, i16 a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_SET_FIELD_SHORT, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function i32 std.core.ETSGLOBAL.ValueAPISetFieldShort() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPISetFieldIntTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.Object {}
        .function void std.core.ETSGLOBAL.ValueAPISetFieldInt(std.core.Object a0, i64 a1, i32 a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_SET_FIELD_INT, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function i32 std.core.ETSGLOBAL.ValueAPISetFieldInt() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPISetFieldLongTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.Object {}
        .function void std.core.ETSGLOBAL.ValueAPISetFieldLong(std.core.Object a0, i64 a1, i64 a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_SET_FIELD_LONG, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function i32 std.core.ETSGLOBAL.ValueAPISetFieldLong() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPISetFieldFloatTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.Object {}
        .function void std.core.ETSGLOBAL.ValueAPISetFieldFloat(std.core.Object a0, i64 a1, f32 a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_SET_FIELD_FLOAT, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function i32 std.core.ETSGLOBAL.ValueAPISetFieldFloat() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPISetFieldDoubleTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.Object {}
        .function void std.core.ETSGLOBAL.ValueAPISetFieldDouble(std.core.Object a0, i64 a1, f64 a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_SET_FIELD_DOUBLE, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function i32 std.core.ETSGLOBAL.ValueAPISetFieldDouble() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPISetFieldByNameObjectTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.String {}
        .record std.core.Object {}
        .function void std.core.ETSGLOBAL.ValueAPISetFieldByNameObject(std.core.Object a0, std.core.String a1, std.core.Object a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_SET_FIELD_BY_NAME_OBJECT,
                 "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function i32 std.core.ETSGLOBAL.ValueAPISetFieldByNameObject() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPISetFieldByNameBooleanTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.String {}
        .record std.core.Object {}
        .function void std.core.ETSGLOBAL.ValueAPISetFieldByNameBoolean(std.core.Object a0, std.core.String a1, u1 a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_SET_FIELD_BY_NAME_BOOLEAN,
                 "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function i32 std.core.ETSGLOBAL.ValueAPISetFieldByNameBoolean() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPISetFieldByNameByteTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.String {}
        .record std.core.Object {}
        .function void std.core.ETSGLOBAL.ValueAPISetFieldByNameByte(std.core.Object a0, std.core.String a1, i8 a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_SET_FIELD_BY_NAME_BYTE,
                 "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function i32 std.core.ETSGLOBAL.ValueAPISetFieldByNameByte() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPISetFieldByNameCharTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.String {}
        .record std.core.Object {}
        .function void std.core.ETSGLOBAL.ValueAPISetFieldByNameChar(std.core.Object a0, std.core.String a1, u16 a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_SET_FIELD_BY_NAME_CHAR,
                 "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function i32 std.core.ETSGLOBAL.ValueAPISetFieldByNameChar() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPISetFieldByNameShortTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.String {}
        .record std.core.Object {}
        .function void std.core.ETSGLOBAL.ValueAPISetFieldByNameShort(std.core.Object a0, std.core.String a1, i16 a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_SET_FIELD_BY_NAME_SHORT,
                 "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function i32 std.core.ETSGLOBAL.ValueAPISetFieldByNameShort() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPISetFieldByNameIntTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.String {}
        .record std.core.Object {}
        .function void std.core.ETSGLOBAL.ValueAPISetFieldByNameInt(std.core.Object a0, std.core.String a1, i32 a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_SET_FIELD_BY_NAME_INT,
                 "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function i32 std.core.ETSGLOBAL.ValueAPISetFieldByNameInt() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPISetFieldByNameLongTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.String {}
        .record std.core.Object {}
        .function void std.core.ETSGLOBAL.ValueAPISetFieldByNameLong(std.core.Object a0, std.core.String a1, i64 a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_SET_FIELD_BY_NAME_LONG,
                 "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function i32 std.core.ETSGLOBAL.ValueAPISetFieldByNameLong() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPISetFieldByNameFloatTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.String {}
        .record std.core.Object {}
        .function void std.core.ETSGLOBAL.ValueAPISetFieldByNameFloat(std.core.Object a0, std.core.String a1, f32 a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_SET_FIELD_BY_NAME_FLOAT,
                 "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function i32 std.core.ETSGLOBAL.ValueAPISetFieldByNameFloat() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPISetFieldByNameDoubleTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.String {}
        .record std.core.Object {}
        .function void std.core.ETSGLOBAL.ValueAPISetFieldByNameDouble(std.core.Object a0, std.core.String a1, f64 a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_SET_FIELD_BY_NAME_DOUBLE,
                 "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function i32 std.core.ETSGLOBAL.ValueAPISetFieldByNameDouble() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPIGetArrayLengthTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.Object {}
        .function i64 std.core.ETSGLOBAL.ValueAPIGetArrayLength(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_GET_ARRAY_LENGTH, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.ValueAPIGetArrayLength() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPISetElementObjectTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.Object {}
        .function void std.core.ETSGLOBAL.ValueAPISetElementObject(std.core.Object a0, i64 a1, std.core.Object a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_SET_ELEMENT_OBJECT, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function i32 std.core.ETSGLOBAL.ValueAPISetElementObject() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPISetElementBooleanTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.Object {}
        .function void std.core.ETSGLOBAL.ValueAPISetElementBoolean(std.core.Object a0, i64 a1, u1 a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_SET_ELEMENT_BOOLEAN, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function i32 std.core.ETSGLOBAL.ValueAPISetElementBoolean() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPISetElementByteTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.Object {}
        .function void std.core.ETSGLOBAL.ValueAPISetElementByte(std.core.Object a0, i64 a1, i8 a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_SET_ELEMENT_BYTE, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function i32 std.core.ETSGLOBAL.ValueAPISetElementByte() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPISetElementCharTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.Object {}
        .function void std.core.ETSGLOBAL.ValueAPISetElementChar(std.core.Object a0, i64 a1, u16 a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_SET_ELEMENT_CHAR, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function i32 std.core.ETSGLOBAL.ValueAPISetElementChar() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPISetElementShortTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.Object {}
        .function void std.core.ETSGLOBAL.ValueAPISetElementShort(std.core.Object a0, i64 a1, i16 a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_SET_ELEMENT_SHORT, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function i32 std.core.ETSGLOBAL.ValueAPISetElementShort() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPISetElementIntTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.Object {}
        .function void std.core.ETSGLOBAL.ValueAPISetElementInt(std.core.Object a0, i64 a1, i32 a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_SET_ELEMENT_INT, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function i32 std.core.ETSGLOBAL.ValueAPISetElementInt() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPISetElementLongTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.Object {}
        .function void std.core.ETSGLOBAL.ValueAPISetElementLong(std.core.Object a0, i64 a1, i64 a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_SET_ELEMENT_LONG, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function i32 std.core.ETSGLOBAL.ValueAPISetElementLong() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPISetElementFloatTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.Object {}
        .function void std.core.ETSGLOBAL.ValueAPISetElementFloat(std.core.Object a0, i64 a1, f32 a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_SET_ELEMENT_FLOAT, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function i32 std.core.ETSGLOBAL.ValueAPISetElementFloat() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPISetElementDoubleTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.Object {}
        .function void std.core.ETSGLOBAL.ValueAPISetElementDouble(std.core.Object a0, i64 a1, f64 a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_SET_ELEMENT_DOUBLE, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function i32 std.core.ETSGLOBAL.ValueAPISetElementDouble() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPIGetElementObjectTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.Object {}
        .function std.core.Object std.core.ETSGLOBAL.ValueAPIGetElementObject(std.core.Object a0, i64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_GET_ELEMENT_OBJECT, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.ValueAPIGetElementObject() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPIGetElementBooleanTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.Object {}
        .function u1 std.core.ETSGLOBAL.ValueAPIGetElementBoolean(std.core.Object a0, i64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_GET_ELEMENT_BOOLEAN, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.ValueAPIGetElementBoolean() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPIGetElementByteTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.Object {}
        .function i8 std.core.ETSGLOBAL.ValueAPIGetElementByte(std.core.Object a0, i64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_GET_ELEMENT_BYTE, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.ValueAPIGetElementByte() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPIGetElementCharTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.Object {}
        .function u16 std.core.ETSGLOBAL.ValueAPIGetElementChar(std.core.Object a0, i64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_GET_ELEMENT_CHAR, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.ValueAPIGetElementChar() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPIGetElementShortTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.Object {}
        .function i16 std.core.ETSGLOBAL.ValueAPIGetElementShort(std.core.Object a0, i64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_GET_ELEMENT_SHORT, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.ValueAPIGetElementShort() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPIGetElementIntTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.Object {}
        .function i32 std.core.ETSGLOBAL.ValueAPIGetElementInt(std.core.Object a0, i64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_GET_ELEMENT_INT, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.ValueAPIGetElementInt() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPIGetElementLongTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.Object {}
        .function i64 std.core.ETSGLOBAL.ValueAPIGetElementLong(std.core.Object a0, i64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_GET_ELEMENT_LONG, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.ValueAPIGetElementLong() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPIGetElementFloatTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.Object {}
        .function f32 std.core.ETSGLOBAL.ValueAPIGetElementFloat(std.core.Object a0, i64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_GET_ELEMENT_FLOAT, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.ValueAPIGetElementFloat() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, ValueAPIGetElementDoubleTest)
{
    std::string source = R"(
        .record std.core.ETSGLOBAL {}
        .record std.core.Object {}
        .function f64 std.core.ETSGLOBAL.ValueAPIGetElementDouble(std.core.Object a0, i64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_VALUE_API_GET_ELEMENT_DOUBLE, "std.core.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.ValueAPIGetElementDouble() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, EscompatJSONStringifyFastTest)
{
    std::string source = R"(
        .record escompat.JSON {}
        .record std.core.String {}
        .record std.core.Object {}
        .function std.core.String escompat.JSON.stringifyFast(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_ESCOMPAT_JSON_STRINGIFY_FAST, "escompat.JSON");

    source = R"(
        .record escompat.JSON {}
        .function void escompat.JSON.stringifyFast() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "escompat.JSON");
}

TEST_F(RuntimeAdapterStaticTest, JSONStringifyTest)
{
    std::string source = R"(
        .record escompat.JSON {}
        .record std.core.String {}
        .record std.interop.js.JSValue {}
        .function std.core.String escompat.JSON.stringifyJSValue(std.interop.js.JSValue a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JSON_STRINGIFY, "escompat.JSON");

    source = R"(
        .record escompat.JSON {}
        .function void escompat.JSON.stringifyJSValue() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "escompat.JSON");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreStringCodePointToCharTest)
{
    std::string source = R"(
        .record std.core.String {}
        .function i32 std.core.String.codePointToChar(i32 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_CODE_POINT_TO_CHAR,
                 "std.core.String");

    source = R"(
        .record std.core.String {}
        .function void std.core.String.codePointToChar() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.String");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreStringConcat2Test)
{
    std::string source = R"(
        .record std.core.String {}
        .function void std.core.String.concat2() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.String");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreStringConcat3Test)
{
    std::string source = R"(
        .record std.core.String {}
        .function void std.core.String.concat3() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.String");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreStringConcat4Test)
{
    std::string source = R"(
        .record std.core.String {}
        .function void std.core.String.concat4() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.String");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreStringFromCharCodeTest)
{
    std::string source = R"(
        .record std.core.String {}
        .record escompat.Array {}
        .function std.core.String std.core.String.fromCharCode(escompat.Array a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_FROM_CHAR_CODE, "std.core.String");

    source = R"(
        .record std.core.String {}
        .function void std.core.String.fromCharCode() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.String");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreStringFromCharCodeSingleTest)
{
    std::string source = R"(
        .record std.core.String {}
        .function std.core.String std.core.String.fromCharCode(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_FROM_CHAR_CODE_SINGLE,
                 "std.core.String");

    source = R"(
        .record std.core.String {}
        .function void std.core.String.fromCharCode() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.String");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreToStringBooleanTest)
{
    std::string source = R"(
        .record std.core.StringBuilder {}
        .record std.core.String {}
        .function std.core.String std.core.StringBuilder.toString(u1 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_TO_STRING_BOOLEAN, "std.core.StringBuilder");

    source = R"(
        .record std.core.StringBuilder {}
        .function void std.core.StringBuilder.toString() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.StringBuilder");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreToStringByteTest)
{
    std::string source = R"(
        .record std.core.StringBuilder {}
        .record std.core.String {}
        .function std.core.String std.core.StringBuilder.toString(i8 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_TO_STRING_BYTE, "std.core.StringBuilder");

    source = R"(
        .record std.core.StringBuilder {}
        .function void std.core.StringBuilder.toString() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.StringBuilder");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreToStringCharTest)
{
    std::string source = R"(
        .record std.core.StringBuilder {}
        .record std.core.String {}
        .function std.core.String std.core.StringBuilder.toString(u16 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_TO_STRING_CHAR, "std.core.StringBuilder");

    source = R"(
        .record std.core.StringBuilder {}
        .function void std.core.StringBuilder.toString() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.StringBuilder");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreToStringShortTest)
{
    std::string source = R"(
        .record std.core.StringBuilder {}
        .record std.core.String {}
        .function std.core.String std.core.StringBuilder.toString(i16 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_TO_STRING_SHORT, "std.core.StringBuilder");

    source = R"(
        .record std.core.StringBuilder {}
        .function void std.core.StringBuilder.toString() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.StringBuilder");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreToStringIntTest)
{
    std::string source = R"(
        .record std.core.StringBuilder {}
        .record std.core.String {}
        .function std.core.String std.core.StringBuilder.toString(i32 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_TO_STRING_INT, "std.core.StringBuilder");

    source = R"(
        .record std.core.StringBuilder {}
        .function void std.core.StringBuilder.toString() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.StringBuilder");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreToStringLongTest)
{
    std::string source = R"(
        .record std.core.StringBuilder {}
        .record std.core.String {}
        .function std.core.String std.core.StringBuilder.toString(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_TO_STRING_LONG, "std.core.StringBuilder");

    source = R"(
        .record std.core.StringBuilder {}
        .function void std.core.StringBuilder.toString() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.StringBuilder");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreMathMaxTest)
{
    std::string source = R"(
        .record std.core.Math {}
        .function f64 std.core.Math.max(f64 a0, f64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_MATH_MAX, "std.core.Math");

    source = R"(
        .record std.core.Math {}
        .function void std.core.Math.max() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Math");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreMathMinTest)
{
    std::string source = R"(
        .record std.core.Math {}
        .function f64 std.core.Math.min(f64 a0, f64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_MATH_MIN, "std.core.Math");

    source = R"(
        .record std.core.Math {}
        .function void std.core.Math.min() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Math");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreMathRandomTest)
{
    std::string source = R"(
        .record std.core.Math {}
        .function f64 std.core.Math.random() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_MATH_RANDOM, "std.core.Math");

    source = R"(
        .record std.core.Math {}
        .function void std.core.Math.random() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Math");
}

TEST_F(RuntimeAdapterStaticTest, EscompatJSONGetJSONStringifyIgnoreByIdxTest)
{
    std::string source = R"(
        .record escompat.JSONAPI {}
        .record std.core.Class {}
        .function u1 escompat.JSONAPI.getJSONStringifyIgnoreByIdx(std.core.Class a0, i64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_ESCOMPAT_JSON_GET_JSON_STRINGIFY_IGNORE_BY_IDX,
                 "escompat.JSONAPI");

    source = R"(
        .record escompat.JSONAPI {}
        .function void escompat.JSONAPI.getJSONStringifyIgnoreByIdx() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "escompat.JSONAPI");
}

TEST_F(RuntimeAdapterStaticTest, EscompatJSONGetJSONStringifyIgnoreByNameTest)
{
    std::string source = R"(
        .record escompat.JSONAPI {}
        .record std.core.String {}
        .record std.core.Class {}
        .function u1 escompat.JSONAPI.getJSONStringifyIgnoreByName(std.core.Class a0, std.core.String a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_ESCOMPAT_JSON_GET_JSON_STRINGIFY_IGNORE_BY_NAME,
                 "escompat.JSONAPI");

    source = R"(
        .record escompat.JSONAPI {}
        .function void escompat.JSONAPI.getJSONStringifyIgnoreByName() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "escompat.JSONAPI");
}

TEST_F(RuntimeAdapterStaticTest, EscompatJSONGetJSONParseIgnoreFromAnnotationTest)
{
    std::string source = R"(
        .record escompat.JSONAPI {}
        .record std.core.Class {}
        .function u1 escompat.JSONAPI.getJSONParseIgnoreFromAnnotation(std.core.Class a0, i64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_ESCOMPAT_JSON_GET_JSON_PARSE_IGNORE_FROM_ANNOTATION,
                 "escompat.JSONAPI");

    source = R"(
        .record escompat.JSONAPI {}
        .function void escompat.JSONAPI.getJSONParseIgnoreFromAnnotation() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "escompat.JSONAPI");
}

TEST_F(RuntimeAdapterStaticTest, EscompatJSONGetJSONRenameByIdxTest)
{
    std::string source = R"(
        .record escompat.JSONAPI {}
        .record std.core.String {}
        .record std.core.Class {}
        .function std.core.String escompat.JSONAPI.getJSONRenameByIdx(std.core.Class a0, i64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_ESCOMPAT_JSON_GET_JSON_RENAME_BY_IDX,
                 "escompat.JSONAPI");

    source = R"(
        .record escompat.JSONAPI {}
        .function void escompat.JSONAPI.getJSONRenameByIdx() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "escompat.JSONAPI");
}

TEST_F(RuntimeAdapterStaticTest, EscompatJSONGetJSONRenameByNameTest)
{
    std::string source = R"(
        .record escompat.JSONAPI {}
        .record std.core.String {}
        .record std.core.Class {}
        .function std.core.String escompat.JSONAPI.getJSONRenameByName(std.core.Class a0, std.core.String a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_ESCOMPAT_JSON_GET_JSON_RENAME_BY_NAME,
                 "escompat.JSONAPI");

    source = R"(
        .record escompat.JSONAPI {}
        .function void escompat.JSONAPI.getJSONRenameByName() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "escompat.JSONAPI");
}

TEST_F(RuntimeAdapterStaticTest, EscompatDateNowTest)
{
    std::string source = R"(
        .record escompat.Date {}
        .function i64 escompat.Date.now() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_ESCOMPAT_DATE_NOW, "escompat.Date");

    source = R"(
        .record escompat.Date {}
        .function void escompat.Date.now() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "escompat.Date");
}

TEST_F(RuntimeAdapterStaticTest, EscompatDateGetLocalTimezoneOffsetTest)
{
    std::string source = R"(
        .record escompat.Date {}
        .function i64 escompat.Date.getLocalTimezoneOffset(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_ESCOMPAT_DATE_GET_LOCAL_TIMEZONE_OFFSET,
                 "escompat.Date");

    source = R"(
        .record escompat.Date {}
        .function void escompat.Date.getLocalTimezoneOffset() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "escompat.Date");
}

TEST_F(RuntimeAdapterStaticTest, EscompatDateGetTimezoneNameTest)
{
    std::string source = R"(
        .record escompat.Date {}
        .record std.core.String {}
        .function std.core.String escompat.Date.getTimezoneName(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_ESCOMPAT_DATE_GET_TIMEZONE_NAME, "escompat.Date");

    source = R"(
        .record escompat.Date {}
        .function void escompat.Date.getTimezoneName() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "escompat.Date");
}

TEST_F(RuntimeAdapterStaticTest, EscompatArrayIsPlatformArrayTest)
{
    std::string source = R"(
        .record escompat.Array {}
        .record std.core.Object {}
        .function u1 escompat.Array.isPlatformArray(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_ESCOMPAT_ARRAY_IS_PLATFORM_ARRAY, "escompat.Array");

    source = R"(
        .record escompat.Array {}
        .function void escompat.Array.isPlatformArray() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "escompat.Array");
}

TEST_F(RuntimeAdapterStaticTest, EscompatArrayGetBufferTest)
{
    std::string source = R"(
        .record escompat.Array {}
        .record std.core.Object {}
        .function std.core.Object[] escompat.Array.getBuffer(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_ESCOMPAT_ARRAY_GET_BUFFER, "escompat.Array");

    source = R"(
        .record escompat.Array {}
        .function void escompat.Array.getBuffer() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "escompat.Array");
}

TEST_F(RuntimeAdapterStaticTest, EscompatArrayBufferAllocateNonMovableTest)
{
    std::string source = R"(
        .record std.core.ArrayBuffer {}
        .function i8[] std.core.ArrayBuffer.allocateNonMovable(i32 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_ESCOMPAT_ARRAY_BUFFER_ALLOCATE_NON_MOVABLE,
                 "std.core.ArrayBuffer");

    source = R"(
        .record std.core.ArrayBuffer {}
        .function void std.core.ArrayBuffer.allocateNonMovable() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ArrayBuffer");
}

TEST_F(RuntimeAdapterStaticTest, EscompatArrayBufferGetAddressTest)
{
    std::string source = R"(
        .record std.core.ArrayBuffer {}
        .function i64 std.core.ArrayBuffer.getAddress(i8[] a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_ESCOMPAT_ARRAY_BUFFER_GET_ADDRESS,
                 "std.core.ArrayBuffer");

    source = R"(
        .record std.core.ArrayBuffer {}
        .function void std.core.ArrayBuffer.getAddress() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ArrayBuffer");
}

TEST_F(RuntimeAdapterStaticTest, EscompatArrayBufferFromEncodedStringTest)
{
    std::string source = R"(
        .record std.core.ArrayBuffer {}
        .record std.core.String {}
        .function std.core.ArrayBuffer std.core.ArrayBuffer.from(std.core.String a0, std.core.String a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_ESCOMPAT_ARRAY_BUFFER_FROM_ENCODED_STRING,
                 "std.core.ArrayBuffer");

    source = R"(
        .record std.core.ArrayBuffer {}
        .function void std.core.ArrayBuffer.from() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ArrayBuffer");
}

TEST_F(RuntimeAdapterStaticTest, EscompatArrayBufferFromBufferSliceTest)
{
    std::string source = R"(
        .record std.core.ArrayBuffer {}
        .function void std.core.ArrayBuffer.from() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ArrayBuffer");
}

TEST_F(RuntimeAdapterStaticTest, EtsStringBytesLengthTest)
{
    std::string source = R"(
        .record std.core.ArrayBuffer {}
        .record std.core.String {}
        .function i32 std.core.ArrayBuffer.bytesLength(std.core.String a0, std.core.String a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_ETS_STRING_BYTES_LENGTH, "std.core.ArrayBuffer");

    source = R"(
        .record std.core.ArrayBuffer {}
        .function void std.core.ArrayBuffer.bytesLength() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ArrayBuffer");
}

TEST_F(RuntimeAdapterStaticTest, EtsArrayBufferToStringTest)
{
    std::string source = R"(
        .record std.core.ArrayBuffer {}
        .function void std.core.ArrayBuffer.stringify() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ArrayBuffer");
}

TEST_F(RuntimeAdapterStaticTest, EtsArrayBufferRegisterWeakRefTest)
{
    std::string source = R"(
        .record std.core.ArrayBuffer {}
        .function void std.core.ArrayBuffer.registerWeakRef() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ArrayBuffer");
}

TEST_F(RuntimeAdapterStaticTest, EtsArrayBufferUnregisterWeakRefTest)
{
    std::string source = R"(
        .record std.core.ArrayBuffer {}
        .record std.core.Object {}
        .function i64 std.core.ArrayBuffer.unregisterWeakRef(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_ETS_ARRAY_BUFFER_UNREGISTER_WEAK_REF,
                 "std.core.ArrayBuffer");

    source = R"(
        .record std.core.ArrayBuffer {}
        .function void std.core.ArrayBuffer.unregisterWeakRef() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ArrayBuffer");
}

TEST_F(RuntimeAdapterStaticTest, Uint8ClampedArrayToUint8ClampedTest)
{
    std::string source = R"(
        .record escompat.Uint8ClampedArray {}
        .function i32 escompat.Uint8ClampedArray.toUint8Clamped(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_UINT8_CLAMPED_ARRAY_TO_UINT8_CLAMPED,
                 "escompat.Uint8ClampedArray");

    source = R"(
        .record escompat.Uint8ClampedArray {}
        .function void escompat.Uint8ClampedArray.toUint8Clamped() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "escompat.Uint8ClampedArray");
}

TEST_F(RuntimeAdapterStaticTest, TaskpoolGenerateIdTest)
{
    std::string source = R"(
        .record std.concurrency.taskpool.Task {}
        .function i32 std.concurrency.taskpool.Task.generateId() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TASKPOOL_GENERATE_ID,
                 "std.concurrency.taskpool.Task");

    source = R"(
        .record std.concurrency.taskpool.Task {}
        .function void std.concurrency.taskpool.Task.generateId() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.concurrency.taskpool.Task");
}

TEST_F(RuntimeAdapterStaticTest, TaskpoolGenerateGroupIdTest)
{
    std::string source = R"(
        .record std.concurrency.taskpool.GroupState {}
        .function i32 std.concurrency.taskpool.GroupState.generateGroupId() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TASKPOOL_GENERATE_GROUP_ID,
                 "std.concurrency.taskpool.GroupState");

    source = R"(
        .record std.concurrency.taskpool.GroupState {}
        .function void std.concurrency.taskpool.GroupState.generateGroupId() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.concurrency.taskpool.GroupState");
}

TEST_F(RuntimeAdapterStaticTest, TaskpoolIsUsingLaunchTest)
{
    std::string source = R"(
        .record std.concurrency.taskpool {}
        .function u1 std.concurrency.taskpool.isUsingLaunch() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TASKPOOL_IS_USING_LAUNCH, "std.concurrency.taskpool");

    source = R"(
        .record std.concurrency.taskpool {}
        .function void std.concurrency.taskpool.isUsingLaunch() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.concurrency.taskpool");
}

TEST_F(RuntimeAdapterStaticTest, TaskpoolIsSupportInteropTest)
{
    std::string source = R"(
        .record std.concurrency.taskpool {}
        .function u1 std.concurrency.taskpool.isSupportingInterop() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TASKPOOL_IS_SUPPORT_INTEROP,
                 "std.concurrency.taskpool");

    source = R"(
        .record std.concurrency.taskpool {}
        .function void std.concurrency.taskpool.isSupportingInterop() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.concurrency.taskpool");
}

TEST_F(RuntimeAdapterStaticTest, TaskpoolGetTaskPoolWorkersLimitTest)
{
    std::string source = R"(
        .record std.concurrency.taskpool {}
        .function i32 std.concurrency.taskpool.getTaskPoolWorkersLimit() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TASKPOOL_GET_TASK_POOL_WORKERS_LIMIT,
                 "std.concurrency.taskpool");

    source = R"(
        .record std.concurrency.taskpool {}
        .function void std.concurrency.taskpool.getTaskPoolWorkersLimit() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.concurrency.taskpool");
}

TEST_F(RuntimeAdapterStaticTest, TaskpoolGenerateSequenceRunnerIdTest)
{
    std::string source = R"(
        .record std.concurrency.taskpool.SequenceRunner {}
        .function i32 std.concurrency.taskpool.SequenceRunner.generateSeqRunnerId() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TASKPOOL_GENERATE_SEQUENCE_RUNNER_ID,
                 "std.concurrency.taskpool.SequenceRunner");

    source = R"(
        .record std.concurrency.taskpool.SequenceRunner {}
        .function void std.concurrency.taskpool.SequenceRunner.generateSeqRunnerId() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.concurrency.taskpool.SequenceRunner");
}

TEST_F(RuntimeAdapterStaticTest, TaskpoolGenerateAsyncRunnerIdTest)
{
    std::string source = R"(
        .record std.concurrency.taskpool.AsyncRunner {}
        .function i32 std.concurrency.taskpool.AsyncRunner.generateAsyncRunnerId() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TASKPOOL_GENERATE_ASYNC_RUNNER_ID,
                 "std.concurrency.taskpool.AsyncRunner");

    source = R"(
        .record std.concurrency.taskpool.AsyncRunner {}
        .function void std.concurrency.taskpool.AsyncRunner.generateAsyncRunnerId() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.concurrency.taskpool.AsyncRunner");
}

TEST_F(RuntimeAdapterStaticTest, StdTimeDateNanoNowTest)
{
    std::string source = R"(
        .record std.time.Chrono {}
        .function i64 std.time.Chrono.nanoNow() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_TIME_DATE_NANO_NOW, "std.time.Chrono");

    source = R"(
        .record std.time.Chrono {}
        .function void std.time.Chrono.nanoNow() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.time.Chrono");
}

TEST_F(RuntimeAdapterStaticTest, StdTimeChronoGetCpuTimeTest)
{
    std::string source = R"(
        .record std.time.Chrono {}
        .function i64 std.time.Chrono.getCpuTime() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_TIME_CHRONO_GET_CPU_TIME, "std.time.Chrono");

    source = R"(
        .record std.time.Chrono {}
        .function void std.time.Chrono.getCpuTime() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.time.Chrono");
}

TEST_F(RuntimeAdapterStaticTest, StdFloatToStringTest)
{
    std::string source = R"(
        .record std.core.Float {}
        .record std.core.String {}
        .function std.core.String std.core.Float.toString(f32 a0, i32 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_FLOAT_TO_STRING, "std.core.Float");

    source = R"(
        .record std.core.Float {}
        .function void std.core.Float.toString() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Float");
}

TEST_F(RuntimeAdapterStaticTest, StdFloatIsNanTest)
{
    std::string source = R"(
        .record std.core.Float {}
        .function u1 std.core.Float.isNaN(f32 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_FLOAT_IS_NAN, "std.core.Float");

    source = R"(
        .record std.core.Float {}
        .function void std.core.Float.isNaN() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Float");
}

TEST_F(RuntimeAdapterStaticTest, StdFloatIsFiniteTest)
{
    std::string source = R"(
        .record std.core.Float {}
        .function u1 std.core.Float.isFinite(f32 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_FLOAT_IS_FINITE, "std.core.Float");

    source = R"(
        .record std.core.Float {}
        .function void std.core.Float.isFinite() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Float");
}

TEST_F(RuntimeAdapterStaticTest, StdFloatBitCastFromIntTest)
{
    std::string source = R"(
        .record std.core.Float {}
        .function f32 std.core.Float.bitCastFromInt(i32 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_FLOAT_BIT_CAST_FROM_INT, "std.core.Float");

    source = R"(
        .record std.core.Float {}
        .function void std.core.Float.bitCastFromInt() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Float");
}

TEST_F(RuntimeAdapterStaticTest, StdFloatBitCastToIntTest)
{
    std::string source = R"(
        .record std.core.Float {}
        .function i32 std.core.Float.bitCastToInt(f32 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_FLOAT_BIT_CAST_TO_INT, "std.core.Float");

    source = R"(
        .record std.core.Float {}
        .function void std.core.Float.bitCastToInt() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Float");
}

TEST_F(RuntimeAdapterStaticTest, StdFloatIsIntegerTest)
{
    std::string source = R"(
        .record std.core.Float {}
        .function u1 std.core.Float.isInteger(f32 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_FLOAT_IS_INTEGER, "std.core.Float");

    source = R"(
        .record std.core.Float {}
        .function void std.core.Float.isInteger() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Float");
}

TEST_F(RuntimeAdapterStaticTest, StdFloatIsSafeIntegerTest)
{
    std::string source = R"(
        .record std.core.Float {}
        .function u1 std.core.Float.isSafeInteger(f32 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_FLOAT_IS_SAFE_INTEGER, "std.core.Float");

    source = R"(
        .record std.core.Float {}
        .function void std.core.Float.isSafeInteger() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Float");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreFloatToShortTest)
{
    std::string source = R"(
        .record std.core.Float {}
        .function i16 std.core.Float.toShort(f32 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_FLOAT_TO_SHORT, "std.core.Float");

    source = R"(
        .record std.core.Float {}
        .function void std.core.Float.toShort() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Float");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreFloatToByteTest)
{
    std::string source = R"(
        .record std.core.Float {}
        .function i8 std.core.Float.toByte(f32 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_FLOAT_TO_BYTE, "std.core.Float");

    source = R"(
        .record std.core.Float {}
        .function void std.core.Float.toByte() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Float");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreFloatToIntTest)
{
    std::string source = R"(
        .record std.core.Float {}
        .function i32 std.core.Float.toInt(f32 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_FLOAT_TO_INT, "std.core.Float");

    source = R"(
        .record std.core.Float {}
        .function void std.core.Float.toInt() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Float");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreFloatToLongTest)
{
    std::string source = R"(
        .record std.core.Float {}
        .function i64 std.core.Float.toLong(f32 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_FLOAT_TO_LONG, "std.core.Float");

    source = R"(
        .record std.core.Float {}
        .function void std.core.Float.toLong() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Float");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreFloatToDoubleTest)
{
    std::string source = R"(
        .record std.core.Float {}
        .function f64 std.core.Float.toDouble(f32 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_FLOAT_TO_DOUBLE, "std.core.Float");

    source = R"(
        .record std.core.Float {}
        .function void std.core.Float.toDouble() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Float");
}

TEST_F(RuntimeAdapterStaticTest, StdDoubleToStringTest)
{
    std::string source = R"(
        .record std.core.Double {}
        .record std.core.String {}
        .function std.core.String std.core.Double.toString(f64 a0, i32 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_DOUBLE_TO_STRING, "std.core.Double");

    source = R"(
        .record std.core.Double {}
        .function void std.core.Double.toString() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Double");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreDoubleParseFloatTest)
{
    std::string source = R"(
        .record std.core.Double {}
        .record std.core.String {}
        .function f64 std.core.Double.parseFloat(std.core.String a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_DOUBLE_PARSE_FLOAT, "std.core.Double");

    source = R"(
        .record std.core.Double {}
        .function void std.core.Double.parseFloat() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Double");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreDoubleParseIntTest)
{
    std::string source = R"(
        .record std.core.Double {}
        .record std.core.String {}
        .function f64 std.core.Double.parseIntCore(std.core.String a0, i32 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_DOUBLE_PARSE_INT, "std.core.Double");

    source = R"(
        .record std.core.Double {}
        .function void std.core.Double.parseIntCore() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Double");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreDoubleNumberFromStringTest)
{
    std::string source = R"(
        .record std.core.Double {}
        .record std.core.String {}
        .function f64 std.core.Double.numberFromString(std.core.String a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_DOUBLE_NUMBER_FROM_STRING,
                 "std.core.Double");

    source = R"(
        .record std.core.Double {}
        .function void std.core.Double.numberFromString() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Double");
}

TEST_F(RuntimeAdapterStaticTest, StdDoubleIsNanTest)
{
    std::string source = R"(
        .record std.core.Double {}
        .function u1 std.core.Double.isNaN(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_DOUBLE_IS_NAN, "std.core.Double");

    source = R"(
        .record std.core.Double {}
        .function void std.core.Double.isNaN() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Double");
}

TEST_F(RuntimeAdapterStaticTest, StdDoubleIsFiniteTest)
{
    std::string source = R"(
        .record std.core.Double {}
        .function u1 std.core.Double.isFinite(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_DOUBLE_IS_FINITE, "std.core.Double");

    source = R"(
        .record std.core.Double {}
        .function void std.core.Double.isFinite() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Double");
}

TEST_F(RuntimeAdapterStaticTest, StdDoubleBitCastFromLongTest)
{
    std::string source = R"(
        .record std.core.Double {}
        .function f64 std.core.Double.bitCastFromLong(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_DOUBLE_BIT_CAST_FROM_LONG, "std.core.Double");

    source = R"(
        .record std.core.Double {}
        .function void std.core.Double.bitCastFromLong() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Double");
}

TEST_F(RuntimeAdapterStaticTest, StdDoubleBitCastToLongTest)
{
    std::string source = R"(
        .record std.core.Double {}
        .function i64 std.core.Double.bitCastToLong(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_DOUBLE_BIT_CAST_TO_LONG, "std.core.Double");

    source = R"(
        .record std.core.Double {}
        .function void std.core.Double.bitCastToLong() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Double");
}

TEST_F(RuntimeAdapterStaticTest, StdDoubleIsIntegerTest)
{
    std::string source = R"(
        .record std.core.Double {}
        .function u1 std.core.Double.isInteger(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_DOUBLE_IS_INTEGER, "std.core.Double");

    source = R"(
        .record std.core.Double {}
        .function void std.core.Double.isInteger() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Double");
}

TEST_F(RuntimeAdapterStaticTest, StdDoubleIsSafeIntegerTest)
{
    std::string source = R"(
        .record std.core.Double {}
        .function u1 std.core.Double.isSafeInteger(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_DOUBLE_IS_SAFE_INTEGER, "std.core.Double");

    source = R"(
        .record std.core.Double {}
        .function void std.core.Double.isSafeInteger() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Double");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreDoubleToShortTest)
{
    std::string source = R"(
        .record std.core.Double {}
        .function i16 std.core.Double.toShort(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_DOUBLE_TO_SHORT, "std.core.Double");

    source = R"(
        .record std.core.Double {}
        .function void std.core.Double.toShort() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Double");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreDoubleToByteTest)
{
    std::string source = R"(
        .record std.core.Double {}
        .function i8 std.core.Double.toByte(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_DOUBLE_TO_BYTE, "std.core.Double");

    source = R"(
        .record std.core.Double {}
        .function void std.core.Double.toByte() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Double");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreDoubleToIntTest)
{
    std::string source = R"(
        .record std.core.Double {}
        .function i32 std.core.Double.toInt(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_DOUBLE_TO_INT, "std.core.Double");

    source = R"(
        .record std.core.Double {}
        .function void std.core.Double.toInt() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Double");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreDoubleToLongTest)
{
    std::string source = R"(
        .record std.core.Double {}
        .function i64 std.core.Double.toLong(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_DOUBLE_TO_LONG, "std.core.Double");

    source = R"(
        .record std.core.Double {}
        .function void std.core.Double.toLong() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Double");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreDoubleToFloatTest)
{
    std::string source = R"(
        .record std.core.Double {}
        .function f32 std.core.Double.toFloat(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_DOUBLE_TO_FLOAT, "std.core.Double");

    source = R"(
        .record std.core.Double {}
        .function void std.core.Double.toFloat() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Double");
}

TEST_F(RuntimeAdapterStaticTest, Int32ArrayDoubleToIntTest)
{
    std::string source = R"(
        .record escompat.Int32Array {}
        .function i32 escompat.Int32Array.doubleToInt(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_INT32_ARRAY_DOUBLE_TO_INT, "escompat.Int32Array");

    source = R"(
        .record escompat.Int32Array {}
        .function void escompat.Int32Array.doubleToInt() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "escompat.Int32Array");
}

TEST_F(RuntimeAdapterStaticTest, Int16ArrayDoubleToIntTest)
{
    std::string source = R"(
        .record escompat.Int16Array {}
        .function i16 escompat.Int16Array.doubleToInt(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_INT16_ARRAY_DOUBLE_TO_INT, "escompat.Int16Array");

    source = R"(
        .record escompat.Int16Array {}
        .function void escompat.Int16Array.doubleToInt() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "escompat.Int16Array");
}

TEST_F(RuntimeAdapterStaticTest, Int8ArrayDoubleToIntTest)
{
    std::string source = R"(
        .record escompat.Int8Array {}
        .function i8 escompat.Int8Array.doubleToInt(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_INT8_ARRAY_DOUBLE_TO_INT, "escompat.Int8Array");

    source = R"(
        .record escompat.Int8Array {}
        .function void escompat.Int8Array.doubleToInt() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "escompat.Int8Array");
}

TEST_F(RuntimeAdapterStaticTest, Uint32ArrayDoubleToIntTest)
{
    std::string source = R"(
        .record escompat.Uint32Array {}
        .function i64 escompat.Uint32Array.doubleToInt(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_UINT32_ARRAY_DOUBLE_TO_INT, "escompat.Uint32Array");

    source = R"(
        .record escompat.Uint32Array {}
        .function void escompat.Uint32Array.doubleToInt() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "escompat.Uint32Array");
}

TEST_F(RuntimeAdapterStaticTest, Uint16ArrayDoubleToIntTest)
{
    std::string source = R"(
        .record escompat.Uint16Array {}
        .function i32 escompat.Uint16Array.doubleToInt(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_UINT16_ARRAY_DOUBLE_TO_INT, "escompat.Uint16Array");

    source = R"(
        .record escompat.Uint16Array {}
        .function void escompat.Uint16Array.doubleToInt() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "escompat.Uint16Array");
}

TEST_F(RuntimeAdapterStaticTest, Uint8ArrayDoubleToIntTest)
{
    std::string source = R"(
        .record escompat.Uint8Array {}
        .function i32 escompat.Uint8Array.doubleToInt(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_UINT8_ARRAY_DOUBLE_TO_INT, "escompat.Uint8Array");

    source = R"(
        .record escompat.Uint8Array {}
        .function void escompat.Uint8Array.doubleToInt() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "escompat.Uint8Array");
}

TEST_F(RuntimeAdapterStaticTest, UnsafeMemoryWriteInt8Test)
{
    std::string source = R"(
        .record std.core.unsafeMemory {}
        .function void std.core.unsafeMemory.writeInt8(i64 a0, i8 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_UNSAFE_MEMORY_WRITE_INT8, "std.core.unsafeMemory");

    source = R"(
        .record std.core.unsafeMemory {}
        .function i32 std.core.unsafeMemory.writeInt8() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.unsafeMemory");
}

TEST_F(RuntimeAdapterStaticTest, UnsafeMemoryWriteInt16Test)
{
    std::string source = R"(
        .record std.core.unsafeMemory {}
        .function void std.core.unsafeMemory.writeInt16(i64 a0, i16 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_UNSAFE_MEMORY_WRITE_INT16, "std.core.unsafeMemory");

    source = R"(
        .record std.core.unsafeMemory {}
        .function i32 std.core.unsafeMemory.writeInt16() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.unsafeMemory");
}

TEST_F(RuntimeAdapterStaticTest, UnsafeMemoryWriteInt32Test)
{
    std::string source = R"(
        .record std.core.unsafeMemory {}
        .function void std.core.unsafeMemory.writeInt32(i64 a0, i32 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_UNSAFE_MEMORY_WRITE_INT32, "std.core.unsafeMemory");

    source = R"(
        .record std.core.unsafeMemory {}
        .function i32 std.core.unsafeMemory.writeInt32() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.unsafeMemory");
}

TEST_F(RuntimeAdapterStaticTest, UnsafeMemoryWriteInt64Test)
{
    std::string source = R"(
        .record std.core.unsafeMemory {}
        .function void std.core.unsafeMemory.writeInt64(i64 a0, i64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_UNSAFE_MEMORY_WRITE_INT64, "std.core.unsafeMemory");

    source = R"(
        .record std.core.unsafeMemory {}
        .function i32 std.core.unsafeMemory.writeInt64() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.unsafeMemory");
}

TEST_F(RuntimeAdapterStaticTest, UnsafeMemoryWriteBooleanTest)
{
    std::string source = R"(
        .record std.core.unsafeMemory {}
        .function void std.core.unsafeMemory.writeBoolean(i64 a0, u1 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_UNSAFE_MEMORY_WRITE_BOOLEAN, "std.core.unsafeMemory");

    source = R"(
        .record std.core.unsafeMemory {}
        .function i32 std.core.unsafeMemory.writeBoolean() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.unsafeMemory");
}

TEST_F(RuntimeAdapterStaticTest, UnsafeMemoryWriteFloat32Test)
{
    std::string source = R"(
        .record std.core.unsafeMemory {}
        .function void std.core.unsafeMemory.writeFloat32(i64 a0, f32 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_UNSAFE_MEMORY_WRITE_FLOAT32, "std.core.unsafeMemory");

    source = R"(
        .record std.core.unsafeMemory {}
        .function i32 std.core.unsafeMemory.writeFloat32() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.unsafeMemory");
}

TEST_F(RuntimeAdapterStaticTest, UnsafeMemoryWriteFloat64Test)
{
    std::string source = R"(
        .record std.core.unsafeMemory {}
        .function void std.core.unsafeMemory.writeFloat64(i64 a0, f64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_UNSAFE_MEMORY_WRITE_FLOAT64, "std.core.unsafeMemory");

    source = R"(
        .record std.core.unsafeMemory {}
        .function i32 std.core.unsafeMemory.writeFloat64() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.unsafeMemory");
}

TEST_F(RuntimeAdapterStaticTest, UnsafeMemoryWriteNumberTest)
{
    std::string source = R"(
        .record std.core.unsafeMemory {}
        .function void std.core.unsafeMemory.writeNumber(i64 a0, f64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_UNSAFE_MEMORY_WRITE_NUMBER, "std.core.unsafeMemory");

    source = R"(
        .record std.core.unsafeMemory {}
        .function i32 std.core.unsafeMemory.writeNumber() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.unsafeMemory");
}

TEST_F(RuntimeAdapterStaticTest, UnsafeMemoryWriteStringTest)
{
    std::string source = R"(
        .record std.core.unsafeMemory {}
        .record std.core.String {}
        .function i32 std.core.unsafeMemory.writeString(i64 a0, std.core.String a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_UNSAFE_MEMORY_WRITE_STRING, "std.core.unsafeMemory");

    source = R"(
        .record std.core.unsafeMemory {}
        .function void std.core.unsafeMemory.writeString() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.unsafeMemory");
}

TEST_F(RuntimeAdapterStaticTest, UnsafeMemoryReadBooleanTest)
{
    std::string source = R"(
        .record std.core.unsafeMemory {}
        .function u1 std.core.unsafeMemory.readBoolean(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_UNSAFE_MEMORY_READ_BOOLEAN, "std.core.unsafeMemory");

    source = R"(
        .record std.core.unsafeMemory {}
        .function void std.core.unsafeMemory.readBoolean() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.unsafeMemory");
}

TEST_F(RuntimeAdapterStaticTest, UnsafeMemoryReadInt8Test)
{
    std::string source = R"(
        .record std.core.unsafeMemory {}
        .function i8 std.core.unsafeMemory.readInt8(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_UNSAFE_MEMORY_READ_INT8, "std.core.unsafeMemory");

    source = R"(
        .record std.core.unsafeMemory {}
        .function void std.core.unsafeMemory.readInt8() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.unsafeMemory");
}

TEST_F(RuntimeAdapterStaticTest, UnsafeMemoryReadInt16Test)
{
    std::string source = R"(
        .record std.core.unsafeMemory {}
        .function i16 std.core.unsafeMemory.readInt16(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_UNSAFE_MEMORY_READ_INT16, "std.core.unsafeMemory");

    source = R"(
        .record std.core.unsafeMemory {}
        .function void std.core.unsafeMemory.readInt16() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.unsafeMemory");
}

TEST_F(RuntimeAdapterStaticTest, UnsafeMemoryReadInt32Test)
{
    std::string source = R"(
        .record std.core.unsafeMemory {}
        .function i32 std.core.unsafeMemory.readInt32(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_UNSAFE_MEMORY_READ_INT32, "std.core.unsafeMemory");

    source = R"(
        .record std.core.unsafeMemory {}
        .function void std.core.unsafeMemory.readInt32() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.unsafeMemory");
}

TEST_F(RuntimeAdapterStaticTest, UnsafeMemoryReadInt64Test)
{
    std::string source = R"(
        .record std.core.unsafeMemory {}
        .function i64 std.core.unsafeMemory.readInt64(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_UNSAFE_MEMORY_READ_INT64, "std.core.unsafeMemory");

    source = R"(
        .record std.core.unsafeMemory {}
        .function void std.core.unsafeMemory.readInt64() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.unsafeMemory");
}

TEST_F(RuntimeAdapterStaticTest, UnsafeMemoryReadFloat32Test)
{
    std::string source = R"(
        .record std.core.unsafeMemory {}
        .function f32 std.core.unsafeMemory.readFloat32(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_UNSAFE_MEMORY_READ_FLOAT32, "std.core.unsafeMemory");

    source = R"(
        .record std.core.unsafeMemory {}
        .function void std.core.unsafeMemory.readFloat32() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.unsafeMemory");
}

TEST_F(RuntimeAdapterStaticTest, UnsafeMemoryReadFloat64Test)
{
    std::string source = R"(
        .record std.core.unsafeMemory {}
        .function f64 std.core.unsafeMemory.readFloat64(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_UNSAFE_MEMORY_READ_FLOAT64, "std.core.unsafeMemory");

    source = R"(
        .record std.core.unsafeMemory {}
        .function void std.core.unsafeMemory.readFloat64() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.unsafeMemory");
}

TEST_F(RuntimeAdapterStaticTest, UnsafeMemoryReadNumberTest)
{
    std::string source = R"(
        .record std.core.unsafeMemory {}
        .function f64 std.core.unsafeMemory.readNumber(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_UNSAFE_MEMORY_READ_NUMBER, "std.core.unsafeMemory");

    source = R"(
        .record std.core.unsafeMemory {}
        .function void std.core.unsafeMemory.readNumber() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.unsafeMemory");
}

TEST_F(RuntimeAdapterStaticTest, UnsafeMemoryReadStringTest)
{
    std::string source = R"(
        .record std.core.unsafeMemory {}
        .record std.core.String {}
        .function std.core.String std.core.unsafeMemory.readString(i64 a0, i32 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_UNSAFE_MEMORY_READ_STRING, "std.core.unsafeMemory");

    source = R"(
        .record std.core.unsafeMemory {}
        .function void std.core.unsafeMemory.readString() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.unsafeMemory");
}

TEST_F(RuntimeAdapterStaticTest, UnsafeMemoryGetStringSizeInBytesTest)
{
    std::string source = R"(
        .record std.core.unsafeMemory {}
        .record std.core.String {}
        .function i32 std.core.unsafeMemory.getStringSizeInBytes(std.core.String a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_UNSAFE_MEMORY_GET_STRING_SIZE_IN_BYTES,
                 "std.core.unsafeMemory");

    source = R"(
        .record std.core.unsafeMemory {}
        .function void std.core.unsafeMemory.getStringSizeInBytes() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.unsafeMemory");
}

TEST_F(RuntimeAdapterStaticTest, EscompatObjectFastCopyToUncheckedTest)
{
    std::string source = R"(
        .record escompat.ETSGLOBAL {}
        .function void escompat.ETSGLOBAL.copyToFast(std.core.Object[] a0, std.core.Object[] a1, i32 a2, i32 a3, i32 a4) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_ESCOMPAT_OBJECT_FAST_COPY_TO_UNCHECKED,
                 "escompat.ETSGLOBAL");

    source = R"(
        .record escompat.ETSGLOBAL {}
        .function i32 escompat.ETSGLOBAL.copyToFast() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "escompat.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreRegisterTimerTest)
{
    std::string source = R"(
        .record escompat.ETSGLOBAL {}
        .record std.core.Object {}
        .function i32 escompat.ETSGLOBAL.registerTimer(std.core.Object a0, i32 a1, u1 a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_REGISTER_TIMER, "escompat.ETSGLOBAL");

    source = R"(
        .record escompat.ETSGLOBAL {}
        .function void escompat.ETSGLOBAL.registerTimer() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "escompat.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreClearTimerTest)
{
    std::string source = R"(
        .record escompat.ETSGLOBAL {}
        .function void escompat.ETSGLOBAL.clearTimer(i32 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_CLEAR_TIMER, "escompat.ETSGLOBAL");

    source = R"(
        .record escompat.ETSGLOBAL {}
        .function i32 escompat.ETSGLOBAL.clearTimer() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "escompat.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreStackTraceProvisionStackTraceTest)
{
    std::string source = R"(
        .record std.core.StackTrace {}
        .function std.core.StackTraceElement[] std.core.StackTrace.provisionStackTrace() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STACK_TRACE_PROVISION_STACK_TRACE,
                 "std.core.StackTrace");

    source = R"(
        .record std.core.StackTrace {}
        .function void std.core.StackTrace.provisionStackTrace() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.StackTrace");
}

TEST_F(RuntimeAdapterStaticTest, StdGCStartGCTest)
{
    std::string source = R"(
        .record std.core.GC {}
        .record std.core.Object {}
        .function i64 std.core.GC.startGCImpl(i32 a0, std.core.Object a1, u1 a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_GC_START_GC, "std.core.GC");

    source = R"(
        .record std.core.GC {}
        .function void std.core.GC.startGCImpl() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.GC");
}

TEST_F(RuntimeAdapterStaticTest, StdGCWaitForFinishGCTest)
{
    std::string source = R"(
        .record std.core.GC {}
        .function void std.core.GC.waitForFinishGC(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_GC_WAIT_FOR_FINISH_GC, "std.core.GC");

    source = R"(
        .record std.core.GC {}
        .function i32 std.core.GC.waitForFinishGC() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.GC");
}

TEST_F(RuntimeAdapterStaticTest, StdGCScheduleGCAfterNthAllocTest)
{
    std::string source = R"(
        .record std.core.GC {}
        .function void std.core.GC.scheduleGcAfterNthAllocImpl(i32 a0, i32 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_GC_SCHEDULE_GC_AFTER_NTH_ALLOC, "std.core.GC");

    source = R"(
        .record std.core.GC {}
        .function i32 std.core.GC.scheduleGcAfterNthAllocImpl() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.GC");
}

TEST_F(RuntimeAdapterStaticTest, StdGCIsScheduledGCTriggeredTest)
{
    std::string source = R"(
        .record std.core.GC {}
        .function u1 std.core.GC.isScheduledGCTriggered() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_GC_IS_SCHEDULED_GC_TRIGGERED, "std.core.GC");

    source = R"(
        .record std.core.GC {}
        .function void std.core.GC.isScheduledGCTriggered() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.GC");
}

TEST_F(RuntimeAdapterStaticTest, StdGCPostponeGCStartTest)
{
    std::string source = R"(
        .record std.core.GC {}
        .function void std.core.GC.postponeGCStart() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_GC_POSTPONE_GC_START, "std.core.GC");

    source = R"(
        .record std.core.GC {}
        .function i32 std.core.GC.postponeGCStart() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.GC");
}

TEST_F(RuntimeAdapterStaticTest, StdGCPostponeGCEndTest)
{
    std::string source = R"(
        .record std.core.GC {}
        .function void std.core.GC.postponeGCEnd() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_GC_POSTPONE_GC_END, "std.core.GC");

    source = R"(
        .record std.core.GC {}
        .function i32 std.core.GC.postponeGCEnd() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.GC");
}

TEST_F(RuntimeAdapterStaticTest, StdGCAllocatePinnedBooleanArrayTest)
{
    std::string source = R"(
        .record std.core.GC {}
        .function u1[] std.core.GC.allocatePinnedBooleanArray(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_GC_ALLOCATE_PINNED_BOOLEAN_ARRAY, "std.core.GC");

    source = R"(
        .record std.core.GC {}
        .function void std.core.GC.allocatePinnedBooleanArray() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.GC");
}

TEST_F(RuntimeAdapterStaticTest, StdGCAllocatePinnedByteArrayTest)
{
    std::string source = R"(
        .record std.core.GC {}
        .function i8[] std.core.GC.allocatePinnedByteArray(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_GC_ALLOCATE_PINNED_BYTE_ARRAY, "std.core.GC");

    source = R"(
        .record std.core.GC {}
        .function void std.core.GC.allocatePinnedByteArray() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.GC");
}

TEST_F(RuntimeAdapterStaticTest, StdGCAllocatePinnedCharArrayTest)
{
    std::string source = R"(
        .record std.core.GC {}
        .function u16[] std.core.GC.allocatePinnedCharArray(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_GC_ALLOCATE_PINNED_CHAR_ARRAY, "std.core.GC");

    source = R"(
        .record std.core.GC {}
        .function void std.core.GC.allocatePinnedCharArray() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.GC");
}

TEST_F(RuntimeAdapterStaticTest, StdGCAllocatePinnedShortArrayTest)
{
    std::string source = R"(
        .record std.core.GC {}
        .function i16[] std.core.GC.allocatePinnedShortArray(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_GC_ALLOCATE_PINNED_SHORT_ARRAY, "std.core.GC");

    source = R"(
        .record std.core.GC {}
        .function void std.core.GC.allocatePinnedShortArray() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.GC");
}

TEST_F(RuntimeAdapterStaticTest, StdGCAllocatePinnedIntArrayTest)
{
    std::string source = R"(
        .record std.core.GC {}
        .function i32[] std.core.GC.allocatePinnedIntArray(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_GC_ALLOCATE_PINNED_INT_ARRAY, "std.core.GC");

    source = R"(
        .record std.core.GC {}
        .function void std.core.GC.allocatePinnedIntArray() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.GC");
}

TEST_F(RuntimeAdapterStaticTest, StdGCAllocatePinnedLongArrayTest)
{
    std::string source = R"(
        .record std.core.GC {}
        .function i64[] std.core.GC.allocatePinnedLongArray(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_GC_ALLOCATE_PINNED_LONG_ARRAY, "std.core.GC");

    source = R"(
        .record std.core.GC {}
        .function void std.core.GC.allocatePinnedLongArray() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.GC");
}

TEST_F(RuntimeAdapterStaticTest, StdGCAllocatePinnedFloatArrayTest)
{
    std::string source = R"(
        .record std.core.GC {}
        .function f32[] std.core.GC.allocatePinnedFloatArray(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_GC_ALLOCATE_PINNED_FLOAT_ARRAY, "std.core.GC");

    source = R"(
        .record std.core.GC {}
        .function void std.core.GC.allocatePinnedFloatArray() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.GC");
}

TEST_F(RuntimeAdapterStaticTest, StdGCAllocatePinnedDoubleArrayTest)
{
    std::string source = R"(
        .record std.core.GC {}
        .function f64[] std.core.GC.allocatePinnedDoubleArray(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_GC_ALLOCATE_PINNED_DOUBLE_ARRAY, "std.core.GC");

    source = R"(
        .record std.core.GC {}
        .function void std.core.GC.allocatePinnedDoubleArray() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.GC");
}

TEST_F(RuntimeAdapterStaticTest, StdGCGetObjectSpaceTypeTest)
{
    std::string source = R"(
        .record std.core.GC {}
        .record std.core.Object {}
        .function i32 std.core.GC.getObjectSpaceTypeImpl(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_GC_GET_OBJECT_SPACE_TYPE, "std.core.GC");

    source = R"(
        .record std.core.GC {}
        .function void std.core.GC.getObjectSpaceTypeImpl() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.GC");
}

TEST_F(RuntimeAdapterStaticTest, StdGCGetObjectAddressTest)
{
    std::string source = R"(
        .record std.core.GC {}
        .record std.core.Object {}
        .function i64 std.core.GC.getObjectAddress(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_GC_GET_OBJECT_ADDRESS, "std.core.GC");

    source = R"(
        .record std.core.GC {}
        .function void std.core.GC.getObjectAddress() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.GC");
}

TEST_F(RuntimeAdapterStaticTest, StdGetObjectSizeTest)
{
    std::string source = R"(
        .record std.core.GC {}
        .record std.core.Object {}
        .function i64 std.core.GC.getObjectSize(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_GET_OBJECT_SIZE, "std.core.GC");

    source = R"(
        .record std.core.GC {}
        .function void std.core.GC.getObjectSize() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.GC");
}

TEST_F(RuntimeAdapterStaticTest, StdGCPinObjectTest)
{
    std::string source = R"(
        .record std.core.GC {}
        .record std.core.Object {}
        .function void std.core.GC.pinObject(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_GC_PIN_OBJECT, "std.core.GC");

    source = R"(
        .record std.core.GC {}
        .function i32 std.core.GC.pinObject() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.GC");
}

TEST_F(RuntimeAdapterStaticTest, StdGCUnpinObjectTest)
{
    std::string source = R"(
        .record std.core.GC {}
        .record std.core.Object {}
        .function void std.core.GC.unpinObject(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_GC_UNPIN_OBJECT, "std.core.GC");

    source = R"(
        .record std.core.GC {}
        .function i32 std.core.GC.unpinObject() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.GC");
}

TEST_F(RuntimeAdapterStaticTest, StdGetFreeHeapSizeTest)
{
    std::string source = R"(
        .record std.core.GC {}
        .function i64 std.core.GC.getFreeHeapSize() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_GET_FREE_HEAP_SIZE, "std.core.GC");

    source = R"(
        .record std.core.GC {}
        .function void std.core.GC.getFreeHeapSize() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.GC");
}

TEST_F(RuntimeAdapterStaticTest, StdGetUsedHeapSizeTest)
{
    std::string source = R"(
        .record std.core.GC {}
        .function i64 std.core.GC.getUsedHeapSize() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_GET_USED_HEAP_SIZE, "std.core.GC");

    source = R"(
        .record std.core.GC {}
        .function void std.core.GC.getUsedHeapSize() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.GC");
}

TEST_F(RuntimeAdapterStaticTest, StdGetReservedHeapSizeTest)
{
    std::string source = R"(
        .record std.core.GC {}
        .function i64 std.core.GC.getReservedHeapSize() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_GET_RESERVED_HEAP_SIZE, "std.core.GC");

    source = R"(
        .record std.core.GC {}
        .function void std.core.GC.getReservedHeapSize() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.GC");
}

TEST_F(RuntimeAdapterStaticTest, StdGCRegisterNativeAllocationTest)
{
    std::string source = R"(
        .record std.core.GC {}
        .function void std.core.GC.registerNativeAllocation(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_GC_REGISTER_NATIVE_ALLOCATION, "std.core.GC");

    source = R"(
        .record std.core.GC {}
        .function i32 std.core.GC.registerNativeAllocation() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.GC");
}

TEST_F(RuntimeAdapterStaticTest, StdGCRegisterNativeFreeTest)
{
    std::string source = R"(
        .record std.core.GC {}
        .function void std.core.GC.registerNativeFree(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_GC_REGISTER_NATIVE_FREE, "std.core.GC");

    source = R"(
        .record std.core.GC {}
        .function i32 std.core.GC.registerNativeFree() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.GC");
}

TEST_F(RuntimeAdapterStaticTest, StdFinalizationRegistryRegisterInstanceTest)
{
    std::string source = R"(
        .record std.core.FinalizationRegistry {}
        .record std.core.Object {}
        .function i32 std.core.FinalizationRegistry.registerInstance(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_FINALIZATION_REGISTRY_REGISTER_INSTANCE,
                 "std.core.FinalizationRegistry");

    source = R"(
        .record std.core.FinalizationRegistry {}
        .function void std.core.FinalizationRegistry.registerInstance() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.FinalizationRegistry");
}

TEST_F(RuntimeAdapterStaticTest, StdFinalizationRegistryFinishCleanupTest)
{
    std::string source = R"(
        .record std.core.FinalizationRegistry {}
        .function void std.core.FinalizationRegistry.finishCleanup() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_FINALIZATION_REGISTRY_FINISH_CLEANUP,
                 "std.core.FinalizationRegistry");

    source = R"(
        .record std.core.FinalizationRegistry {}
        .function i32 std.core.FinalizationRegistry.finishCleanup() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.FinalizationRegistry");
}

TEST_F(RuntimeAdapterStaticTest, StdCoroutineScheduleTest)
{
    std::string source = R"(
        .record std.core.Coroutine {}
        .function void std.core.Coroutine.Schedule() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_COROUTINE_SCHEDULE, "std.core.Coroutine");

    source = R"(
        .record std.core.Coroutine {}
        .function i32 std.core.Coroutine.Schedule() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Coroutine");
}

TEST_F(RuntimeAdapterStaticTest, StdMutexLockTest)
{
    std::string source = R"(
        .record std.core.Mutex {}
        .record std.core.Object {}
        .function void std.core.Mutex.lock(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MUTEX_LOCK, "std.core.Mutex");

    source = R"(
        .record std.core.Mutex {}
        .function i32 std.core.Mutex.lock() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Mutex");
}

TEST_F(RuntimeAdapterStaticTest, StdMutexUnlockTest)
{
    std::string source = R"(
        .record std.core.Mutex {}
        .record std.core.Object {}
        .function void std.core.Mutex.unlock(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_MUTEX_UNLOCK, "std.core.Mutex");

    source = R"(
        .record std.core.Mutex {}
        .function i32 std.core.Mutex.unlock() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Mutex");
}

TEST_F(RuntimeAdapterStaticTest, StdEventWaitTest)
{
    std::string source = R"(
        .record std.core.Event {}
        .record std.core.Object {}
        .function void std.core.Event.wait(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_EVENT_WAIT, "std.core.Event");

    source = R"(
        .record std.core.Event {}
        .function i32 std.core.Event.wait() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Event");
}

TEST_F(RuntimeAdapterStaticTest, StdEventFireTest)
{
    std::string source = R"(
        .record std.core.Event {}
        .record std.core.Object {}
        .function void std.core.Event.fire(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_EVENT_FIRE, "std.core.Event");

    source = R"(
        .record std.core.Event {}
        .function i32 std.core.Event.fire() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Event");
}

TEST_F(RuntimeAdapterStaticTest, StdCondVarWaitTest)
{
    std::string source = R"(
        .record std.core.CondVar {}
        .record std.core.Object {}
        .function void std.core.CondVar.wait(std.core.Object a0, std.core.Object a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_COND_VAR_WAIT, "std.core.CondVar");

    source = R"(
        .record std.core.CondVar {}
        .function i32 std.core.CondVar.wait() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.CondVar");
}

TEST_F(RuntimeAdapterStaticTest, StdCondVarNotfiyOneTest)
{
    std::string source = R"(
        .record std.core.CondVar {}
        .record std.core.Object {}
        .function void std.core.CondVar.notifyOne(std.core.Object a0, std.core.Object a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_COND_VAR_NOTFIY_ONE, "std.core.CondVar");

    source = R"(
        .record std.core.CondVar {}
        .function i32 std.core.CondVar.notifyOne() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.CondVar");
}

TEST_F(RuntimeAdapterStaticTest, StdCondVarNotifyAllTest)
{
    std::string source = R"(
        .record std.core.CondVar {}
        .record std.core.Object {}
        .function void std.core.CondVar.notifyAll(std.core.Object a0, std.core.Object a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_COND_VAR_NOTIFY_ALL, "std.core.CondVar");

    source = R"(
        .record std.core.CondVar {}
        .function i32 std.core.CondVar.notifyAll() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.CondVar");
}

TEST_F(RuntimeAdapterStaticTest, StdQueueSpinlockGuardTest)
{
    std::string source = R"(
        .record std.core.QueueSpinlock {}
        .record std.core.Object {}
        .function void std.core.QueueSpinlock.guard(std.core.Object a0, std.core.Object a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_QUEUE_SPINLOCK_GUARD, "std.core.QueueSpinlock");

    source = R"(
        .record std.core.QueueSpinlock {}
        .function i32 std.core.QueueSpinlock.guard() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.QueueSpinlock");
}

TEST_F(RuntimeAdapterStaticTest, StdReadLockTest)
{
    std::string source = R"(
        .record std.core.ReadLock {}
        .record std.core.Object {}
        .function void std.core.ReadLock.lock(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_READ_LOCK, "std.core.ReadLock");

    source = R"(
        .record std.core.ReadLock {}
        .function i32 std.core.ReadLock.lock() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ReadLock");
}

TEST_F(RuntimeAdapterStaticTest, StdReadUnlockTest)
{
    std::string source = R"(
        .record std.core.ReadLock {}
        .record std.core.Object {}
        .function void std.core.ReadLock.unlock(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_READ_UNLOCK, "std.core.ReadLock");

    source = R"(
        .record std.core.ReadLock {}
        .function i32 std.core.ReadLock.unlock() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ReadLock");
}

TEST_F(RuntimeAdapterStaticTest, StdWriteLockTest)
{
    std::string source = R"(
        .record std.core.WriteLock {}
        .record std.core.Object {}
        .function void std.core.WriteLock.lock(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_WRITE_LOCK, "std.core.WriteLock");

    source = R"(
        .record std.core.WriteLock {}
        .function i32 std.core.WriteLock.lock() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.WriteLock");
}

TEST_F(RuntimeAdapterStaticTest, StdWriteUnlockTest)
{
    std::string source = R"(
        .record std.core.WriteLock {}
        .record std.core.Object {}
        .function void std.core.WriteLock.unlock(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_WRITE_UNLOCK, "std.core.WriteLock");

    source = R"(
        .record std.core.WriteLock {}
        .function i32 std.core.WriteLock.unlock() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.WriteLock");
}

TEST_F(RuntimeAdapterStaticTest, StdCoroutineGetWorkerIdTest)
{
    std::string source = R"(
        .record std.debug.concurrency.CoroutineExtras {}
        .function i32 std.debug.concurrency.CoroutineExtras.getWorkerId() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_COROUTINE_GET_WORKER_ID,
                 "std.debug.concurrency.CoroutineExtras");

    source = R"(
        .record std.debug.concurrency.CoroutineExtras {}
        .function void std.debug.concurrency.CoroutineExtras.getWorkerId() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.debug.concurrency.CoroutineExtras");
}

TEST_F(RuntimeAdapterStaticTest, StdTaskPoolIsSupportingInteropTest)
{
    std::string source = R"(
        .record std.debug.concurrency.CoroutineExtras {}
        .function u1 std.debug.concurrency.CoroutineExtras.isTaskpoolSupportingInterop() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_TASK_POOL_IS_SUPPORTING_INTEROP,
                 "std.debug.concurrency.CoroutineExtras");

    source = R"(
        .record std.debug.concurrency.CoroutineExtras {}
        .function void std.debug.concurrency.CoroutineExtras.isTaskpoolSupportingInterop() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.debug.concurrency.CoroutineExtras");
}

TEST_F(RuntimeAdapterStaticTest, StdTaskPoolIsUsingLaunchModeTest)
{
    std::string source = R"(
        .record std.debug.concurrency.CoroutineExtras {}
        .function u1 std.debug.concurrency.CoroutineExtras.isTaskpoolUsingLaunchMode() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_TASK_POOL_IS_USING_LAUNCH_MODE,
                 "std.debug.concurrency.CoroutineExtras");

    source = R"(
        .record std.debug.concurrency.CoroutineExtras {}
        .function void std.debug.concurrency.CoroutineExtras.isTaskpoolUsingLaunchMode() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.debug.concurrency.CoroutineExtras");
}

TEST_F(RuntimeAdapterStaticTest, StdCoroutineSetSchedulingPolicyTest)
{
    std::string source = R"(
        .record std.debug.concurrency.CoroutineExtras {}
        .function void std.debug.concurrency.CoroutineExtras.setSchedulingPolicy(i32 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_COROUTINE_SET_SCHEDULING_POLICY,
                 "std.debug.concurrency.CoroutineExtras");

    source = R"(
        .record std.debug.concurrency.CoroutineExtras {}
        .function i32 std.debug.concurrency.CoroutineExtras.setSchedulingPolicy() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.debug.concurrency.CoroutineExtras");
}

TEST_F(RuntimeAdapterStaticTest, StdCoroutineGetCoroutineIdTest)
{
    std::string source = R"(
        .record std.debug.concurrency.CoroutineExtras {}
        .function i32 std.debug.concurrency.CoroutineExtras.getCoroutineId() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_COROUTINE_GET_COROUTINE_ID,
                 "std.debug.concurrency.CoroutineExtras");

    source = R"(
        .record std.debug.concurrency.CoroutineExtras {}
        .function void std.debug.concurrency.CoroutineExtras.getCoroutineId() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.debug.concurrency.CoroutineExtras");
}

TEST_F(RuntimeAdapterStaticTest, StdCoroutineIsMainWorkerTest)
{
    std::string source = R"(
        .record std.debug.concurrency.CoroutineExtras {}
        .function u1 std.debug.concurrency.CoroutineExtras.isMainWorker() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_COROUTINE_IS_MAIN_WORKER,
                 "std.debug.concurrency.CoroutineExtras");

    source = R"(
        .record std.debug.concurrency.CoroutineExtras {}
        .function void std.debug.concurrency.CoroutineExtras.isMainWorker() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.debug.concurrency.CoroutineExtras");
}

TEST_F(RuntimeAdapterStaticTest, StdCoroutineWorkerHasExternalSchedulerTest)
{
    std::string source = R"(
        .record std.debug.concurrency.CoroutineExtras {}
        .function u1 std.debug.concurrency.CoroutineExtras.workerHasExternalScheduler() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_COROUTINE_WORKER_HAS_EXTERNAL_SCHEDULER,
                 "std.debug.concurrency.CoroutineExtras");

    source = R"(
        .record std.debug.concurrency.CoroutineExtras {}
        .function void std.debug.concurrency.CoroutineExtras.workerHasExternalScheduler() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.debug.concurrency.CoroutineExtras");
}

TEST_F(RuntimeAdapterStaticTest, StdCoroutineScaleWorkersPoolTest)
{
    std::string source = R"(
        .record std.debug.concurrency.CoroutineExtras {}
        .function void std.debug.concurrency.CoroutineExtras.scaleWorkersPool(i32 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_COROUTINE_SCALE_WORKERS_POOL,
                 "std.debug.concurrency.CoroutineExtras");

    source = R"(
        .record std.debug.concurrency.CoroutineExtras {}
        .function i32 std.debug.concurrency.CoroutineExtras.scaleWorkersPool() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.debug.concurrency.CoroutineExtras");
}

TEST_F(RuntimeAdapterStaticTest, StdCoroutineStopTaskpoolTest)
{
    std::string source = R"(
        .record std.debug.concurrency.CoroutineExtras {}
        .function void std.debug.concurrency.CoroutineExtras.stopTaskpool() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_COROUTINE_STOP_TASKPOOL,
                 "std.debug.concurrency.CoroutineExtras");

    source = R"(
        .record std.debug.concurrency.CoroutineExtras {}
        .function i32 std.debug.concurrency.CoroutineExtras.stopTaskpool() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.debug.concurrency.CoroutineExtras");
}

TEST_F(RuntimeAdapterStaticTest, StdCoroutineIncreaseTaskpoolWorkersToNTest)
{
    std::string source = R"(
        .record std.debug.concurrency.CoroutineExtras {}
        .function void std.debug.concurrency.CoroutineExtras.increaseTaskpoolWorkersToN(i32 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_COROUTINE_INCREASE_TASKPOOL_WORKERS_TO_N,
                 "std.debug.concurrency.CoroutineExtras");

    source = R"(
        .record std.debug.concurrency.CoroutineExtras {}
        .function i32 std.debug.concurrency.CoroutineExtras.increaseTaskpoolWorkersToN() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.debug.concurrency.CoroutineExtras");
}

TEST_F(RuntimeAdapterStaticTest, StdCoroutineSetTaskPoolTriggerShrinkIntervalTest)
{
    std::string source = R"(
        .record std.debug.concurrency.CoroutineExtras {}
        .function void std.debug.concurrency.CoroutineExtras.setTaskPoolTriggerShrinkInterval(i32 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_COROUTINE_SET_TASK_POOL_TRIGGER_SHRINK_INTERVAL,
                 "std.debug.concurrency.CoroutineExtras");

    source = R"(
        .record std.debug.concurrency.CoroutineExtras {}
        .function i32 std.debug.concurrency.CoroutineExtras.setTaskPoolTriggerShrinkInterval() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.debug.concurrency.CoroutineExtras");
}

TEST_F(RuntimeAdapterStaticTest, StdCoroutineSetTaskPoolIdleThresholdTest)
{
    std::string source = R"(
        .record std.debug.concurrency.CoroutineExtras {}
        .function void std.debug.concurrency.CoroutineExtras.setTaskPoolIdleThreshold(i32 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_COROUTINE_SET_TASK_POOL_IDLE_THRESHOLD,
                 "std.debug.concurrency.CoroutineExtras");

    source = R"(
        .record std.debug.concurrency.CoroutineExtras {}
        .function i32 std.debug.concurrency.CoroutineExtras.setTaskPoolIdleThreshold() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.debug.concurrency.CoroutineExtras");
}

TEST_F(RuntimeAdapterStaticTest, StdCoroutineGetTaskPoolWorkersNumTest)
{
    std::string source = R"(
        .record std.debug.concurrency.CoroutineExtras {}
        .function i32 std.debug.concurrency.CoroutineExtras.getTaskPoolWorkersNum() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_COROUTINE_GET_TASK_POOL_WORKERS_NUM,
                 "std.debug.concurrency.CoroutineExtras");

    source = R"(
        .record std.debug.concurrency.CoroutineExtras {}
        .function void std.debug.concurrency.CoroutineExtras.getTaskPoolWorkersNum() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.debug.concurrency.CoroutineExtras");
}

TEST_F(RuntimeAdapterStaticTest, StdCoroutineSetTaskPoolWorkersLimitTest)
{
    std::string source = R"(
        .record std.debug.concurrency.CoroutineExtras {}
        .function void std.debug.concurrency.CoroutineExtras.setTaskPoolWorkersLimit(i32 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_COROUTINE_SET_TASK_POOL_WORKERS_LIMIT,
                 "std.debug.concurrency.CoroutineExtras");

    source = R"(
        .record std.debug.concurrency.CoroutineExtras {}
        .function i32 std.debug.concurrency.CoroutineExtras.setTaskPoolWorkersLimit() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.debug.concurrency.CoroutineExtras");
}

TEST_F(RuntimeAdapterStaticTest, StdCoroutineRetriggerTaskPoolShrinkTest)
{
    std::string source = R"(
        .record std.debug.concurrency.CoroutineExtras {}
        .function void std.debug.concurrency.CoroutineExtras.retriggerTaskPoolShrink() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_COROUTINE_RETRIGGER_TASK_POOL_SHRINK,
                 "std.debug.concurrency.CoroutineExtras");

    source = R"(
        .record std.debug.concurrency.CoroutineExtras {}
        .function i32 std.debug.concurrency.CoroutineExtras.retriggerTaskPoolShrink() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.debug.concurrency.CoroutineExtras");
}

TEST_F(RuntimeAdapterStaticTest, StdCoroutineGetTaskPoolWorkersLimitTest)
{
    std::string source = R"(
        .record std.debug.concurrency.CoroutineExtras {}
        .function i32 std.debug.concurrency.CoroutineExtras.getTaskPoolWorkersLimit() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_COROUTINE_GET_TASK_POOL_WORKERS_LIMIT,
                 "std.debug.concurrency.CoroutineExtras");

    source = R"(
        .record std.debug.concurrency.CoroutineExtras {}
        .function void std.debug.concurrency.CoroutineExtras.getTaskPoolWorkersLimit() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.debug.concurrency.CoroutineExtras");
}

TEST_F(RuntimeAdapterStaticTest, StdCoroutineIsExternalTimerEnabledTest)
{
    std::string source = R"(
        .record std.debug.concurrency.CoroutineExtras {}
        .function u1 std.debug.concurrency.CoroutineExtras.isExternalTimerEnabled() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_COROUTINE_IS_EXTERNAL_TIMER_ENABLED,
                 "std.debug.concurrency.CoroutineExtras");

    source = R"(
        .record std.debug.concurrency.CoroutineExtras {}
        .function void std.debug.concurrency.CoroutineExtras.isExternalTimerEnabled() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.debug.concurrency.CoroutineExtras");
}

TEST_F(RuntimeAdapterStaticTest, StdIsLittleEndianPlatformTest)
{
    std::string source = R"(
        .record std.core.Runtime {}
        .function u1 std.core.Runtime.isLittleEndianPlatform() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_IS_LITTLE_ENDIAN_PLATFORM, "std.core.Runtime");

    source = R"(
        .record std.core.Runtime {}
        .function void std.core.Runtime.isLittleEndianPlatform() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Runtime");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreRuntimeIsSameReferenceTest)
{
    std::string source = R"(
        .record std.core.Runtime {}
        .record std.core.Object {}
        .function u1 std.core.Runtime.isSameReference(std.core.Object a0, std.core.Object a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_RUNTIME_IS_SAME_REFERENCE,
                 "std.core.Runtime");

    source = R"(
        .record std.core.Runtime {}
        .function void std.core.Runtime.isSameReference() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Runtime");
}

TEST_F(RuntimeAdapterStaticTest, StdRuntimeGetHashCodeTest)
{
    std::string source = R"(
        .record std.core.Runtime {}
        .record std.core.Object {}
        .function i32 std.core.Runtime.getHashCode(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_RUNTIME_GET_HASH_CODE, "std.core.Runtime");

    source = R"(
        .record std.core.Runtime {}
        .function void std.core.Runtime.getHashCode() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Runtime");
}

TEST_F(RuntimeAdapterStaticTest, StdRuntimeGetHashCodeByValueTest)
{
    std::string source = R"(
        .record std.core.Runtime {}
        .record std.core.Object {}
        .function i64 std.core.Runtime.getHashCodeByValue(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_RUNTIME_GET_HASH_CODE_BY_VALUE,
                 "std.core.Runtime");

    source = R"(
        .record std.core.Runtime {}
        .function void std.core.Runtime.getHashCodeByValue() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Runtime");
}

TEST_F(RuntimeAdapterStaticTest, StdRuntimeSameValueZeroTest)
{
    std::string source = R"(
        .record std.core.Runtime {}
        .record std.core.Object {}
        .function u1 std.core.Runtime.sameValueZero(std.core.Object a0, std.core.Object a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_RUNTIME_SAME_VALUE_ZERO, "std.core.Runtime");

    source = R"(
        .record std.core.Runtime {}
        .function void std.core.Runtime.sameValueZero() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Runtime");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreRuntimeFailedTypeCastExceptionTest)
{
    std::string source = R"(
        .record std.core.Runtime {}
        .record std.core.String {}
        .record std.core.Object {}
        .record std.core.ClassCastError {}
        .function std.core.ClassCastError std.core.Runtime.failedTypeCastException(std.core.Object a0, std.core.String a1, u1 a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_RUNTIME_FAILED_TYPE_CAST_EXCEPTION,
                 "std.core.Runtime");

    source = R"(
        .record std.core.Runtime {}
        .function void std.core.Runtime.failedTypeCastException() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Runtime");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreRuntimeGetTypeInfoTest)
{
    std::string source = R"(
        .record std.core.Runtime {}
        .record std.core.Object {}
        .record std.core.Class {}
        .function std.core.Class std.core.Runtime.__getTypeInfo(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_RUNTIME_GET_TYPE_INFO, "std.core.Runtime");

    source = R"(
        .record std.core.Runtime {}
        .function void std.core.Runtime.__getTypeInfo() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Runtime");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreRuntimeAllocSameTypeArrayTest)
{
    std::string source = R"(
        .record std.core.Runtime {}
        .record std.core.Class {}
        .function std.core.Object[] std.core.Runtime.allocSameTypeArray(std.core.Class a0, i32 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_RUNTIME_ALLOC_SAME_TYPE_ARRAY,
                 "std.core.Runtime");

    source = R"(
        .record std.core.Runtime {}
        .function void std.core.Runtime.allocSameTypeArray() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Runtime");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreReflectMethodGetReturnTypeImplTest)
{
    std::string source = R"(
        .record std.core.reflect.Method {}
        .record std.core.Class {}
        .function std.core.Class std.core.reflect.Method.getReturnTypeImpl(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_REFLECT_METHOD_GET_RETURN_TYPE_IMPL,
                 "std.core.reflect.Method");

    source = R"(
        .record std.core.reflect.Method {}
        .function void std.core.reflect.Method.getReturnTypeImpl() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.reflect.Method");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreReflectMethodGetParameterTypeByIdxImplTest)
{
    std::string source = R"(
        .record std.core.reflect.Method {}
        .record std.core.Class {}
        .function std.core.Class std.core.reflect.Method.getParameterTypeByIdxImpl(i64 a0, i32 a1) <native>
    )";
    MainTestFunc(source,
                 RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_REFLECT_METHOD_GET_PARAMETER_TYPE_BY_IDX_IMPL,
                 "std.core.reflect.Method");

    source = R"(
        .record std.core.reflect.Method {}
        .function void std.core.reflect.Method.getParameterTypeByIdxImpl() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.reflect.Method");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreReflectMethodGetParametersTypesImplTest)
{
    std::string source = R"(
        .record std.core.reflect.Method {}
        .function std.core.Class[] std.core.reflect.Method.getParametersTypesImpl(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_REFLECT_METHOD_GET_PARAMETERS_TYPES_IMPL,
                 "std.core.reflect.Method");

    source = R"(
        .record std.core.reflect.Method {}
        .function void std.core.reflect.Method.getParametersTypesImpl() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.reflect.Method");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreReflectMethodGetParametersNumImplTest)
{
    std::string source = R"(
        .record std.core.reflect.Method {}
        .function i32 std.core.reflect.Method.getParametersNumImpl(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_REFLECT_METHOD_GET_PARAMETERS_NUM_IMPL,
                 "std.core.reflect.Method");

    source = R"(
        .record std.core.reflect.Method {}
        .function void std.core.reflect.Method.getParametersNumImpl() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.reflect.Method");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreReflectMethodGetNameInternalTest)
{
    std::string source = R"(
        .record std.core.reflect.Method {}
        .record std.core.String {}
        .function std.core.String std.core.reflect.Method.getNameInternal(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_REFLECT_METHOD_GET_NAME_INTERNAL,
                 "std.core.reflect.Method");

    source = R"(
        .record std.core.reflect.Method {}
        .function void std.core.reflect.Method.getNameInternal() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.reflect.Method");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreReflectFieldGetNameInternalTest)
{
    std::string source = R"(
        .record std.core.reflect.Field {}
        .record std.core.String {}
        .function std.core.String std.core.reflect.Field.getNameInternal(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_REFLECT_FIELD_GET_NAME_INTERNAL,
                 "std.core.reflect.Field");

    source = R"(
        .record std.core.reflect.Field {}
        .function void std.core.reflect.Field.getNameInternal() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.reflect.Field");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreClassOfTest)
{
    std::string source = R"(
        .record std.core.Class {}
        .record std.core.Object {}
        .function std.core.Class std.core.Class.ofObject(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_CLASS_OF, "std.core.Class");

    source = R"(
        .record std.core.Class {}
        .function void std.core.Class.ofObject() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Class");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreClassOfNullTest)
{
    std::string source = R"(
        .record std.core.Class {}
        .function std.core.Class std.core.Class.ofNull() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_CLASS_OF_NULL, "std.core.Class");

    source = R"(
        .record std.core.Class {}
        .function void std.core.Class.ofNull() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Class");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreClassCurrentTest)
{
    std::string source = R"(
        .record std.core.Class {}
        .function std.core.Class std.core.Class.current() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_CLASS_CURRENT, "std.core.Class");

    source = R"(
        .record std.core.Class {}
        .function void std.core.Class.current() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Class");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreClassOfCallerTest)
{
    std::string source = R"(
        .record std.core.Class {}
        .function std.core.Class std.core.Class.ofCaller() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_CLASS_OF_CALLER, "std.core.Class");

    source = R"(
        .record std.core.Class {}
        .function void std.core.Class.ofCaller() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Class");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreAbcFileLoadAbcFileTest)
{
    std::string source = R"(
        .record std.core.AbcFile {}
        .record std.core.String {}
        .record std.core.RuntimeLinker {}
        .function std.core.AbcFile std.core.AbcFile.loadAbcFile(std.core.RuntimeLinker a0, std.core.String a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_ABC_FILE_LOAD_ABC_FILE, "std.core.AbcFile");

    source = R"(
        .record std.core.AbcFile {}
        .function void std.core.AbcFile.loadAbcFile() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.AbcFile");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreAbcFileLoadFromMemoryTest)
{
    std::string source = R"(
        .record std.core.AbcFile {}
        .record std.core.RuntimeLinker {}
        .function std.core.AbcFile std.core.AbcFile.loadFromMemory(std.core.RuntimeLinker a0, i8[] a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_ABC_FILE_LOAD_FROM_MEMORY,
                 "std.core.AbcFile");

    source = R"(
        .record std.core.AbcFile {}
        .function void std.core.AbcFile.loadFromMemory() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.AbcFile");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreRegExpParseTest)
{
    std::string source = R"(
        .record std.core.RegExp {}
        .record std.core.String {}
        .function std.core.String std.core.RegExp.parse(std.core.String a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_REG_EXP_PARSE, "std.core.RegExp");

    source = R"(
        .record std.core.RegExp {}
        .function void std.core.RegExp.parse() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.RegExp");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPIGetTypeKindTest)
{
    std::string source = R"(
        .record std.core.TypeAPI {}
        .record std.core.String {}
        .record std.core.RuntimeLinker {}
        .function i8 std.core.TypeAPI.getTypeKind(std.core.String a0, std.core.RuntimeLinker a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_GET_TYPE_KIND, "std.core.TypeAPI");

    source = R"(
        .record std.core.TypeAPI {}
        .function void std.core.TypeAPI.getTypeKind() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeAPI");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPIIsValueTypeTest)
{
    std::string source = R"(
        .record std.core.TypeAPI {}
        .record std.core.String {}
        .record std.core.RuntimeLinker {}
        .function u1 std.core.TypeAPI.isValueType(std.core.String a0, std.core.RuntimeLinker a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_IS_VALUE_TYPE, "std.core.TypeAPI");

    source = R"(
        .record std.core.TypeAPI {}
        .function void std.core.TypeAPI.isValueType() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeAPI");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPIGetNullTypeDescriptorTest)
{
    std::string source = R"(
        .record std.core.TypeAPI {}
        .record std.core.String {}
        .function std.core.String std.core.TypeAPI.getNullTypeDescriptor() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_GET_NULL_TYPE_DESCRIPTOR,
                 "std.core.TypeAPI");

    source = R"(
        .record std.core.TypeAPI {}
        .function void std.core.TypeAPI.getNullTypeDescriptor() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeAPI");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPIGetUndefinedTypeDescriptorTest)
{
    std::string source = R"(
        .record std.core.TypeAPI {}
        .record std.core.String {}
        .function std.core.String std.core.TypeAPI.getUndefinedTypeDescriptor() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_GET_UNDEFINED_TYPE_DESCRIPTOR,
                 "std.core.TypeAPI");

    source = R"(
        .record std.core.TypeAPI {}
        .function void std.core.TypeAPI.getUndefinedTypeDescriptor() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeAPI");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPIGetClassTest)
{
    std::string source = R"(
        .record std.core.TypeAPI {}
        .record std.core.String {}
        .record std.core.RuntimeLinker {}
        .record std.core.Class {}
        .function std.core.Class std.core.TypeAPI.getClass(std.core.String a0, std.core.RuntimeLinker a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_GET_CLASS, "std.core.TypeAPI");

    source = R"(
        .record std.core.TypeAPI {}
        .function void std.core.TypeAPI.getClass() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeAPI");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPIGetClassAttributesTest)
{
    std::string source = R"(
        .record std.core.TypeAPI {}
        .record std.core.Class {}
        .function i32 std.core.TypeAPI.getClassAttributes(std.core.Class a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_GET_CLASS_ATTRIBUTES, "std.core.TypeAPI");

    source = R"(
        .record std.core.TypeAPI {}
        .function void std.core.TypeAPI.getClassAttributes() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeAPI");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPIGetFieldsNumTest)
{
    std::string source = R"(
        .record std.core.TypeAPI {}
        .record std.core.Class {}
        .function i64 std.core.TypeAPI.getFieldsNum(std.core.Class a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_GET_FIELDS_NUM, "std.core.TypeAPI");

    source = R"(
        .record std.core.TypeAPI {}
        .function void std.core.TypeAPI.getFieldsNum() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeAPI");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPIGetOwnFieldsNumTest)
{
    std::string source = R"(
        .record std.core.TypeAPI {}
        .record std.core.Class {}
        .function i64 std.core.TypeAPI.getOwnFieldsNum(std.core.Class a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_GET_OWN_FIELDS_NUM, "std.core.TypeAPI");

    source = R"(
        .record std.core.TypeAPI {}
        .function void std.core.TypeAPI.getOwnFieldsNum() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeAPI");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPIGetFieldPtrTest)
{
    std::string source = R"(
        .record std.core.TypeAPI {}
        .record std.core.Long {}
        .record std.core.Class {}
        .function std.core.Long std.core.TypeAPI.getFieldPtr(std.core.Class a0, i64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_GET_FIELD_PTR, "std.core.TypeAPI");

    source = R"(
        .record std.core.TypeAPI {}
        .function void std.core.TypeAPI.getFieldPtr() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeAPI");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPIGetOwnFieldPtrTest)
{
    std::string source = R"(
        .record std.core.TypeAPI {}
        .record std.core.Long {}
        .record std.core.Class {}
        .function std.core.Long std.core.TypeAPI.getOwnFieldPtr(std.core.Class a0, i64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_GET_OWN_FIELD_PTR, "std.core.TypeAPI");

    source = R"(
        .record std.core.TypeAPI {}
        .function void std.core.TypeAPI.getOwnFieldPtr() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeAPI");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPIGetFieldPtrByNameTest)
{
    std::string source = R"(
        .record std.core.TypeAPI {}
        .record std.core.String {}
        .record std.core.Long {}
        .record std.core.Class {}
        .function std.core.Long std.core.TypeAPI.getFieldPtrByName(std.core.Class a0, std.core.String a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_GET_FIELD_PTR_BY_NAME, "std.core.TypeAPI");

    source = R"(
        .record std.core.TypeAPI {}
        .function void std.core.TypeAPI.getFieldPtrByName() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeAPI");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPIGetFieldDescriptorTest)
{
    std::string source = R"(
        .record std.core.TypeAPI {}
        .record std.core.String {}
        .function std.core.String std.core.TypeAPI.getFieldDescriptor(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_GET_FIELD_DESCRIPTOR, "std.core.TypeAPI");

    source = R"(
        .record std.core.TypeAPI {}
        .function void std.core.TypeAPI.getFieldDescriptor() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeAPI");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPIGetFieldOwnerTest)
{
    std::string source = R"(
        .record std.core.TypeAPI {}
        .record std.core.Class {}
        .function std.core.Class std.core.TypeAPI.getFieldOwner(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_GET_FIELD_OWNER, "std.core.TypeAPI");

    source = R"(
        .record std.core.TypeAPI {}
        .function void std.core.TypeAPI.getFieldOwner() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeAPI");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPIGetFieldTest)
{
    std::string source = R"(
        .record std.core.TypeAPI {}
        .record std.core.Type {}
        .record std.core.Field {}
        .function std.core.Field std.core.TypeAPI.getField(i64 a0, std.core.Type a1, std.core.Type a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_GET_FIELD, "std.core.TypeAPI");

    source = R"(
        .record std.core.TypeAPI {}
        .function void std.core.TypeAPI.getField() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeAPI");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPIGetMethodsNumTest)
{
    std::string source = R"(
        .record std.core.TypeAPI {}
        .record std.core.Class {}
        .function i64 std.core.TypeAPI.getMethodsNum(std.core.Class a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_GET_METHODS_NUM, "std.core.TypeAPI");

    source = R"(
        .record std.core.TypeAPI {}
        .function void std.core.TypeAPI.getMethodsNum() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeAPI");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPIGetMethodDescriptorTest)
{
    std::string source = R"(
        .record std.core.TypeAPI {}
        .record std.core.String {}
        .record std.core.Class {}
        .function std.core.String std.core.TypeAPI.getMethodDescriptor(std.core.Class a0, i64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_GET_METHOD_DESCRIPTOR, "std.core.TypeAPI");

    source = R"(
        .record std.core.TypeAPI {}
        .function void std.core.TypeAPI.getMethodDescriptor() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeAPI");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPIGetMethodTest)
{
    std::string source = R"(
        .record std.core.TypeAPI {}
        .record std.core.Type {}
        .record std.core.Method {}
        .record std.core.MethodType {}
        .function std.core.Method std.core.TypeAPI.getMethod(std.core.MethodType a0, std.core.Type a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_GET_METHOD, "std.core.TypeAPI");

    source = R"(
        .record std.core.TypeAPI {}
        .function void std.core.TypeAPI.getMethod() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeAPI");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPIGetConstructorsNumTest)
{
    std::string source = R"(
        .record std.core.TypeAPI {}
        .record std.core.Class {}
        .function i64 std.core.TypeAPI.getConstructorsNum(std.core.Class a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_GET_CONSTRUCTORS_NUM, "std.core.TypeAPI");

    source = R"(
        .record std.core.TypeAPI {}
        .function void std.core.TypeAPI.getConstructorsNum() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeAPI");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPIGetConstructorDescriptorTest)
{
    std::string source = R"(
        .record std.core.TypeAPI {}
        .record std.core.String {}
        .record std.core.Class {}
        .function std.core.String std.core.TypeAPI.getConstructorDescriptor(std.core.Class a0, i64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_GET_CONSTRUCTOR_DESCRIPTOR,
                 "std.core.TypeAPI");

    source = R"(
        .record std.core.TypeAPI {}
        .function void std.core.TypeAPI.getConstructorDescriptor() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeAPI");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPIGetArrayElementTypeTest)
{
    std::string source = R"(
        .record std.core.TypeAPI {}
        .record std.core.String {}
        .record std.core.Class {}
        .function std.core.String std.core.TypeAPI.getArrayElementType(std.core.Class a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_GET_ARRAY_ELEMENT_TYPE, "std.core.TypeAPI");

    source = R"(
        .record std.core.TypeAPI {}
        .function void std.core.TypeAPI.getArrayElementType() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeAPI");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPIMakeArrayInstanceTest)
{
    std::string source = R"(
        .record std.core.TypeAPI {}
        .record std.core.String {}
        .record std.core.RuntimeLinker {}
        .record std.core.Object {}
        .function std.core.Object std.core.TypeAPI.makeArrayInstance(std.core.String a0, std.core.RuntimeLinker a1, i64 a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_MAKE_ARRAY_INSTANCE, "std.core.TypeAPI");

    source = R"(
        .record std.core.TypeAPI {}
        .function void std.core.TypeAPI.makeArrayInstance() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeAPI");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPIGetInterfacesNumTest)
{
    std::string source = R"(
        .record std.core.TypeAPI {}
        .record std.core.Class {}
        .function i64 std.core.TypeAPI.getInterfacesNum(std.core.Class a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_GET_INTERFACES_NUM, "std.core.TypeAPI");

    source = R"(
        .record std.core.TypeAPI {}
        .function void std.core.TypeAPI.getInterfacesNum() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeAPI");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPIGetInterfaceTest)
{
    std::string source = R"(
        .record std.core.TypeAPI {}
        .record std.core.Class {}
        .function std.core.Class std.core.TypeAPI.getInterface(std.core.Class a0, i64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_GET_INTERFACE, "std.core.TypeAPI");

    source = R"(
        .record std.core.TypeAPI {}
        .function void std.core.TypeAPI.getInterface() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeAPI");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPIIsInheritedFromTest)
{
    std::string source = R"(
        .record std.core.TypeAPI {}
        .record std.core.Class {}
        .function u1 std.core.TypeAPI.isInheritedFrom(std.core.Class a0, std.core.Class a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_IS_INHERITED_FROM, "std.core.TypeAPI");

    source = R"(
        .record std.core.TypeAPI {}
        .function void std.core.TypeAPI.isInheritedFrom() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeAPI");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPIGetFunctionAttributesTest)
{
    std::string source = R"(
        .record std.core.TypeAPI {}
        .record std.core.String {}
        .function i32 std.core.TypeAPI.getFunctionAttributes(std.core.String a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_GET_FUNCTION_ATTRIBUTES, "std.core.TypeAPI");

    source = R"(
        .record std.core.TypeAPI {}
        .function void std.core.TypeAPI.getFunctionAttributes() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeAPI");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPIGetDeclaringClassImplTest)
{
    std::string source = R"(
        .record std.core.TypeAPI {}
        .record std.core.Class {}
        .function std.core.Class std.core.TypeAPI.getDeclaringClassImpl({Ustd.core.LambdaType, std.core.MethodType} a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_GET_DECLARING_CLASS_IMPL,
                 "std.core.TypeAPI");

    source = R"(
        .record std.core.TypeAPI {}
        .function void std.core.TypeAPI.getDeclaringClassImpl() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeAPI");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreCharToByteTest)
{
    std::string source = R"(
        .record std.core.Char {}
        .function i8 std.core.Char.toByte(u16 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_CHAR_TO_BYTE, "std.core.Char");

    source = R"(
        .record std.core.Char {}
        .function void std.core.Char.toByte() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Char");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreCharToShortTest)
{
    std::string source = R"(
        .record std.core.Char {}
        .function i16 std.core.Char.toShort(u16 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_CHAR_TO_SHORT, "std.core.Char");

    source = R"(
        .record std.core.Char {}
        .function void std.core.Char.toShort() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Char");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreCharToIntTest)
{
    std::string source = R"(
        .record std.core.Char {}
        .function i32 std.core.Char.toInt(u16 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_CHAR_TO_INT, "std.core.Char");

    source = R"(
        .record std.core.Char {}
        .function void std.core.Char.toInt() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Char");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreCharToLongTest)
{
    std::string source = R"(
        .record std.core.Char {}
        .function i64 std.core.Char.toLong(u16 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_CHAR_TO_LONG, "std.core.Char");

    source = R"(
        .record std.core.Char {}
        .function void std.core.Char.toLong() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Char");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreCharToFloatTest)
{
    std::string source = R"(
        .record std.core.Char {}
        .function f32 std.core.Char.toFloat(u16 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_CHAR_TO_FLOAT, "std.core.Char");

    source = R"(
        .record std.core.Char {}
        .function void std.core.Char.toFloat() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Char");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreCharToDoubleTest)
{
    std::string source = R"(
        .record std.core.Char {}
        .function f64 std.core.Char.toDouble(u16 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_CHAR_TO_DOUBLE, "std.core.Char");

    source = R"(
        .record std.core.Char {}
        .function void std.core.Char.toDouble() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Char");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreCharToStringTest)
{
    std::string source = R"(
        .record std.core.Char {}
        .record std.core.String {}
        .function std.core.String std.core.Char.toString(u16 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_CHAR_TO_STRING, "std.core.Char");

    source = R"(
        .record std.core.Char {}
        .function void std.core.Char.toString() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Char");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreCharIsUpperCaseTest)
{
    std::string source = R"(
        .record std.core.Char {}
        .function u1 std.core.Char.isUpperCase(u16 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_CHAR_IS_UPPER_CASE, "std.core.Char");

    source = R"(
        .record std.core.Char {}
        .function void std.core.Char.isUpperCase() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Char");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreCharToUpperCaseTest)
{
    std::string source = R"(
        .record std.core.Char {}
        .function u16 std.core.Char.toUpperCase(u16 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_CHAR_TO_UPPER_CASE, "std.core.Char");

    source = R"(
        .record std.core.Char {}
        .function void std.core.Char.toUpperCase() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Char");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreCharIsLowerCaseTest)
{
    std::string source = R"(
        .record std.core.Char {}
        .function u1 std.core.Char.isLowerCase(u16 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_CHAR_IS_LOWER_CASE, "std.core.Char");

    source = R"(
        .record std.core.Char {}
        .function void std.core.Char.isLowerCase() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Char");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreCharToLowerCaseTest)
{
    std::string source = R"(
        .record std.core.Char {}
        .function u16 std.core.Char.toLowerCase(u16 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_CHAR_TO_LOWER_CASE, "std.core.Char");

    source = R"(
        .record std.core.Char {}
        .function void std.core.Char.toLowerCase() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Char");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreCharIsWhiteSpaceTest)
{
    std::string source = R"(
        .record std.core.Char {}
        .function u1 std.core.Char.isWhiteSpace(u16 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_CHAR_IS_WHITE_SPACE, "std.core.Char");

    source = R"(
        .record std.core.Char {}
        .function void std.core.Char.isWhiteSpace() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Char");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPITypeCreatorCtxCreateTest)
{
    std::string source = R"(
        .record std.core.TypeCreatorCtx {}
        .function i64 std.core.TypeCreatorCtx.createCtx() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_TYPE_CREATOR_CTX_CREATE,
                 "std.core.TypeCreatorCtx");

    source = R"(
        .record std.core.TypeCreatorCtx {}
        .function void std.core.TypeCreatorCtx.createCtx() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeCreatorCtx");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPITypeCreatorCtxDestroyTest)
{
    std::string source = R"(
        .record std.core.TypeCreatorCtx {}
        .function void std.core.TypeCreatorCtx.destroyCtx(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_TYPE_CREATOR_CTX_DESTROY,
                 "std.core.TypeCreatorCtx");

    source = R"(
        .record std.core.TypeCreatorCtx {}
        .function i32 std.core.TypeCreatorCtx.destroyCtx() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeCreatorCtx");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPITypeCreatorCtxCommitTest)
{
    std::string source = R"(
        .record std.core.TypeCreatorCtx {}
        .record std.core.String {}
        .record std.core.RuntimeLinker {}
        .function std.core.String std.core.TypeCreatorCtx.commit(i64 a0, std.core.Object[] a1, std.core.RuntimeLinker a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_TYPE_CREATOR_CTX_COMMIT,
                 "std.core.TypeCreatorCtx");

    source = R"(
        .record std.core.TypeCreatorCtx {}
        .function void std.core.TypeCreatorCtx.commit() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeCreatorCtx");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPITypeCreatorCtxGetErrorTest)
{
    std::string source = R"(
        .record std.core.TypeCreatorCtx {}
        .record std.core.String {}
        .function std.core.String std.core.TypeCreatorCtx.getError(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_TYPE_CREATOR_CTX_GET_ERROR,
                 "std.core.TypeCreatorCtx");

    source = R"(
        .record std.core.TypeCreatorCtx {}
        .function void std.core.TypeCreatorCtx.getError() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeCreatorCtx");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPITypeCreatorCtxGetObjectsArrayForCCtorTest)
{
    std::string source = R"(
        .record std.core.TypeCreatorCtx {}
        .function std.core.Object[] std.core.TypeCreatorCtx.getObjectsArrayForCCtor(i64 a0) <native>
    )";
    MainTestFunc(source,
                 RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_TYPE_CREATOR_CTX_GET_OBJECTS_ARRAY_FOR_C_CTOR,
                 "std.core.TypeCreatorCtx");

    source = R"(
        .record std.core.TypeCreatorCtx {}
        .function void std.core.TypeCreatorCtx.getObjectsArrayForCCtor() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeCreatorCtx");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPITypeCreatorCtxGetTypeDescFromPointerTest)
{
    std::string source = R"(
        .record std.core.TypeCreatorCtx {}
        .record std.core.String {}
        .function std.core.String std.core.TypeCreatorCtx.getTypeDescFromPointer(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_TYPE_CREATOR_CTX_GET_TYPE_DESC_FROM_POINTER,
                 "std.core.TypeCreatorCtx");

    source = R"(
        .record std.core.TypeCreatorCtx {}
        .function void std.core.TypeCreatorCtx.getTypeDescFromPointer() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeCreatorCtx");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPITypeCreatorCtxClassCreateTest)
{
    std::string source = R"(
        .record std.core.TypeCreatorCtx {}
        .record std.core.String {}
        .function i64 std.core.TypeCreatorCtx.classCreate(i64 a0, std.core.String a1, i32 a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_TYPE_CREATOR_CTX_CLASS_CREATE,
                 "std.core.TypeCreatorCtx");

    source = R"(
        .record std.core.TypeCreatorCtx {}
        .function void std.core.TypeCreatorCtx.classCreate() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeCreatorCtx");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPITypeCreatorCtxClassSetBaseTest)
{
    std::string source = R"(
        .record std.core.TypeCreatorCtx {}
        .record std.core.String {}
        .function std.core.String std.core.TypeCreatorCtx.classSetBase(i64 a0, std.core.String a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_TYPE_CREATOR_CTX_CLASS_SET_BASE,
                 "std.core.TypeCreatorCtx");

    source = R"(
        .record std.core.TypeCreatorCtx {}
        .function void std.core.TypeCreatorCtx.classSetBase() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeCreatorCtx");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPITypeCreatorCtxClassAddIfaceTest)
{
    std::string source = R"(
        .record std.core.TypeCreatorCtx {}
        .record std.core.String {}
        .function std.core.String std.core.TypeCreatorCtx.classAddIface(i64 a0, std.core.String a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_TYPE_CREATOR_CTX_CLASS_ADD_IFACE,
                 "std.core.TypeCreatorCtx");

    source = R"(
        .record std.core.TypeCreatorCtx {}
        .function void std.core.TypeCreatorCtx.classAddIface() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeCreatorCtx");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPITypeCreatorCtxClassAddFieldTest)
{
    std::string source = R"(
        .record std.core.TypeCreatorCtx {}
        .record std.core.String {}
        .function std.core.String std.core.TypeCreatorCtx.classAddField(i64 a0, std.core.String a1, std.core.String a2, i32 a3, i32 a4) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_TYPE_CREATOR_CTX_CLASS_ADD_FIELD,
                 "std.core.TypeCreatorCtx");

    source = R"(
        .record std.core.TypeCreatorCtx {}
        .function void std.core.TypeCreatorCtx.classAddField() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeCreatorCtx");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPITypeCreatorCtxInterfaceCreateTest)
{
    std::string source = R"(
        .record std.core.TypeCreatorCtx {}
        .record std.core.String {}
        .function i64 std.core.TypeCreatorCtx.interfaceCreate(i64 a0, std.core.String a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_TYPE_CREATOR_CTX_INTERFACE_CREATE,
                 "std.core.TypeCreatorCtx");

    source = R"(
        .record std.core.TypeCreatorCtx {}
        .function void std.core.TypeCreatorCtx.interfaceCreate() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeCreatorCtx");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPITypeCreatorCtxInterfaceAddBaseTest)
{
    std::string source = R"(
        .record std.core.TypeCreatorCtx {}
        .record std.core.String {}
        .function std.core.String std.core.TypeCreatorCtx.interfaceAddBase(i64 a0, std.core.String a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_TYPE_CREATOR_CTX_INTERFACE_ADD_BASE,
                 "std.core.TypeCreatorCtx");

    source = R"(
        .record std.core.TypeCreatorCtx {}
        .function void std.core.TypeCreatorCtx.interfaceAddBase() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeCreatorCtx");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPITypeCreatorCtxMethodCreateTest)
{
    std::string source = R"(
        .record std.core.TypeCreatorCtx {}
        .record std.core.String {}
        .function i64 std.core.TypeCreatorCtx.methodCreate(i64 a0, std.core.String a1, i32 a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_TYPE_CREATOR_CTX_METHOD_CREATE,
                 "std.core.TypeCreatorCtx");

    source = R"(
        .record std.core.TypeCreatorCtx {}
        .function void std.core.TypeCreatorCtx.methodCreate() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeCreatorCtx");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPITypeCreatorCtxMethodAddAccessModTest)
{
    std::string source = R"(
        .record std.core.TypeCreatorCtx {}
        .record std.core.String {}
        .function std.core.String std.core.TypeCreatorCtx.methodAddAccessMod(i64 a0, i32 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_TYPE_CREATOR_CTX_METHOD_ADD_ACCESS_MOD,
                 "std.core.TypeCreatorCtx");

    source = R"(
        .record std.core.TypeCreatorCtx {}
        .function void std.core.TypeCreatorCtx.methodAddAccessMod() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeCreatorCtx");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPITypeCreatorCtxMethodAddParamTest)
{
    std::string source = R"(
        .record std.core.TypeCreatorCtx {}
        .record std.core.String {}
        .function std.core.String std.core.TypeCreatorCtx.methodAddParam(i64 a0, std.core.String a1, std.core.String a2, i32 a3) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_TYPE_CREATOR_CTX_METHOD_ADD_PARAM,
                 "std.core.TypeCreatorCtx");

    source = R"(
        .record std.core.TypeCreatorCtx {}
        .function void std.core.TypeCreatorCtx.methodAddParam() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeCreatorCtx");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPITypeCreatorCtxMethodAddBodyFromMethodTest)
{
    std::string source = R"(
        .record std.core.TypeCreatorCtx {}
        .record std.core.String {}
        .record std.core.MethodType {}
        .function std.core.String std.core.TypeCreatorCtx.methodAddBodyFromMethod(i64 a0, std.core.MethodType a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_TYPE_CREATOR_CTX_METHOD_ADD_BODY_FROM_METHOD,
                 "std.core.TypeCreatorCtx");

    source = R"(
        .record std.core.TypeCreatorCtx {}
        .function void std.core.TypeCreatorCtx.methodAddBodyFromMethod() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeCreatorCtx");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPITypeCreatorCtxMethodAddBodyFromLambdaTest)
{
    std::string source = R"(
        .record std.core.TypeCreatorCtx {}
        .record std.core.String {}
        .record std.core.LambdaType {}
        .function std.core.String std.core.TypeCreatorCtx.methodAddBodyFromLambda(i64 a0, i32 a1, std.core.LambdaType a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_TYPE_CREATOR_CTX_METHOD_ADD_BODY_FROM_LAMBDA,
                 "std.core.TypeCreatorCtx");

    source = R"(
        .record std.core.TypeCreatorCtx {}
        .function void std.core.TypeCreatorCtx.methodAddBodyFromLambda() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeCreatorCtx");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPITypeCreatorCtxMethodAddBodyFromErasedLambdaTest)
{
    std::string source = R"(
        .record std.core.TypeCreatorCtx {}
        .record std.core.String {}
        .function std.core.String std.core.TypeCreatorCtx.methodAddBodyFromErasedLambda(i64 a0, i32 a1) <native>
    )";
    MainTestFunc(source,
                 RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_TYPE_CREATOR_CTX_METHOD_ADD_BODY_FROM_ERASED_LAMBDA,
                 "std.core.TypeCreatorCtx");

    source = R"(
        .record std.core.TypeCreatorCtx {}
        .function void std.core.TypeCreatorCtx.methodAddBodyFromErasedLambda() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeCreatorCtx");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPITypeCreatorCtxMethodAddBodyDefaultTest)
{
    std::string source = R"(
        .record std.core.TypeCreatorCtx {}
        .record std.core.String {}
        .function std.core.String std.core.TypeCreatorCtx.methodAddBodyDefault(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_TYPE_CREATOR_CTX_METHOD_ADD_BODY_DEFAULT,
                 "std.core.TypeCreatorCtx");

    source = R"(
        .record std.core.TypeCreatorCtx {}
        .function void std.core.TypeCreatorCtx.methodAddBodyDefault() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeCreatorCtx");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPITypeCreatorCtxMethodAddResultTest)
{
    std::string source = R"(
        .record std.core.TypeCreatorCtx {}
        .record std.core.String {}
        .function std.core.String std.core.TypeCreatorCtx.methodAddResult(i64 a0, std.core.String a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_TYPE_CREATOR_CTX_METHOD_ADD_RESULT,
                 "std.core.TypeCreatorCtx");

    source = R"(
        .record std.core.TypeCreatorCtx {}
        .function void std.core.TypeCreatorCtx.methodAddResult() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeCreatorCtx");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPITypeCreatorCtxMethodAddTest)
{
    std::string source = R"(
        .record std.core.TypeCreatorCtx {}
        .record std.core.String {}
        .function std.core.String std.core.TypeCreatorCtx.methodAdd(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_TYPE_CREATOR_CTX_METHOD_ADD,
                 "std.core.TypeCreatorCtx");

    source = R"(
        .record std.core.TypeCreatorCtx {}
        .function void std.core.TypeCreatorCtx.methodAdd() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeCreatorCtx");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPITypeCreatorCtxLambdaTypeCreateTest)
{
    std::string source = R"(
        .record std.core.TypeCreatorCtx {}
        .function i64 std.core.TypeCreatorCtx.lambdaTypeCreate(i64 a0, i32 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_TYPE_CREATOR_CTX_LAMBDA_TYPE_CREATE,
                 "std.core.TypeCreatorCtx");

    source = R"(
        .record std.core.TypeCreatorCtx {}
        .function void std.core.TypeCreatorCtx.lambdaTypeCreate() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeCreatorCtx");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPITypeCreatorCtxLambdaTypeAddParamTest)
{
    std::string source = R"(
        .record std.core.TypeCreatorCtx {}
        .record std.core.String {}
        .function std.core.String std.core.TypeCreatorCtx.lambdaTypeAddParam(i64 a0, std.core.String a1, i32 a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_TYPE_CREATOR_CTX_LAMBDA_TYPE_ADD_PARAM,
                 "std.core.TypeCreatorCtx");

    source = R"(
        .record std.core.TypeCreatorCtx {}
        .function void std.core.TypeCreatorCtx.lambdaTypeAddParam() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeCreatorCtx");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPITypeCreatorCtxLambdaTypeAddResultTest)
{
    std::string source = R"(
        .record std.core.TypeCreatorCtx {}
        .record std.core.String {}
        .function std.core.String std.core.TypeCreatorCtx.lambdaTypeAddResult(i64 a0, std.core.String a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_TYPE_CREATOR_CTX_LAMBDA_TYPE_ADD_RESULT,
                 "std.core.TypeCreatorCtx");

    source = R"(
        .record std.core.TypeCreatorCtx {}
        .function void std.core.TypeCreatorCtx.lambdaTypeAddResult() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeCreatorCtx");
}

TEST_F(RuntimeAdapterStaticTest, TypeAPITypeCreatorCtxLambdaTypeAddTest)
{
    std::string source = R"(
        .record std.core.TypeCreatorCtx {}
        .record std.core.String {}
        .function std.core.String std.core.TypeCreatorCtx.lambdaTypeAdd(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_TYPE_API_TYPE_CREATOR_CTX_LAMBDA_TYPE_ADD,
                 "std.core.TypeCreatorCtx");

    source = R"(
        .record std.core.TypeCreatorCtx {}
        .function void std.core.TypeCreatorCtx.lambdaTypeAdd() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeCreatorCtx");
}

TEST_F(RuntimeAdapterStaticTest, ConcurrencyHelpersMutexCreateTest)
{
    std::string source = R"(
        .record std.containers.ConcurrencyHelpers {}
        .record std.core.Object {}
        .function std.core.Object std.containers.ConcurrencyHelpers.mutexCreate() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_CONCURRENCY_HELPERS_MUTEX_CREATE,
                 "std.containers.ConcurrencyHelpers");

    source = R"(
        .record std.containers.ConcurrencyHelpers {}
        .function void std.containers.ConcurrencyHelpers.mutexCreate() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.containers.ConcurrencyHelpers");
}

TEST_F(RuntimeAdapterStaticTest, ConcurrencyHelpersMutexLockTest)
{
    std::string source = R"(
        .record std.containers.ConcurrencyHelpers {}
        .record std.core.Object {}
        .function void std.containers.ConcurrencyHelpers.mutexLock(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_CONCURRENCY_HELPERS_MUTEX_LOCK,
                 "std.containers.ConcurrencyHelpers");

    source = R"(
        .record std.containers.ConcurrencyHelpers {}
        .function i32 std.containers.ConcurrencyHelpers.mutexLock() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.containers.ConcurrencyHelpers");
}

TEST_F(RuntimeAdapterStaticTest, ConcurrencyHelpersMutexUnlockTest)
{
    std::string source = R"(
        .record std.containers.ConcurrencyHelpers {}
        .record std.core.Object {}
        .function void std.containers.ConcurrencyHelpers.mutexUnlock(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_CONCURRENCY_HELPERS_MUTEX_UNLOCK,
                 "std.containers.ConcurrencyHelpers");

    source = R"(
        .record std.containers.ConcurrencyHelpers {}
        .function i32 std.containers.ConcurrencyHelpers.mutexUnlock() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.containers.ConcurrencyHelpers");
}

TEST_F(RuntimeAdapterStaticTest, ConcurrencyHelpersEventCreateTest)
{
    std::string source = R"(
        .record std.containers.ConcurrencyHelpers {}
        .record std.core.Object {}
        .function std.core.Object std.containers.ConcurrencyHelpers.eventCreate() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_CONCURRENCY_HELPERS_EVENT_CREATE,
                 "std.containers.ConcurrencyHelpers");

    source = R"(
        .record std.containers.ConcurrencyHelpers {}
        .function void std.containers.ConcurrencyHelpers.eventCreate() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.containers.ConcurrencyHelpers");
}

TEST_F(RuntimeAdapterStaticTest, ConcurrencyHelpersEventWaitTest)
{
    std::string source = R"(
        .record std.containers.ConcurrencyHelpers {}
        .record std.core.Object {}
        .function void std.containers.ConcurrencyHelpers.eventWait(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_CONCURRENCY_HELPERS_EVENT_WAIT,
                 "std.containers.ConcurrencyHelpers");

    source = R"(
        .record std.containers.ConcurrencyHelpers {}
        .function i32 std.containers.ConcurrencyHelpers.eventWait() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.containers.ConcurrencyHelpers");
}

TEST_F(RuntimeAdapterStaticTest, ConcurrencyHelpersEventFireTest)
{
    std::string source = R"(
        .record std.containers.ConcurrencyHelpers {}
        .record std.core.Object {}
        .function void std.containers.ConcurrencyHelpers.eventFire(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_CONCURRENCY_HELPERS_EVENT_FIRE,
                 "std.containers.ConcurrencyHelpers");

    source = R"(
        .record std.containers.ConcurrencyHelpers {}
        .function i32 std.containers.ConcurrencyHelpers.eventFire() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.containers.ConcurrencyHelpers");
}

TEST_F(RuntimeAdapterStaticTest, ConcurrencyHelpersCondVarCreateTest)
{
    std::string source = R"(
        .record std.containers.ConcurrencyHelpers {}
        .record std.core.Object {}
        .function std.core.Object std.containers.ConcurrencyHelpers.condVarCreate() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_CONCURRENCY_HELPERS_COND_VAR_CREATE,
                 "std.containers.ConcurrencyHelpers");

    source = R"(
        .record std.containers.ConcurrencyHelpers {}
        .function void std.containers.ConcurrencyHelpers.condVarCreate() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.containers.ConcurrencyHelpers");
}

TEST_F(RuntimeAdapterStaticTest, ConcurrencyHelpersCondVarWaitTest)
{
    std::string source = R"(
        .record std.containers.ConcurrencyHelpers {}
        .record std.core.Object {}
        .function void std.containers.ConcurrencyHelpers.condVarWait(std.core.Object a0, std.core.Object a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_CONCURRENCY_HELPERS_COND_VAR_WAIT,
                 "std.containers.ConcurrencyHelpers");

    source = R"(
        .record std.containers.ConcurrencyHelpers {}
        .function i32 std.containers.ConcurrencyHelpers.condVarWait() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.containers.ConcurrencyHelpers");
}

TEST_F(RuntimeAdapterStaticTest, ConcurrencyHelpersCondVarNotifyOneTest)
{
    std::string source = R"(
        .record std.containers.ConcurrencyHelpers {}
        .record std.core.Object {}
        .function void std.containers.ConcurrencyHelpers.condVarNotifyOne(std.core.Object a0, std.core.Object a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_CONCURRENCY_HELPERS_COND_VAR_NOTIFY_ONE,
                 "std.containers.ConcurrencyHelpers");

    source = R"(
        .record std.containers.ConcurrencyHelpers {}
        .function i32 std.containers.ConcurrencyHelpers.condVarNotifyOne() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.containers.ConcurrencyHelpers");
}

TEST_F(RuntimeAdapterStaticTest, ConcurrencyHelpersCondVarNotifyAllTest)
{
    std::string source = R"(
        .record std.containers.ConcurrencyHelpers {}
        .record std.core.Object {}
        .function void std.containers.ConcurrencyHelpers.condVarNotifyAll(std.core.Object a0, std.core.Object a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_CONCURRENCY_HELPERS_COND_VAR_NOTIFY_ALL,
                 "std.containers.ConcurrencyHelpers");

    source = R"(
        .record std.containers.ConcurrencyHelpers {}
        .function i32 std.containers.ConcurrencyHelpers.condVarNotifyAll() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.containers.ConcurrencyHelpers");
}

TEST_F(RuntimeAdapterStaticTest, ConcurrencyHelpersSpinlockCreateTest)
{
    std::string source = R"(
        .record std.containers.ConcurrencyHelpers {}
        .record std.core.Object {}
        .function std.core.Object std.containers.ConcurrencyHelpers.spinlockCreate() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_CONCURRENCY_HELPERS_SPINLOCK_CREATE,
                 "std.containers.ConcurrencyHelpers");

    source = R"(
        .record std.containers.ConcurrencyHelpers {}
        .function void std.containers.ConcurrencyHelpers.spinlockCreate() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.containers.ConcurrencyHelpers");
}

TEST_F(RuntimeAdapterStaticTest, ConcurrencyHelpersSpinlockGuardTest)
{
    std::string source = R"(
        .record std.containers.ConcurrencyHelpers {}
        .record std.core.Object {}
        .function void std.containers.ConcurrencyHelpers.spinlockGuard(std.core.Object a0, std.core.Object a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_CONCURRENCY_HELPERS_SPINLOCK_GUARD,
                 "std.containers.ConcurrencyHelpers");

    source = R"(
        .record std.containers.ConcurrencyHelpers {}
        .function i32 std.containers.ConcurrencyHelpers.spinlockGuard() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.containers.ConcurrencyHelpers");
}

TEST_F(RuntimeAdapterStaticTest, StdDebugCountInstancesOfClassTest)
{
    std::string source = R"(
        .record std.debug.RuntimeDebug {}
        .record std.core.ClassType {}
        .function i32 std.debug.RuntimeDebug.countInstancesOfClass(std.core.ClassType a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_DEBUG_COUNT_INSTANCES_OF_CLASS,
                 "std.debug.RuntimeDebug");

    source = R"(
        .record std.debug.RuntimeDebug {}
        .function void std.debug.RuntimeDebug.countInstancesOfClass() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.debug.RuntimeDebug");
}

TEST_F(RuntimeAdapterStaticTest, DebuggerAPIGetLocalBooleanTest)
{
    std::string source = R"(
        .record std.debug.DebuggerAPI {}
        .function u1 std.debug.DebuggerAPI.getLocalBoolean(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_DEBUGGER_API_GET_LOCAL_BOOLEAN,
                 "std.debug.DebuggerAPI");

    source = R"(
        .record std.debug.DebuggerAPI {}
        .function void std.debug.DebuggerAPI.getLocalBoolean() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.debug.DebuggerAPI");
}

TEST_F(RuntimeAdapterStaticTest, DebuggerAPIGetLocalByteTest)
{
    std::string source = R"(
        .record std.debug.DebuggerAPI {}
        .function i8 std.debug.DebuggerAPI.getLocalByte(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_DEBUGGER_API_GET_LOCAL_BYTE, "std.debug.DebuggerAPI");

    source = R"(
        .record std.debug.DebuggerAPI {}
        .function void std.debug.DebuggerAPI.getLocalByte() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.debug.DebuggerAPI");
}

TEST_F(RuntimeAdapterStaticTest, DebuggerAPIGetLocalShortTest)
{
    std::string source = R"(
        .record std.debug.DebuggerAPI {}
        .function i16 std.debug.DebuggerAPI.getLocalShort(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_DEBUGGER_API_GET_LOCAL_SHORT,
                 "std.debug.DebuggerAPI");

    source = R"(
        .record std.debug.DebuggerAPI {}
        .function void std.debug.DebuggerAPI.getLocalShort() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.debug.DebuggerAPI");
}

TEST_F(RuntimeAdapterStaticTest, DebuggerAPIGetLocalCharTest)
{
    std::string source = R"(
        .record std.debug.DebuggerAPI {}
        .function u16 std.debug.DebuggerAPI.getLocalChar(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_DEBUGGER_API_GET_LOCAL_CHAR, "std.debug.DebuggerAPI");

    source = R"(
        .record std.debug.DebuggerAPI {}
        .function void std.debug.DebuggerAPI.getLocalChar() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.debug.DebuggerAPI");
}

TEST_F(RuntimeAdapterStaticTest, DebuggerAPIGetLocalIntTest)
{
    std::string source = R"(
        .record std.debug.DebuggerAPI {}
        .function i32 std.debug.DebuggerAPI.getLocalInt(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_DEBUGGER_API_GET_LOCAL_INT, "std.debug.DebuggerAPI");

    source = R"(
        .record std.debug.DebuggerAPI {}
        .function void std.debug.DebuggerAPI.getLocalInt() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.debug.DebuggerAPI");
}

TEST_F(RuntimeAdapterStaticTest, DebuggerAPIGetLocalFloatTest)
{
    std::string source = R"(
        .record std.debug.DebuggerAPI {}
        .function f32 std.debug.DebuggerAPI.getLocalFloat(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_DEBUGGER_API_GET_LOCAL_FLOAT,
                 "std.debug.DebuggerAPI");

    source = R"(
        .record std.debug.DebuggerAPI {}
        .function void std.debug.DebuggerAPI.getLocalFloat() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.debug.DebuggerAPI");
}

TEST_F(RuntimeAdapterStaticTest, DebuggerAPIGetLocalDoubleTest)
{
    std::string source = R"(
        .record std.debug.DebuggerAPI {}
        .function f64 std.debug.DebuggerAPI.getLocalDouble(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_DEBUGGER_API_GET_LOCAL_DOUBLE,
                 "std.debug.DebuggerAPI");

    source = R"(
        .record std.debug.DebuggerAPI {}
        .function void std.debug.DebuggerAPI.getLocalDouble() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.debug.DebuggerAPI");
}

TEST_F(RuntimeAdapterStaticTest, DebuggerAPIGetLocalLongTest)
{
    std::string source = R"(
        .record std.debug.DebuggerAPI {}
        .function i64 std.debug.DebuggerAPI.getLocalLong(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_DEBUGGER_API_GET_LOCAL_LONG, "std.debug.DebuggerAPI");

    source = R"(
        .record std.debug.DebuggerAPI {}
        .function void std.debug.DebuggerAPI.getLocalLong() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.debug.DebuggerAPI");
}

TEST_F(RuntimeAdapterStaticTest, DebuggerAPIGetLocalObjectTest)
{
    std::string source = R"(
        .record std.debug.DebuggerAPI {}
        .record std.core.Object {}
        .function std.core.Object std.debug.DebuggerAPI.getLocalObject(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_DEBUGGER_API_GET_LOCAL_OBJECT,
                 "std.debug.DebuggerAPI");

    source = R"(
        .record std.debug.DebuggerAPI {}
        .function void std.debug.DebuggerAPI.getLocalObject() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.debug.DebuggerAPI");
}

TEST_F(RuntimeAdapterStaticTest, DebuggerAPISetLocalBooleanTest)
{
    std::string source = R"(
        .record std.debug.DebuggerAPI {}
        .function void std.debug.DebuggerAPI.setLocalBoolean(i64 a0, u1 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_DEBUGGER_API_SET_LOCAL_BOOLEAN,
                 "std.debug.DebuggerAPI");

    source = R"(
        .record std.debug.DebuggerAPI {}
        .function i32 std.debug.DebuggerAPI.setLocalBoolean() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.debug.DebuggerAPI");
}

TEST_F(RuntimeAdapterStaticTest, DebuggerAPISetLocalByteTest)
{
    std::string source = R"(
        .record std.debug.DebuggerAPI {}
        .function void std.debug.DebuggerAPI.setLocalByte(i64 a0, i8 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_DEBUGGER_API_SET_LOCAL_BYTE, "std.debug.DebuggerAPI");

    source = R"(
        .record std.debug.DebuggerAPI {}
        .function i32 std.debug.DebuggerAPI.setLocalByte() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.debug.DebuggerAPI");
}

TEST_F(RuntimeAdapterStaticTest, DebuggerAPISetLocalShortTest)
{
    std::string source = R"(
        .record std.debug.DebuggerAPI {}
        .function void std.debug.DebuggerAPI.setLocalShort(i64 a0, i16 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_DEBUGGER_API_SET_LOCAL_SHORT,
                 "std.debug.DebuggerAPI");

    source = R"(
        .record std.debug.DebuggerAPI {}
        .function i32 std.debug.DebuggerAPI.setLocalShort() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.debug.DebuggerAPI");
}

TEST_F(RuntimeAdapterStaticTest, DebuggerAPISetLocalCharTest)
{
    std::string source = R"(
        .record std.debug.DebuggerAPI {}
        .function void std.debug.DebuggerAPI.setLocalChar(i64 a0, u16 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_DEBUGGER_API_SET_LOCAL_CHAR, "std.debug.DebuggerAPI");

    source = R"(
        .record std.debug.DebuggerAPI {}
        .function i32 std.debug.DebuggerAPI.setLocalChar() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.debug.DebuggerAPI");
}

TEST_F(RuntimeAdapterStaticTest, DebuggerAPISetLocalIntTest)
{
    std::string source = R"(
        .record std.debug.DebuggerAPI {}
        .function void std.debug.DebuggerAPI.setLocalInt(i64 a0, i32 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_DEBUGGER_API_SET_LOCAL_INT, "std.debug.DebuggerAPI");

    source = R"(
        .record std.debug.DebuggerAPI {}
        .function i32 std.debug.DebuggerAPI.setLocalInt() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.debug.DebuggerAPI");
}

TEST_F(RuntimeAdapterStaticTest, DebuggerAPISetLocalFloatTest)
{
    std::string source = R"(
        .record std.debug.DebuggerAPI {}
        .function void std.debug.DebuggerAPI.setLocalFloat(i64 a0, f32 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_DEBUGGER_API_SET_LOCAL_FLOAT,
                 "std.debug.DebuggerAPI");

    source = R"(
        .record std.debug.DebuggerAPI {}
        .function i32 std.debug.DebuggerAPI.setLocalFloat() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.debug.DebuggerAPI");
}

TEST_F(RuntimeAdapterStaticTest, DebuggerAPISetLocalDoubleTest)
{
    std::string source = R"(
        .record std.debug.DebuggerAPI {}
        .function void std.debug.DebuggerAPI.setLocalDouble(i64 a0, f64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_DEBUGGER_API_SET_LOCAL_DOUBLE,
                 "std.debug.DebuggerAPI");

    source = R"(
        .record std.debug.DebuggerAPI {}
        .function i32 std.debug.DebuggerAPI.setLocalDouble() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.debug.DebuggerAPI");
}

TEST_F(RuntimeAdapterStaticTest, DebuggerAPISetLocalLongTest)
{
    std::string source = R"(
        .record std.debug.DebuggerAPI {}
        .function void std.debug.DebuggerAPI.setLocalLong(i64 a0, i64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_DEBUGGER_API_SET_LOCAL_LONG, "std.debug.DebuggerAPI");

    source = R"(
        .record std.debug.DebuggerAPI {}
        .function i32 std.debug.DebuggerAPI.setLocalLong() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.debug.DebuggerAPI");
}

TEST_F(RuntimeAdapterStaticTest, DebuggerAPISetLocalObjectTest)
{
    std::string source = R"(
        .record std.debug.DebuggerAPI {}
        .record std.core.Object {}
        .function void std.debug.DebuggerAPI.setLocalObject(i64 a0, std.core.Object a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_DEBUGGER_API_SET_LOCAL_OBJECT,
                 "std.debug.DebuggerAPI");

    source = R"(
        .record std.debug.DebuggerAPI {}
        .function i32 std.debug.DebuggerAPI.setLocalObject() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.debug.DebuggerAPI");
}

TEST_F(RuntimeAdapterStaticTest, escompatConcurrencyHelpersMutexCreateTest)
{
    std::string source = R"(
        .record escompat.ConcurrencyHelpers {}
        .record std.core.Object {}
        .function std.core.Object escompat.ConcurrencyHelpers.mutexCreate() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_ESCOMPAT_CONCURRENCY_HELPERS_MUTEX_CREATE,
                 "escompat.ConcurrencyHelpers");

    source = R"(
        .record escompat.ConcurrencyHelpers {}
        .function void escompat.ConcurrencyHelpers.mutexCreate() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "escompat.ConcurrencyHelpers");
}

TEST_F(RuntimeAdapterStaticTest, escompatConcurrencyHelpersMutexLockTest)
{
    std::string source = R"(
        .record escompat.ConcurrencyHelpers {}
        .record std.core.Object {}
        .function void escompat.ConcurrencyHelpers.mutexLock(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_ESCOMPAT_CONCURRENCY_HELPERS_MUTEX_LOCK,
                 "escompat.ConcurrencyHelpers");

    source = R"(
        .record escompat.ConcurrencyHelpers {}
        .function i32 escompat.ConcurrencyHelpers.mutexLock() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "escompat.ConcurrencyHelpers");
}

TEST_F(RuntimeAdapterStaticTest, escompatConcurrencyHelpersMutexUnlockTest)
{
    std::string source = R"(
        .record escompat.ConcurrencyHelpers {}
        .record std.core.Object {}
        .function void escompat.ConcurrencyHelpers.mutexUnlock(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_ESCOMPAT_CONCURRENCY_HELPERS_MUTEX_UNLOCK,
                 "escompat.ConcurrencyHelpers");

    source = R"(
        .record escompat.ConcurrencyHelpers {}
        .function i32 escompat.ConcurrencyHelpers.mutexUnlock() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "escompat.ConcurrencyHelpers");
}

TEST_F(RuntimeAdapterStaticTest, escompatConcurrencyHelpersEventCreateTest)
{
    std::string source = R"(
        .record escompat.ConcurrencyHelpers {}
        .record std.core.Object {}
        .function std.core.Object escompat.ConcurrencyHelpers.eventCreate() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_ESCOMPAT_CONCURRENCY_HELPERS_EVENT_CREATE,
                 "escompat.ConcurrencyHelpers");

    source = R"(
        .record escompat.ConcurrencyHelpers {}
        .function void escompat.ConcurrencyHelpers.eventCreate() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "escompat.ConcurrencyHelpers");
}

TEST_F(RuntimeAdapterStaticTest, escompatConcurrencyHelpersEventWaitTest)
{
    std::string source = R"(
        .record escompat.ConcurrencyHelpers {}
        .record std.core.Object {}
        .function void escompat.ConcurrencyHelpers.eventWait(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_ESCOMPAT_CONCURRENCY_HELPERS_EVENT_WAIT,
                 "escompat.ConcurrencyHelpers");

    source = R"(
        .record escompat.ConcurrencyHelpers {}
        .function i32 escompat.ConcurrencyHelpers.eventWait() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "escompat.ConcurrencyHelpers");
}

TEST_F(RuntimeAdapterStaticTest, escompatConcurrencyHelpersEventFireTest)
{
    std::string source = R"(
        .record escompat.ConcurrencyHelpers {}
        .record std.core.Object {}
        .function void escompat.ConcurrencyHelpers.eventFire(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_ESCOMPAT_CONCURRENCY_HELPERS_EVENT_FIRE,
                 "escompat.ConcurrencyHelpers");

    source = R"(
        .record escompat.ConcurrencyHelpers {}
        .function i32 escompat.ConcurrencyHelpers.eventFire() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "escompat.ConcurrencyHelpers");
}

TEST_F(RuntimeAdapterStaticTest, stdCoreReflectIsInterfaceLiteralTest)
{
    std::string source = R"(
        .record std.core.Reflect {}
        .record std.core.Object {}
        .function u1 std.core.Reflect.isLiteralInitializedInterfaceImpl(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_REFLECT_IS_INTERFACE_LITERAL,
                 "std.core.Reflect");

    source = R"(
        .record std.core.Reflect {}
        .function void std.core.Reflect.isLiteralInitializedInterfaceImpl() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Reflect");
}

TEST_F(RuntimeAdapterStaticTest, stdConcurrencyConcurrencyHelpersMutexCreateTest)
{
    std::string source = R"(
        .record std.concurrency.ConcurrencyHelpers {}
        .record std.core.Object {}
        .function std.core.Object std.concurrency.ConcurrencyHelpers.mutexCreate() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CONCURRENCY_CONCURRENCY_HELPERS_MUTEX_CREATE,
                 "std.concurrency.ConcurrencyHelpers");

    source = R"(
        .record std.concurrency.ConcurrencyHelpers {}
        .function void std.concurrency.ConcurrencyHelpers.mutexCreate() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.concurrency.ConcurrencyHelpers");
}

TEST_F(RuntimeAdapterStaticTest, stdConcurrencyConcurrencyHelpersMutexLockTest)
{
    std::string source = R"(
        .record std.concurrency.ConcurrencyHelpers {}
        .record std.core.Object {}
        .function void std.concurrency.ConcurrencyHelpers.mutexLock(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CONCURRENCY_CONCURRENCY_HELPERS_MUTEX_LOCK,
                 "std.concurrency.ConcurrencyHelpers");

    source = R"(
        .record std.concurrency.ConcurrencyHelpers {}
        .function i32 std.concurrency.ConcurrencyHelpers.mutexLock() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.concurrency.ConcurrencyHelpers");
}

TEST_F(RuntimeAdapterStaticTest, stdConcurrencyConcurrencyHelpersMutexUnlockTest)
{
    std::string source = R"(
        .record std.concurrency.ConcurrencyHelpers {}
        .record std.core.Object {}
        .function void std.concurrency.ConcurrencyHelpers.mutexUnlock(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CONCURRENCY_CONCURRENCY_HELPERS_MUTEX_UNLOCK,
                 "std.concurrency.ConcurrencyHelpers");

    source = R"(
        .record std.concurrency.ConcurrencyHelpers {}
        .function i32 std.concurrency.ConcurrencyHelpers.mutexUnlock() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.concurrency.ConcurrencyHelpers");
}

TEST_F(RuntimeAdapterStaticTest, stdConcurrencyHelpersEventCreateTest)
{
    std::string source = R"(
        .record std.concurrency.ConcurrencyHelpers {}
        .record std.core.Object {}
        .function std.core.Object std.concurrency.ConcurrencyHelpers.eventCreate() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CONCURRENCY_HELPERS_EVENT_CREATE,
                 "std.concurrency.ConcurrencyHelpers");

    source = R"(
        .record std.concurrency.ConcurrencyHelpers {}
        .function void std.concurrency.ConcurrencyHelpers.eventCreate() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.concurrency.ConcurrencyHelpers");
}

TEST_F(RuntimeAdapterStaticTest, stdConcurrencyHelpersEventWaitTest)
{
    std::string source = R"(
        .record std.concurrency.ConcurrencyHelpers {}
        .record std.core.Object {}
        .function void std.concurrency.ConcurrencyHelpers.eventWait(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CONCURRENCY_HELPERS_EVENT_WAIT,
                 "std.concurrency.ConcurrencyHelpers");

    source = R"(
        .record std.concurrency.ConcurrencyHelpers {}
        .function i32 std.concurrency.ConcurrencyHelpers.eventWait() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.concurrency.ConcurrencyHelpers");
}

TEST_F(RuntimeAdapterStaticTest, stdConcurrencyHelpersEventFireTest)
{
    std::string source = R"(
        .record std.concurrency.ConcurrencyHelpers {}
        .record std.core.Object {}
        .function void std.concurrency.ConcurrencyHelpers.eventFire(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CONCURRENCY_HELPERS_EVENT_FIRE,
                 "std.concurrency.ConcurrencyHelpers");

    source = R"(
        .record std.concurrency.ConcurrencyHelpers {}
        .function i32 std.concurrency.ConcurrencyHelpers.eventFire() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.concurrency.ConcurrencyHelpers");
}

TEST_F(RuntimeAdapterStaticTest, stdConcurrencyHelpersSpinlockCreateTest)
{
    std::string source = R"(
        .record std.concurrency.ConcurrencyHelpers {}
        .record std.core.Object {}
        .function std.core.Object std.concurrency.ConcurrencyHelpers.spinlockCreate() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CONCURRENCY_HELPERS_SPINLOCK_CREATE,
                 "std.concurrency.ConcurrencyHelpers");

    source = R"(
        .record std.concurrency.ConcurrencyHelpers {}
        .function void std.concurrency.ConcurrencyHelpers.spinlockCreate() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.concurrency.ConcurrencyHelpers");
}

TEST_F(RuntimeAdapterStaticTest, stdConcurrencyHelpersSpinlockGuardTest)
{
    std::string source = R"(
        .record std.concurrency.ConcurrencyHelpers {}
        .record std.core.Object {}
        .function void std.concurrency.ConcurrencyHelpers.spinlockGuard(std.core.Object a0, std.core.Object a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CONCURRENCY_HELPERS_SPINLOCK_GUARD,
                 "std.concurrency.ConcurrencyHelpers");

    source = R"(
        .record std.concurrency.ConcurrencyHelpers {}
        .function i32 std.concurrency.ConcurrencyHelpers.spinlockGuard() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.concurrency.ConcurrencyHelpers");
}

TEST_F(RuntimeAdapterStaticTest, stdConcurrencyConcurrencyHelpersCondVarCreateTest)
{
    std::string source = R"(
        .record std.concurrency.ConcurrencyHelpers {}
        .record std.core.Object {}
        .function std.core.Object std.concurrency.ConcurrencyHelpers.condVarCreate() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CONCURRENCY_CONCURRENCY_HELPERS_COND_VAR_CREATE,
                 "std.concurrency.ConcurrencyHelpers");

    source = R"(
        .record std.concurrency.ConcurrencyHelpers {}
        .function void std.concurrency.ConcurrencyHelpers.condVarCreate() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.concurrency.ConcurrencyHelpers");
}

TEST_F(RuntimeAdapterStaticTest, escompatConcurrencyHelpersCondVarWaitTest)
{
    std::string source = R"(
        .record std.concurrency.ConcurrencyHelpers {}
        .record std.core.Object {}
        .function void std.concurrency.ConcurrencyHelpers.condVarWait(std.core.Object a0, std.core.Object a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_ESCOMPAT_CONCURRENCY_HELPERS_COND_VAR_WAIT,
                 "std.concurrency.ConcurrencyHelpers");

    source = R"(
        .record std.concurrency.ConcurrencyHelpers {}
        .function i32 std.concurrency.ConcurrencyHelpers.condVarWait() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.concurrency.ConcurrencyHelpers");
}

TEST_F(RuntimeAdapterStaticTest, stdConcurrencyConcurrencyHelpersCondVarNotifyOneTest)
{
    std::string source = R"(
        .record std.concurrency.ConcurrencyHelpers {}
        .record std.core.Object {}
        .function void std.concurrency.ConcurrencyHelpers.condVarNotifyOne(std.core.Object a0, std.core.Object a1) <native>
    )";
    MainTestFunc(source,
                 RuntimeInterface::IntrinsicId::INTRINSIC_STD_CONCURRENCY_CONCURRENCY_HELPERS_COND_VAR_NOTIFY_ONE,
                 "std.concurrency.ConcurrencyHelpers");

    source = R"(
        .record std.concurrency.ConcurrencyHelpers {}
        .function i32 std.concurrency.ConcurrencyHelpers.condVarNotifyOne() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.concurrency.ConcurrencyHelpers");
}

TEST_F(RuntimeAdapterStaticTest, stdConcurrencyConcurrencyHelpersCondVarNotifyAllTest)
{
    std::string source = R"(
        .record std.concurrency.ConcurrencyHelpers {}
        .record std.core.Object {}
        .function void std.concurrency.ConcurrencyHelpers.condVarNotifyAll(std.core.Object a0, std.core.Object a1) <native>
    )";
    MainTestFunc(source,
                 RuntimeInterface::IntrinsicId::INTRINSIC_STD_CONCURRENCY_CONCURRENCY_HELPERS_COND_VAR_NOTIFY_ALL,
                 "std.concurrency.ConcurrencyHelpers");

    source = R"(
        .record std.concurrency.ConcurrencyHelpers {}
        .function i32 std.concurrency.ConcurrencyHelpers.condVarNotifyAll() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.concurrency.ConcurrencyHelpers");
}

TEST_F(RuntimeAdapterStaticTest, stdConcurrencyNativeAsyncWorkHelperAsyncWorkNativeInvokeTest)
{
    std::string source = R"(
        .record std.concurrency.NativeAsyncWorkHelper {}
        .function void std.concurrency.NativeAsyncWorkHelper.asyncWorkNativeInvoke(i64 a0, i64 a1, u1 a2) <native>
    )";
    MainTestFunc(
        source,
        RuntimeInterface::IntrinsicId::INTRINSIC_STD_CONCURRENCY_NATIVE_ASYNC_WORK_HELPER_ASYNC_WORK_NATIVE_INVOKE,
        "std.concurrency.NativeAsyncWorkHelper");

    source = R"(
        .record std.concurrency.NativeAsyncWorkHelper {}
        .function i32 std.concurrency.NativeAsyncWorkHelper.asyncWorkNativeInvoke() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.concurrency.NativeAsyncWorkHelper");
}

TEST_F(RuntimeAdapterStaticTest, stdConcurrencyNativeAsyncWorkHelperEventNativeInvokeTest)
{
    std::string source = R"(
        .record std.concurrency.NativeAsyncWorkHelper {}
        .function void std.concurrency.NativeAsyncWorkHelper.eventNativeInvoke(i64 a0, i64 a1, u1 a2) <native>
    )";
    MainTestFunc(source,
                 RuntimeInterface::IntrinsicId::INTRINSIC_STD_CONCURRENCY_NATIVE_ASYNC_WORK_HELPER_EVENT_NATIVE_INVOKE,
                 "std.concurrency.NativeAsyncWorkHelper");

    source = R"(
        .record std.concurrency.NativeAsyncWorkHelper {}
        .function i32 std.concurrency.NativeAsyncWorkHelper.eventNativeInvoke() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.concurrency.NativeAsyncWorkHelper");
}

TEST_F(RuntimeAdapterStaticTest, StdConcurrencyWorkerGroupGenerateGroupIdImplTest)
{
    std::string source = R"(
        .record std.concurrency.WorkerGroup {}
        .record escompat.Array {}
        .function i64[] std.concurrency.WorkerGroup.generateGroupIdImpl(i32 a0, escompat.Array a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CONCURRENCY_WORKER_GROUP_GENERATE_GROUP_ID_IMPL,
                 "std.concurrency.WorkerGroup");

    source = R"(
        .record std.concurrency.WorkerGroup {}
        .function void std.concurrency.WorkerGroup.generateGroupIdImpl() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.concurrency.WorkerGroup");
}

TEST_F(RuntimeAdapterStaticTest, stdCoreConcurrencyHelpersMutexCreateTest)
{
    std::string source = R"(
        .record std.core.ConcurrencyHelpers {}
        .record std.core.Object {}
        .function std.core.Object std.core.ConcurrencyHelpers.mutexCreate() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_CONCURRENCY_HELPERS_MUTEX_CREATE,
                 "std.core.ConcurrencyHelpers");

    source = R"(
        .record std.core.ConcurrencyHelpers {}
        .function void std.core.ConcurrencyHelpers.mutexCreate() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ConcurrencyHelpers");
}

TEST_F(RuntimeAdapterStaticTest, stdCoreConcurrencyHelpersMutexLockTest)
{
    std::string source = R"(
        .record std.core.ConcurrencyHelpers {}
        .record std.core.Object {}
        .function void std.core.ConcurrencyHelpers.mutexLock(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_CONCURRENCY_HELPERS_MUTEX_LOCK,
                 "std.core.ConcurrencyHelpers");

    source = R"(
        .record std.core.ConcurrencyHelpers {}
        .function i32 std.core.ConcurrencyHelpers.mutexLock() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ConcurrencyHelpers");
}

TEST_F(RuntimeAdapterStaticTest, stdCoreConcurrencyHelpersMutexUnlockTest)
{
    std::string source = R"(
        .record std.core.ConcurrencyHelpers {}
        .record std.core.Object {}
        .function void std.core.ConcurrencyHelpers.mutexUnlock(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_CONCURRENCY_HELPERS_MUTEX_UNLOCK,
                 "std.core.ConcurrencyHelpers");

    source = R"(
        .record std.core.ConcurrencyHelpers {}
        .function i32 std.core.ConcurrencyHelpers.mutexUnlock() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ConcurrencyHelpers");
}

TEST_F(RuntimeAdapterStaticTest, StdExclusiveLauncherLaunchTest)
{
    std::string source = R"(
        .record std.core.ExclusiveLauncher {}
        .record std.core.String {}
        .record std.core.Object {}
        .function i32 std.core.ExclusiveLauncher.elaunch(std.core.Object a0, u1 a1, std.core.String a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_EXCLUSIVE_LAUNCHER_LAUNCH,
                 "std.core.ExclusiveLauncher");

    source = R"(
        .record std.core.ExclusiveLauncher {}
        .function void std.core.ExclusiveLauncher.elaunch() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ExclusiveLauncher");
}

TEST_F(RuntimeAdapterStaticTest, EtsEAWorkerSetPriorityTest)
{
    std::string source = R"(
        .record std.core.InternalWorker {}
        .function void std.core.InternalWorker.setCurrentWorkerPriority(i32 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_ETS_EA_WORKER_SET_PRIORITY,
                 "std.core.InternalWorker");

    source = R"(
        .record std.core.InternalWorker {}
        .function i32 std.core.InternalWorker.setCurrentWorkerPriority() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.InternalWorker");
}

TEST_F(RuntimeAdapterStaticTest, EtsEAWorkerJoinTest)
{
    std::string source = R"(
        .record std.core.InternalWorker {}
        .record std.core.Object {}
        .function void std.core.InternalWorker.join(i32 a0, std.core.Object a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_ETS_EA_WORKER_JOIN, "std.core.InternalWorker");

    source = R"(
        .record std.core.InternalWorker {}
        .function i32 std.core.InternalWorker.join() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.InternalWorker");
}

TEST_F(RuntimeAdapterStaticTest, EtsLaunchInternalJobNativeTest)
{
    std::string source = R"(
        .record std.concurrency.ETSGLOBAL {}
        .record std.core.Object {}
        .record std.core.Job {}
        .function std.core.Job std.concurrency.ETSGLOBAL.launchInternalJobNative(std.core.Object a0, std.core.Object[] a1, u1 a2, i64[] a3) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_ETS_LAUNCH_INTERNAL_JOB_NATIVE,
                 "std.concurrency.ETSGLOBAL");

    source = R"(
        .record std.concurrency.ETSGLOBAL {}
        .function void std.concurrency.ETSGLOBAL.launchInternalJobNative() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.concurrency.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, EtsLaunchOnTheSameWorkerTest)
{
    std::string source = R"(
        .record std.concurrency.ETSGLOBAL {}
        .record std.core.Object {}
        .function void std.concurrency.ETSGLOBAL.launchSameWorker(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_ETS_LAUNCH_ON_THE_SAME_WORKER,
                 "std.concurrency.ETSGLOBAL");

    source = R"(
        .record std.concurrency.ETSGLOBAL {}
        .function i32 std.concurrency.ETSGLOBAL.launchSameWorker() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.concurrency.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreByteToShortTest)
{
    std::string source = R"(
        .record std.core.Byte {}
        .function i16 std.core.Byte.toShort(i8 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_BYTE_TO_SHORT, "std.core.Byte");

    source = R"(
        .record std.core.Byte {}
        .function void std.core.Byte.toShort() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Byte");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreByteToIntTest)
{
    std::string source = R"(
        .record std.core.Byte {}
        .function i32 std.core.Byte.toInt(i8 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_BYTE_TO_INT, "std.core.Byte");

    source = R"(
        .record std.core.Byte {}
        .function void std.core.Byte.toInt() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Byte");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreByteToLongTest)
{
    std::string source = R"(
        .record std.core.Byte {}
        .function i64 std.core.Byte.toLong(i8 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_BYTE_TO_LONG, "std.core.Byte");

    source = R"(
        .record std.core.Byte {}
        .function void std.core.Byte.toLong() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Byte");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreByteToFloatTest)
{
    std::string source = R"(
        .record std.core.Byte {}
        .function f32 std.core.Byte.toFloat(i8 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_BYTE_TO_FLOAT, "std.core.Byte");

    source = R"(
        .record std.core.Byte {}
        .function void std.core.Byte.toFloat() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Byte");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreByteToDoubleTest)
{
    std::string source = R"(
        .record std.core.Byte {}
        .function f64 std.core.Byte.toDouble(i8 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_BYTE_TO_DOUBLE, "std.core.Byte");

    source = R"(
        .record std.core.Byte {}
        .function void std.core.Byte.toDouble() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Byte");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreByteToCharTest)
{
    std::string source = R"(
        .record std.core.Byte {}
        .function u16 std.core.Byte.toChar(i8 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_BYTE_TO_CHAR, "std.core.Byte");

    source = R"(
        .record std.core.Byte {}
        .function void std.core.Byte.toChar() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Byte");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreShortToByteTest)
{
    std::string source = R"(
        .record std.core.Short {}
        .function i8 std.core.Short.toByte(i16 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_SHORT_TO_BYTE, "std.core.Short");

    source = R"(
        .record std.core.Short {}
        .function void std.core.Short.toByte() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Short");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreShortToIntTest)
{
    std::string source = R"(
        .record std.core.Short {}
        .function i32 std.core.Short.toInt(i16 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_SHORT_TO_INT, "std.core.Short");

    source = R"(
        .record std.core.Short {}
        .function void std.core.Short.toInt() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Short");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreShortToLongTest)
{
    std::string source = R"(
        .record std.core.Short {}
        .function i64 std.core.Short.toLong(i16 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_SHORT_TO_LONG, "std.core.Short");

    source = R"(
        .record std.core.Short {}
        .function void std.core.Short.toLong() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Short");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreShortToFloatTest)
{
    std::string source = R"(
        .record std.core.Short {}
        .function f32 std.core.Short.toFloat(i16 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_SHORT_TO_FLOAT, "std.core.Short");

    source = R"(
        .record std.core.Short {}
        .function void std.core.Short.toFloat() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Short");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreShortToDoubleTest)
{
    std::string source = R"(
        .record std.core.Short {}
        .function f64 std.core.Short.toDouble(i16 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_SHORT_TO_DOUBLE, "std.core.Short");

    source = R"(
        .record std.core.Short {}
        .function void std.core.Short.toDouble() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Short");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreShortToCharTest)
{
    std::string source = R"(
        .record std.core.Short {}
        .function u16 std.core.Short.toChar(i16 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_SHORT_TO_CHAR, "std.core.Short");

    source = R"(
        .record std.core.Short {}
        .function void std.core.Short.toChar() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Short");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreIntToShortTest)
{
    std::string source = R"(
        .record std.core.Int {}
        .function i16 std.core.Int.toShort(i32 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_INT_TO_SHORT, "std.core.Int");

    source = R"(
        .record std.core.Int {}
        .function void std.core.Int.toShort() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Int");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreIntToByteTest)
{
    std::string source = R"(
        .record std.core.Int {}
        .function i8 std.core.Int.toByte(i32 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_INT_TO_BYTE, "std.core.Int");

    source = R"(
        .record std.core.Int {}
        .function void std.core.Int.toByte() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Int");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreIntToLongTest)
{
    std::string source = R"(
        .record std.core.Int {}
        .function i64 std.core.Int.toLong(i32 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_INT_TO_LONG, "std.core.Int");

    source = R"(
        .record std.core.Int {}
        .function void std.core.Int.toLong() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Int");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreIntToFloatTest)
{
    std::string source = R"(
        .record std.core.Int {}
        .function f32 std.core.Int.toFloat(i32 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_INT_TO_FLOAT, "std.core.Int");

    source = R"(
        .record std.core.Int {}
        .function void std.core.Int.toFloat() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Int");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreIntToDoubleTest)
{
    std::string source = R"(
        .record std.core.Int {}
        .function f64 std.core.Int.toDouble(i32 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_INT_TO_DOUBLE, "std.core.Int");

    source = R"(
        .record std.core.Int {}
        .function void std.core.Int.toDouble() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Int");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreIntToCharTest)
{
    std::string source = R"(
        .record std.core.Int {}
        .function u16 std.core.Int.toChar(i32 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_INT_TO_CHAR, "std.core.Int");

    source = R"(
        .record std.core.Int {}
        .function void std.core.Int.toChar() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Int");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreLongToShortTest)
{
    std::string source = R"(
        .record std.core.Long {}
        .function i16 std.core.Long.toShort(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_LONG_TO_SHORT, "std.core.Long");

    source = R"(
        .record std.core.Long {}
        .function void std.core.Long.toShort() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Long");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreLongToByteTest)
{
    std::string source = R"(
        .record std.core.Long {}
        .function i8 std.core.Long.toByte(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_LONG_TO_BYTE, "std.core.Long");

    source = R"(
        .record std.core.Long {}
        .function void std.core.Long.toByte() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Long");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreLongToIntTest)
{
    std::string source = R"(
        .record std.core.Long {}
        .function i32 std.core.Long.toInt(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_LONG_TO_INT, "std.core.Long");

    source = R"(
        .record std.core.Long {}
        .function void std.core.Long.toInt() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Long");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreLongToFloatTest)
{
    std::string source = R"(
        .record std.core.Long {}
        .function f32 std.core.Long.toFloat(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_LONG_TO_FLOAT, "std.core.Long");

    source = R"(
        .record std.core.Long {}
        .function void std.core.Long.toFloat() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Long");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreLongToDoubleTest)
{
    std::string source = R"(
        .record std.core.Long {}
        .function f64 std.core.Long.toDouble(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_LONG_TO_DOUBLE, "std.core.Long");

    source = R"(
        .record std.core.Long {}
        .function void std.core.Long.toDouble() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Long");
}

TEST_F(RuntimeAdapterStaticTest, StdCoreLongToCharTest)
{
    std::string source = R"(
        .record std.core.Long {}
        .function u16 std.core.Long.toChar(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_LONG_TO_CHAR, "std.core.Long");

    source = R"(
        .record std.core.Long {}
        .function void std.core.Long.toChar() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Long");
}

TEST_F(RuntimeAdapterStaticTest, ReflectProxyGenerateProxyTest)
{
    std::string source = R"(
        .record std.core.reflect.Proxy {}
        .record std.core.String {}
        .record std.core.RuntimeLinker {}
        .record std.core.Class {}
        .record escompat.Array {}
        .function std.core.Class std.core.reflect.Proxy.generateProxy(std.core.RuntimeLinker a0, std.core.String a1, std.core.Class[] a2, escompat.Array a3) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_REFLECT_PROXY_GENERATE_PROXY,
                 "std.core.reflect.Proxy");

    source = R"(
        .record std.core.reflect.Proxy {}
        .function void std.core.reflect.Proxy.generateProxy() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.reflect.Proxy");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeFinalizationRegistryCallbackTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.core.Object {}
        .function void std.interop.js.JSRuntime.finalizationRegistryCallback(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_FINALIZATION_REGISTRY_CALLBACK,
                 "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function i32 std.interop.js.JSRuntime.finalizationRegistryCallback() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeNewJSValueDoubleTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.interop.js.JSValue {}
        .function std.interop.js.JSValue std.interop.js.JSRuntime.newJSValueDouble(f64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_NEW_JS_VALUE_DOUBLE,
                 "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.newJSValueDouble() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeNewJSValueBooleanTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.interop.js.JSValue {}
        .function std.interop.js.JSValue std.interop.js.JSRuntime.newJSValueBoolean(u1 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_NEW_JS_VALUE_BOOLEAN,
                 "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.newJSValueBoolean() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeNewJSValueStringTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.core.String {}
        .record std.interop.js.JSValue {}
        .function std.interop.js.JSValue std.interop.js.JSRuntime.newJSValueString(std.core.String a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_NEW_JS_VALUE_STRING,
                 "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.newJSValueString() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeNewJSValueObjectTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.core.Object {}
        .record std.interop.js.JSValue {}
        .function std.interop.js.JSValue std.interop.js.JSRuntime.newJSValueObject(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_NEW_JS_VALUE_OBJECT,
                 "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.newJSValueObject() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeIsJSValueTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.core.Object {}
        .function u1 std.interop.js.JSRuntime.isJSValue(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_IS_JS_VALUE, "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.isJSValue() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeNewJSValueBigIntTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.interop.js.JSValue {}
        .record std.core.BigInt {}
        .function std.interop.js.JSValue std.interop.js.JSRuntime.newJSValueBigInt(std.core.BigInt a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_NEW_JS_VALUE_BIG_INT,
                 "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.newJSValueBigInt() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeGetValueDoubleTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.interop.js.JSValue {}
        .function f64 std.interop.js.JSRuntime.getValueDouble(std.interop.js.JSValue a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_GET_VALUE_DOUBLE,
                 "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.getValueDouble() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeGetValueBooleanTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.interop.js.JSValue {}
        .function u1 std.interop.js.JSRuntime.getValueBoolean(std.interop.js.JSValue a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_GET_VALUE_BOOLEAN,
                 "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.getValueBoolean() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeGetValueStringTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.core.String {}
        .record std.interop.js.JSValue {}
        .function std.core.String std.interop.js.JSRuntime.getValueString(std.interop.js.JSValue a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_GET_VALUE_STRING,
                 "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.getValueString() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeGetValueObjectTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.core.Object {}
        .record std.core.Class {}
        .function std.core.Object std.interop.js.JSRuntime.getValueObject(std.core.Object a0, std.core.Class a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_GET_VALUE_OBJECT,
                 "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.getValueObject() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeGetPropertyJSValueTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.core.String {}
        .record std.interop.js.JSValue {}
        .function std.interop.js.JSValue std.interop.js.JSRuntime.getPropertyJSValue(std.interop.js.JSValue a0, std.core.String a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_GET_PROPERTY_JS_VALUE,
                 "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.getPropertyJSValue() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeGetPropertyJSValueByKeyTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.interop.js.JSValue {}
        .function std.interop.js.JSValue std.interop.js.JSRuntime.getPropertyJSValue(std.interop.js.JSValue a0, std.interop.js.JSValue a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_GET_PROPERTY_JS_VALUE_BY_KEY,
                 "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.getPropertyJSValue() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeGetPropertyObjectTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.core.String {}
        .record std.core.Object {}
        .function std.core.Object std.interop.js.JSRuntime.getPropertyObject(std.core.Object a0, std.core.String a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_GET_PROPERTY_OBJECT,
                 "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.getPropertyObject() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeGetPropertyDoubleTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.core.String {}
        .record std.interop.js.JSValue {}
        .function f64 std.interop.js.JSRuntime.getPropertyDouble(std.interop.js.JSValue a0, std.core.String a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_GET_PROPERTY_DOUBLE,
                 "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.getPropertyDouble() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeGetPropertyStringTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.core.String {}
        .record std.interop.js.JSValue {}
        .function std.core.String std.interop.js.JSRuntime.getPropertyString(std.interop.js.JSValue a0, std.core.String a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_GET_PROPERTY_STRING,
                 "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.getPropertyString() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeGetPropertyBooleanTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.core.String {}
        .record std.interop.js.JSValue {}
        .function u1 std.interop.js.JSRuntime.getPropertyBoolean(std.interop.js.JSValue a0, std.core.String a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_GET_PROPERTY_BOOLEAN,
                 "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.getPropertyBoolean() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeSetPropertyJSValueTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.core.String {}
        .record std.interop.js.JSValue {}
        .function void std.interop.js.JSRuntime.setPropertyJSValue(std.interop.js.JSValue a0, std.core.String a1, std.interop.js.JSValue a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_SET_PROPERTY_JS_VALUE,
                 "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function i32 std.interop.js.JSRuntime.setPropertyJSValue() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimesetPropertyObjectTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.core.String {}
        .record std.core.Object {}
        .function void std.interop.js.JSRuntime.setPropertyObject(std.core.Object a0, std.core.String a1, std.core.Object a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIMESET_PROPERTY_OBJECT,
                 "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function i32 std.interop.js.JSRuntime.setPropertyObject() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeSetElementJSValueTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.interop.js.JSValue {}
        .function void std.interop.js.JSRuntime.setElementJSValue(std.interop.js.JSValue a0, i32 a1, std.interop.js.JSValue a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_SET_ELEMENT_JS_VALUE,
                 "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function i32 std.interop.js.JSRuntime.setElementJSValue() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeSetElementObjectTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.core.Object {}
        .function void std.interop.js.JSRuntime.setElementObject(std.core.Object a0, i32 a1, std.core.Object a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_SET_ELEMENT_OBJECT,
                 "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function i32 std.interop.js.JSRuntime.setElementObject() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeSetPropertyDoubleTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.core.String {}
        .record std.interop.js.JSValue {}
        .function void std.interop.js.JSRuntime.setPropertyDouble(std.interop.js.JSValue a0, std.core.String a1, f64 a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_SET_PROPERTY_DOUBLE,
                 "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function i32 std.interop.js.JSRuntime.setPropertyDouble() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeSetPropertyStringTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.core.String {}
        .record std.interop.js.JSValue {}
        .function void std.interop.js.JSRuntime.setPropertyString(std.interop.js.JSValue a0, std.core.String a1, std.core.String a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_SET_PROPERTY_STRING,
                 "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function i32 std.interop.js.JSRuntime.setPropertyString() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeSetPropertyBooleanTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.core.String {}
        .record std.interop.js.JSValue {}
        .function void std.interop.js.JSRuntime.setPropertyBoolean(std.interop.js.JSValue a0, std.core.String a1, u1 a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_SET_PROPERTY_BOOLEAN,
                 "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function i32 std.interop.js.JSRuntime.setPropertyBoolean() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeGetElementJSValueTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.interop.js.JSValue {}
        .function std.interop.js.JSValue std.interop.js.JSRuntime.getElementJSValue(std.interop.js.JSValue a0, i32 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_GET_ELEMENT_JS_VALUE,
                 "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.getElementJSValue() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeGetElementObjectTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.core.Object {}
        .function std.core.Object std.interop.js.JSRuntime.getElementObject(std.core.Object a0, i32 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_GET_ELEMENT_OBJECT,
                 "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.getElementObject() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeGetElementDoubleTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.interop.js.JSValue {}
        .function f64 std.interop.js.JSRuntime.getElementDouble(std.interop.js.JSValue a0, i32 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_GET_ELEMENT_DOUBLE,
                 "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.getElementDouble() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeGetUndefinedTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.interop.js.JSValue {}
        .function std.interop.js.JSValue std.interop.js.JSRuntime.getUndefined() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_GET_UNDEFINED, "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.getUndefined() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeGetNullTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.interop.js.JSValue {}
        .function std.interop.js.JSValue std.interop.js.JSRuntime.getNull() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_GET_NULL, "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.getNull() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeGetGlobalTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.interop.js.JSValue {}
        .function std.interop.js.JSValue std.interop.js.JSRuntime.getGlobal() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_GET_GLOBAL, "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.getGlobal() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeCreateObjectTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.interop.js.JSValue {}
        .function std.interop.js.JSValue std.interop.js.JSRuntime.createObject() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_CREATE_OBJECT, "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.createObject() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeCreateArrayTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.core.Object {}
        .function std.core.Object std.interop.js.JSRuntime.createArray() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_CREATE_ARRAY, "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.createArray() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeInstantiateTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.core.Object {}
        .function std.core.Object std.interop.js.JSRuntime.instantiate(std.core.Object a0, std.core.Object[] a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_INSTANTIATE, "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.instantiate() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeInstanceOfTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.core.Type {}
        .record std.interop.js.JSValue {}
        .function u1 std.interop.js.JSRuntime.instanceOfStaticType(std.interop.js.JSValue a0, std.core.Type a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_INSTANCE_OF, "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.instanceOfStaticType() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeInstanceOfDynamicTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.core.Object {}
        .function u1 std.interop.js.JSRuntime.instanceOfDynamic(std.core.Object a0, std.core.Object a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_INSTANCE_OF_DYNAMIC,
                 "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.instanceOfDynamic() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeInstanceOfStaticTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.core.Class {}
        .record std.interop.js.JSValue {}
        .function u1 std.interop.js.JSRuntime.instanceOfStatic(std.interop.js.JSValue a0, std.core.Class a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_INSTANCE_OF_STATIC,
                 "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.instanceOfStatic() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeInitJSCallClassTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .function u1 std.interop.js.JSRuntime.__initJSCallClass() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_INIT_JS_CALL_CLASS,
                 "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.__initJSCallClass() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeInitJSNewClassTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .function u1 std.interop.js.JSRuntime.__initJSNewClass() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_INIT_JS_NEW_CLASS,
                 "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.__initJSNewClass() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeLoadModuleTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.core.String {}
        .record std.interop.js.JSValue {}
        .function std.interop.js.JSValue std.interop.js.JSRuntime.loadModule(std.core.String a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_LOAD_MODULE, "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.loadModule() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeStrictEqualTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.interop.js.JSValue {}
        .function u1 std.interop.js.JSRuntime.strictEqual(std.interop.js.JSValue a0, std.interop.js.JSValue a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_STRICT_EQUAL, "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.strictEqual() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeSetPropertyTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.core.Object {}
        .function void std.interop.js.JSRuntime.setProperty(std.core.Object a0, std.core.Object a1, std.core.Object a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_SET_PROPERTY, "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function i32 std.interop.js.JSRuntime.setProperty() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeIsStaticValueTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.core.Object {}
        .function u1 std.interop.js.JSRuntime.isStaticValue(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_IS_STATIC_VALUE,
                 "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.isStaticValue() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeHasPropertyTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.core.String {}
        .record std.core.Object {}
        .function u1 std.interop.js.JSRuntime.hasProperty(std.core.Object a0, std.core.String a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_HAS_PROPERTY, "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.hasProperty() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeGetPropertyTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.core.Object {}
        .function std.core.Object std.interop.js.JSRuntime.getProperty(std.core.Object a0, std.core.Object a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_GET_PROPERTY, "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.getProperty() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeHasPropertyObjectTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.core.Object {}
        .function u1 std.interop.js.JSRuntime.hasPropertyObject(std.core.Object a0, std.core.Object a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_HAS_PROPERTY_OBJECT,
                 "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.hasPropertyObject() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeHasElementTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.core.Object {}
        .function u1 std.interop.js.JSRuntime.hasElement(std.core.Object a0, i32 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_HAS_ELEMENT, "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.hasElement() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeHasOwnPropertyTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.core.String {}
        .record std.core.Object {}
        .function u1 std.interop.js.JSRuntime.hasOwnProperty(std.core.Object a0, std.core.String a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_HAS_OWN_PROPERTY,
                 "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.hasOwnProperty() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeHasOwnPropertyObjectTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.core.Object {}
        .function u1 std.interop.js.JSRuntime.hasOwnPropertyJSValue(std.core.Object a0, std.core.Object a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_HAS_OWN_PROPERTY_OBJECT,
                 "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.hasOwnPropertyJSValue() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeTypeOfTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.core.String {}
        .record std.interop.js.JSValue {}
        .function std.core.String std.interop.js.JSRuntime.typeOf(std.interop.js.JSValue a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_TYPE_OF, "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.typeOf() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeIsPromiseTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.interop.js.JSValue {}
        .function u1 std.interop.js.JSRuntime.isPromise(std.interop.js.JSValue a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_IS_PROMISE, "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.isPromise() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeInvokeTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.core.Object {}
        .function std.core.Object std.interop.js.JSRuntime.invoke(std.core.Object a0, std.core.Object a1, std.core.Object[] a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_INVOKE, "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.invoke() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeXgcTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .function i64 std.interop.js.JSRuntime.xgc() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_XGC, "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.xgc() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, JSRuntimeInvokeDynamicFunctionTest)
{
    std::string source = R"(
        .record std.interop.js.JSRuntime {}
        .record std.core.Object {}
        .function std.core.Object std.interop.js.JSRuntime.invokeDynamicFunction(std.core.Object a0, std.core.Object[] a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_INVOKE_DYNAMIC_FUNCTION,
                 "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.invokeDynamicFunction() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");
}

TEST_F(RuntimeAdapterStaticTest, PromiseInteropResolveTest)
{
    std::string source = R"(
        .record std.interop.js.PromiseInterop {}
        .record std.core.Object {}
        .function void std.interop.js.PromiseInterop.resolve(std.core.Object a0, i64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_PROMISE_INTEROP_RESOLVE,
                 "std.interop.js.PromiseInterop");

    source = R"(
        .record std.interop.js.PromiseInterop {}
        .function i32 std.interop.js.PromiseInterop.resolve() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.PromiseInterop");
}

TEST_F(RuntimeAdapterStaticTest, PromiseInteropRejectTest)
{
    std::string source = R"(
        .record std.interop.js.PromiseInterop {}
        .record std.core.Object {}
        .function void std.interop.js.PromiseInterop.reject(std.core.Object a0, i64 a1) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_PROMISE_INTEROP_REJECT,
                 "std.interop.js.PromiseInterop");

    source = R"(
        .record std.interop.js.PromiseInterop {}
        .function i32 std.interop.js.PromiseInterop.reject() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.PromiseInterop");
}

TEST_F(RuntimeAdapterStaticTest, InteropTransferHelperTransferArrayBufferToStaticImplTest)
{
    std::string source = R"(
        .record std.interop.js.InteropTransferHelper {}
        .record std.core.ArrayBuffer {}
        .record std.interop.ESValue {}
        .function std.core.ArrayBuffer std.interop.js.InteropTransferHelper.transferArrayBufferToStaticImpl(std.interop.ESValue a0) <native>
    )";
    MainTestFunc(source,
                 RuntimeInterface::IntrinsicId::INTRINSIC_INTEROP_TRANSFER_HELPER_TRANSFER_ARRAY_BUFFER_TO_STATIC_IMPL,
                 "std.interop.js.InteropTransferHelper");

    source = R"(
        .record std.interop.js.InteropTransferHelper {}
        .function void std.interop.js.InteropTransferHelper.transferArrayBufferToStaticImpl() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.InteropTransferHelper");
}

TEST_F(RuntimeAdapterStaticTest, InteropTransferHelperTransferArrayBufferToDynamicImplTest)
{
    std::string source = R"(
        .record std.interop.js.InteropTransferHelper {}
        .record std.core.Object {}
        .record std.core.ArrayBuffer {}
        .function std.core.Object std.interop.js.InteropTransferHelper.transferArrayBufferToDynamicImpl(std.core.ArrayBuffer a0) <native>
    )";
    MainTestFunc(source,
                 RuntimeInterface::IntrinsicId::INTRINSIC_INTEROP_TRANSFER_HELPER_TRANSFER_ARRAY_BUFFER_TO_DYNAMIC_IMPL,
                 "std.interop.js.InteropTransferHelper");

    source = R"(
        .record std.interop.js.InteropTransferHelper {}
        .function void std.interop.js.InteropTransferHelper.transferArrayBufferToDynamicImpl() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.InteropTransferHelper");
}

TEST_F(RuntimeAdapterStaticTest, InteropTransferHelperCreateDynamicTypedArrayTest)
{
    std::string source = R"(
        .record std.interop.js.InteropTransferHelper {}
        .record std.core.Object {}
        .record std.core.ArrayBuffer {}
        .function std.core.Object std.interop.js.InteropTransferHelper.createDynamicTypedArray(std.core.ArrayBuffer a0, i32 a1, f64 a2, f64 a3) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_INTEROP_TRANSFER_HELPER_CREATE_DYNAMIC_TYPED_ARRAY,
                 "std.interop.js.InteropTransferHelper");

    source = R"(
        .record std.interop.js.InteropTransferHelper {}
        .function void std.interop.js.InteropTransferHelper.createDynamicTypedArray() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.InteropTransferHelper");
}

TEST_F(RuntimeAdapterStaticTest, InteropTransferHelperCreateDynamicDataViewTest)
{
    std::string source = R"(
        .record std.interop.js.InteropTransferHelper {}
        .record std.core.Object {}
        .record std.core.ArrayBuffer {}
        .function std.core.Object std.interop.js.InteropTransferHelper.createDynamicDataView(std.core.ArrayBuffer a0, f64 a1, f64 a2) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_INTEROP_TRANSFER_HELPER_CREATE_DYNAMIC_DATA_VIEW,
                 "std.interop.js.InteropTransferHelper");

    source = R"(
        .record std.interop.js.InteropTransferHelper {}
        .function void std.interop.js.InteropTransferHelper.createDynamicDataView() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.InteropTransferHelper");
}

TEST_F(RuntimeAdapterStaticTest, InteropContextSetInteropRuntimeLinkerImplTest)
{
    std::string source = R"(
        .record std.interop.InteropContext {}
        .record std.core.RuntimeLinker {}
        .function void std.interop.InteropContext.setInteropRuntimeLinker(std.core.RuntimeLinker a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_INTEROP_CONTEXT_SET_INTEROP_RUNTIME_LINKER_IMPL,
                 "std.interop.InteropContext");

    source = R"(
        .record std.interop.InteropContext {}
        .function i32 std.interop.InteropContext.setInteropRuntimeLinker() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.InteropContext");
}

TEST_F(RuntimeAdapterStaticTest, InteropSerializeHelperIsJSInteropRefImplTest)
{
    std::string source = R"(
        .record std.interop.InteropSerializeHelper {}
        .record std.core.Object {}
        .function u1 std.interop.InteropSerializeHelper.isJSInteropRef(std.core.Object a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_INTEROP_SERIALIZE_HELPER_IS_JS_INTEROP_REF_IMPL,
                 "std.interop.InteropSerializeHelper");

    source = R"(
        .record std.interop.InteropSerializeHelper {}
        .function void std.interop.InteropSerializeHelper.isJSInteropRef() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.InteropSerializeHelper");
}

TEST_F(RuntimeAdapterStaticTest, InteropSerializeHelperSerializeHandleImplTest)
{
    std::string source = R"(
        .record std.interop.InteropSerializeHelper {}
        .record std.interop.js.JSValue {}
        .function i64 std.interop.InteropSerializeHelper.serializeHandle(std.interop.js.JSValue a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_INTEROP_SERIALIZE_HELPER_SERIALIZE_HANDLE_IMPL,
                 "std.interop.InteropSerializeHelper");

    source = R"(
        .record std.interop.InteropSerializeHelper {}
        .function void std.interop.InteropSerializeHelper.serializeHandle() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.InteropSerializeHelper");
}

TEST_F(RuntimeAdapterStaticTest, InteropSerializeHelperDeserializeHandleImplTest)
{
    std::string source = R"(
        .record std.interop.InteropSerializeHelper {}
        .record std.interop.js.JSValue {}
        .function std.interop.js.JSValue std.interop.InteropSerializeHelper.deserializeHandle(i64 a0) <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INTRINSIC_INTEROP_SERIALIZE_HELPER_DESERIALIZE_HANDLE_IMPL,
                 "std.interop.InteropSerializeHelper");

    source = R"(
        .record std.interop.InteropSerializeHelper {}
        .function void std.interop.InteropSerializeHelper.deserializeHandle() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.InteropSerializeHelper");
}

TEST_F(RuntimeAdapterStaticTest, InvalidTest0)
{
    std::string source = R"(
        .record Math {}
        .function void Math.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Math");

    source = R"(
        .record Double {}
        .function void Double.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Double");

    source = R"(
        .record Float {}
        .function void Float.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Float");

    source = R"(
        .record IO {}
        .function void IO.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "IO");

    source = R"(
        .record System {}
        .function void System.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "System");

    source = R"(
        .record DebugUtils {}
        .function void DebugUtils.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "DebugUtils");

    source = R"(
        .record Convert {}
        .function void Convert.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Convert");
}

TEST_F(RuntimeAdapterStaticTest, InvalidTest1)
{
    std::string source = R"(
        .record Object {}
        .function void Object.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "Object");

    source = R"(
        .record std.math.ETSGLOBAL {}
        .function void std.math.ETSGLOBAL.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.math.ETSGLOBAL");

    source = R"(
        .record std.core.ETSGLOBAL {}
        .function void std.core.ETSGLOBAL.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ETSGLOBAL");

    source = R"(
        .record escompat.JSON {}
        .function void escompat.JSON.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "escompat.JSON");

    source = R"(
        .record std.core.String {}
        .function void std.core.String.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.String");

    source = R"(
        .record std.core.StringBuilder {}
        .function void std.core.StringBuilder.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.StringBuilder");

    source = R"(
        .record std.core.Math {}
        .function void std.core.Math.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Math");
}

TEST_F(RuntimeAdapterStaticTest, InvalidTest2)
{
    std::string source = R"(
        .record escompat.JSONAPI {}
        .function void escompat.JSONAPI.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "escompat.JSONAPI");

    source = R"(
        .record escompat.Date {}
        .function void escompat.Date.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "escompat.Date");

    source = R"(
        .record escompat.Array {}
        .function void escompat.Array.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "escompat.Array");

    source = R"(
        .record std.core.ArrayBuffer {}
        .function void std.core.ArrayBuffer.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ArrayBuffer");

    source = R"(
        .record escompat.Uint8ClampedArray {}
        .function void escompat.Uint8ClampedArray.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "escompat.Uint8ClampedArray");

    source = R"(
        .record std.concurrency.taskpool.Task {}
        .function void std.concurrency.taskpool.Task.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.concurrency.taskpool.Task");

    source = R"(
        .record std.concurrency.taskpool.GroupState {}
        .function void std.concurrency.taskpool.GroupState.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.concurrency.taskpool.GroupState");
}

TEST_F(RuntimeAdapterStaticTest, InvalidTest3)
{
    std::string source = R"(
        .record std.concurrency.taskpool {}
        .function void std.concurrency.taskpool.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.concurrency.taskpool");

    source = R"(
        .record std.concurrency.taskpool.SequenceRunner {}
        .function void std.concurrency.taskpool.SequenceRunner.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.concurrency.taskpool.SequenceRunner");

    source = R"(
        .record std.concurrency.taskpool.AsyncRunner {}
        .function void std.concurrency.taskpool.AsyncRunner.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.concurrency.taskpool.AsyncRunner");

    source = R"(
        .record std.time.Chrono {}
        .function void std.time.Chrono.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.time.Chrono");

    source = R"(
        .record std.core.Float {}
        .function void std.core.Float.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Float");

    source = R"(
        .record std.core.Double {}
        .function void std.core.Double.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Double");

    source = R"(
        .record escompat.Int32Array {}
        .function void escompat.Int32Array.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "escompat.Int32Array");
}

TEST_F(RuntimeAdapterStaticTest, InvalidTest4)
{
    std::string source = R"(
        .record escompat.Int16Array {}
        .function void escompat.Int16Array.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "escompat.Int16Array");

    source = R"(
        .record escompat.Int8Array {}
        .function void escompat.Int8Array.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "escompat.Int8Array");

    source = R"(
        .record escompat.Uint32Array {}
        .function void escompat.Uint32Array.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "escompat.Uint32Array");

    source = R"(
        .record escompat.Uint16Array {}
        .function void escompat.Uint16Array.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "escompat.Uint16Array");

    source = R"(
        .record escompat.Uint8Array {}
        .function void escompat.Uint8Array.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "escompat.Uint8Array");

    source = R"(
        .record std.core.unsafeMemory {}
        .function void std.core.unsafeMemory.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.unsafeMemory");

    source = R"(
        .record escompat.ETSGLOBAL {}
        .function void escompat.ETSGLOBAL.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "escompat.ETSGLOBAL");
}

TEST_F(RuntimeAdapterStaticTest, InvalidTest5)
{
    std::string source = R"(
        .record std.core.StackTrace {}
        .function void std.core.StackTrace.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.StackTrace");

    source = R"(
        .record std.core.GC {}
        .function void std.core.GC.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.GC");

    source = R"(
        .record std.core.FinalizationRegistry {}
        .function void std.core.FinalizationRegistry.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.FinalizationRegistry");

    source = R"(
        .record std.core.Coroutine {}
        .function void std.core.Coroutine.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Coroutine");

    source = R"(
        .record std.core.Mutex {}
        .function void std.core.Mutex.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Mutex");

    source = R"(
        .record std.core.Event {}
        .function void std.core.Event.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Event");

    source = R"(
        .record std.core.CondVar {}
        .function void std.core.CondVar.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.CondVar");
}

TEST_F(RuntimeAdapterStaticTest, InvalidTest6)
{
    std::string source = R"(
        .record std.core.QueueSpinlock {}
        .function void std.core.QueueSpinlock.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.QueueSpinlock");

    source = R"(
        .record std.core.ReadLock {}
        .function void std.core.ReadLock.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ReadLock");

    source = R"(
        .record std.core.WriteLock {}
        .function void std.core.WriteLock.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.WriteLock");

    source = R"(
        .record std.debug.concurrency.CoroutineExtras {}
        .function void std.debug.concurrency.CoroutineExtras.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.debug.concurrency.CoroutineExtras");

    source = R"(
        .record std.core.Runtime {}
        .function void std.core.Runtime.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Runtime");

    source = R"(
        .record std.core.reflect.Method {}
        .function void std.core.reflect.Method.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.reflect.Method");

    source = R"(
        .record std.core.reflect.Field {}
        .function void std.core.reflect.Field.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.reflect.Field");
}

TEST_F(RuntimeAdapterStaticTest, InvalidTest7)
{
    std::string source = R"(
        .record std.core.Class {}
        .function void std.core.Class.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Class");

    source = R"(
        .record std.core.AbcFile {}
        .function void std.core.AbcFile.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.AbcFile");

    source = R"(
        .record std.core.RegExp {}
        .function void std.core.RegExp.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.RegExp");

    source = R"(
        .record std.core.TypeAPI {}
        .function void std.core.TypeAPI.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeAPI");

    source = R"(
        .record std.core.Char {}
        .function void std.core.Char.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Char");

    source = R"(
        .record std.core.TypeCreatorCtx {}
        .function void std.core.TypeCreatorCtx.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.TypeCreatorCtx");

    source = R"(
        .record std.containers.ConcurrencyHelpers {}
        .function void std.containers.ConcurrencyHelpers.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.containers.ConcurrencyHelpers");
}

TEST_F(RuntimeAdapterStaticTest, InvalidTest8)
{
    std::string source = R"(
        .record std.debug.RuntimeDebug {}
        .function void std.debug.RuntimeDebug.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.debug.RuntimeDebug");

    source = R"(
        .record std.debug.DebuggerAPI {}
        .function void std.debug.DebuggerAPI.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.debug.DebuggerAPI");

    source = R"(
        .record escompat.ConcurrencyHelpers {}
        .function void escompat.ConcurrencyHelpers.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "escompat.ConcurrencyHelpers");

    source = R"(
        .record std.core.Reflect {}
        .function void std.core.Reflect.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Reflect");

    source = R"(
        .record std.concurrency.ConcurrencyHelpers {}
        .function void std.concurrency.ConcurrencyHelpers.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.concurrency.ConcurrencyHelpers");

    source = R"(
        .record std.concurrency.NativeAsyncWorkHelper {}
        .function void std.concurrency.NativeAsyncWorkHelper.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.concurrency.NativeAsyncWorkHelper");

    source = R"(
        .record std.concurrency.WorkerGroup {}
        .function void std.concurrency.WorkerGroup.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.concurrency.WorkerGroup");
}

TEST_F(RuntimeAdapterStaticTest, InvalidTest9)
{
    std::string source = R"(
        .record std.core.ConcurrencyHelpers {}
        .function void std.core.ConcurrencyHelpers.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ConcurrencyHelpers");

    source = R"(
        .record std.core.ExclusiveLauncher {}
        .function void std.core.ExclusiveLauncher.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.ExclusiveLauncher");

    source = R"(
        .record std.core.InternalWorker {}
        .function void std.core.InternalWorker.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.InternalWorker");

    source = R"(
        .record std.concurrency.ETSGLOBAL {}
        .function void std.concurrency.ETSGLOBAL.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.concurrency.ETSGLOBAL");

    source = R"(
        .record std.core.Byte {}
        .function void std.core.Byte.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Byte");

    source = R"(
        .record std.core.Short {}
        .function void std.core.Short.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Short");

    source = R"(
        .record std.core.Int {}
        .function void std.core.Int.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Int");
}

TEST_F(RuntimeAdapterStaticTest, InvalidTest10)
{
    std::string source = R"(
        .record std.core.Long {}
        .function void std.core.Long.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.Long");

    source = R"(
        .record std.core.reflect.Proxy {}
        .function void std.core.reflect.Proxy.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.core.reflect.Proxy");

    source = R"(
        .record std.interop.js.JSRuntime {}
        .function void std.interop.js.JSRuntime.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.JSRuntime");

    source = R"(
        .record std.interop.js.PromiseInterop {}
        .function void std.interop.js.PromiseInterop.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.PromiseInterop");

    source = R"(
        .record std.interop.js.InteropTransferHelper {}
        .function void std.interop.js.InteropTransferHelper.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.js.InteropTransferHelper");

    source = R"(
        .record std.interop.InteropContext {}
        .function void std.interop.InteropContext.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.InteropContext");

    source = R"(
        .record std.interop.InteropSerializeHelper {}
        .function void std.interop.InteropSerializeHelper.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "std.interop.InteropSerializeHelper");

    source = R"(
        .record test {}
        .function void test.test() <native>
    )";
    MainTestFunc(source, RuntimeInterface::IntrinsicId::INVALID, "test");
}

}  // namespace libabckit::test::adapter_static

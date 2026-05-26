/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"
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

#include <array>

#include "ani.h"
#include "ani_gtest.h"

namespace ark::ets::ani::testing {

class ArrayWrongTypeArgs : public AniTest {};

TEST_F(ArrayWrongTypeArgs, getBufferWrongType)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("std.core.Array", &cls), ANI_OK);

    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "getBuffer", "C{std.core.Object}:A{Y}", &method), ANI_OK);

    // FixedArray<Object> is not std.core.Array -> TypeError
    ani_class objectCls;
    ASSERT_EQ(env_->FindClass("std.core.Object", &objectCls), ANI_OK);
    ani_ref undefinedRef;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);
    ani_fixedarray_ref wrongObj;
    ASSERT_EQ(env_->FixedArray_New_Ref(objectCls, 0U, undefinedRef, &wrongObj), ANI_OK);

    ani_ref result;
    std::array<ani_value, 1U> args = {};
    args[0U].r = wrongObj;  // obj C{std.core.Object} as std.core.Array - wrong type! FixedArray != std.core.Array
    ASSERT_EQ(env_->Class_CallStaticMethod_Ref_A(cls, method, &result, args.data()), ANI_PENDING_ERROR);
}

TEST_F(ArrayWrongTypeArgs, fillImplBufferWrongType)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("std.core.Array", &cls), ANI_OK);

    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "fillImpl", "A{Y}iYii:", &method), ANI_OK);

    ani_ref undefinedRef;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);

    ani_array wrongBuffer;
    ASSERT_EQ(env_->Array_New(0U, undefinedRef, &wrongBuffer), ANI_OK);

    const ani_int length = 5U;
    const ani_int start = 0U;
    const ani_int end = length;

    std::array<ani_value, 5U> args = {};
    args[0U].r = wrongBuffer;  // buffer A{Y} - wrong type! std.core.Array != FixedArray
    args[1U].i = length;
    args[2U].r = undefinedRef;
    args[3U].i = start;
    args[4U].i = end;
    ASSERT_EQ(env_->Class_CallStaticMethod_Void_A(cls, method, args.data()), ANI_PENDING_ERROR);
}

TEST_F(ArrayWrongTypeArgs, indexOfImplBufferWrongType)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("std.core.Array", &cls), ANI_OK);

    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "indexOfImpl", "A{Y}iYi:i", &method), ANI_OK);

    ani_ref undefinedRef;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);

    ani_array wrongBuffer;
    ASSERT_EQ(env_->Array_New(0U, undefinedRef, &wrongBuffer), ANI_OK);

    const ani_int length = 5U;
    const ani_int fromIndex = 0U;

    std::array<ani_value, 4U> args = {};
    args[0U].r = wrongBuffer;  // buffer A{Y} - wrong type! std.core.Array != FixedArray
    args[1U].i = length;
    args[2U].r = undefinedRef;
    args[3U].i = fromIndex;

    ani_int result;
    ASSERT_EQ(env_->Class_CallStaticMethod_Int_A(cls, method, &result, args.data()), ANI_PENDING_ERROR);
}

TEST_F(ArrayWrongTypeArgs, lastIndexOfImplBufferWrongType)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("std.core.Array", &cls), ANI_OK);

    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "lastIndexOfImpl", "A{Y}iYi:i", &method), ANI_OK);

    ani_ref undefinedRef;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);

    ani_array wrongBuffer;
    ASSERT_EQ(env_->Array_New(0U, undefinedRef, &wrongBuffer), ANI_OK);

    const ani_int length = 5U;
    const ani_int fromIndex = 0U;

    std::array<ani_value, 4U> args = {};
    args[0U].r = wrongBuffer;  // buffer A{Y} - wrong type! std.core.Array != FixedArray
    args[1U].i = length;
    args[2U].r = undefinedRef;
    args[3U].i = fromIndex;

    ani_int result;
    ASSERT_EQ(env_->Class_CallStaticMethod_Int_A(cls, method, &result, args.data()), ANI_PENDING_ERROR);
}

TEST_F(ArrayWrongTypeArgs, unshiftInternalArrayHeaderWrongType)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("std.core.Array", &cls), ANI_OK);

    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "unshiftInternal", "A{Y}iA{Y}C{std.core.Array}:", &method), ANI_OK);

    ani_ref undefinedRef;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);
    ani_int selfLen = 1U;

    ani_class objectCls;
    ASSERT_EQ(env_->FindClass("std.core.Object", &objectCls), ANI_OK);

    // Create a FixedArray to use as buffer header (valid type for A{Y})
    const ani_size valuesLen = 2U;
    ani_fixedarray_ref buffer;
    ASSERT_EQ(env_->FixedArray_New_Ref(objectCls, valuesLen + selfLen, undefinedRef, &buffer), ANI_OK);

    ani_array valuesArray {};
    ASSERT_EQ(env_->Array_New(valuesLen, undefinedRef, &valuesArray), ANI_OK);

    std::array<ani_value, 4U> args = {};
    args[0U].r = valuesArray;  // arrayHeader A{Y} - wrong type! std.core.Array != FixedArray
    args[1U].i = selfLen;
    args[2U].r = buffer;       // bufferHeader A{Y} - valid FixedArray
    args[3U].r = valuesArray;  // values C{std.core.Array} - valid std.core.Array
    ASSERT_EQ(env_->Class_CallStaticMethod_Void_A(cls, method, args.data()), ANI_PENDING_ERROR);
}

TEST_F(ArrayWrongTypeArgs, unshiftInternalBufferHeaderWrongType)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("std.core.Array", &cls), ANI_OK);

    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "unshiftInternal", "A{Y}iA{Y}C{std.core.Array}:", &method), ANI_OK);

    ani_ref undefinedRef;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);
    ani_int selfLen = 1U;

    ani_class objectCls;
    ASSERT_EQ(env_->FindClass("std.core.Object", &objectCls), ANI_OK);

    const ani_size valuesLen = 2U;
    ani_fixedarray_ref buffer;
    ASSERT_EQ(env_->FixedArray_New_Ref(objectCls, valuesLen + selfLen, undefinedRef, &buffer), ANI_OK);

    ani_array valuesArray {};
    ASSERT_EQ(env_->Array_New(valuesLen, undefinedRef, &valuesArray), ANI_OK);

    std::array<ani_value, 4U> args = {};
    args[0U].r = buffer;  // arrayHeader A{Y} - valid FixedArray
    args[1U].i = selfLen;
    args[2U].r = valuesArray;  // bufferHeader A{Y} - wrong type! std.core.Array != FixedArray
    args[3U].r = valuesArray;  // values C{std.core.Array} - valid std.core.Array
    ASSERT_EQ(env_->Class_CallStaticMethod_Void_A(cls, method, args.data()), ANI_PENDING_ERROR);
}

TEST_F(ArrayWrongTypeArgs, unshiftInternalValuesWrongType)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("std.core.Array", &cls), ANI_OK);

    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "unshiftInternal", "A{Y}iA{Y}C{std.core.Array}:", &method), ANI_OK);

    ani_ref undefinedRef;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);
    ani_int selfLen = 1U;

    ani_class objectCls;
    ASSERT_EQ(env_->FindClass("std.core.Object", &objectCls), ANI_OK);

    // Create a FixedArray to use as buffer header (valid type for A{Y})
    const ani_size valuesLen = 2U;
    ani_fixedarray_ref buffer;
    ASSERT_EQ(env_->FixedArray_New_Ref(objectCls, valuesLen + selfLen, undefinedRef, &buffer), ANI_OK);

    std::array<ani_value, 4U> args = {};
    args[0U].r = buffer;  // arrayHeader A{Y} - valid FixedArray
    args[1U].i = selfLen;
    args[2U].r = buffer;  // bufferHeader A{Y} - valid FixedArray
    args[3U].r = buffer;  // values (C{std.core.Array}) - wrong type! FixedArray != std.core.Array
    ASSERT_EQ(env_->Class_CallStaticMethod_Void_A(cls, method, args.data()), ANI_PENDING_ERROR);
}

TEST_F(ArrayWrongTypeArgs, copyToFastSourceWrongType)
{
    ani_module module;
    env_->FindModule("std.core", &module);

    ani_function function;
    ASSERT_EQ(env_->Module_FindFunction(module, "copyToFast", "A{Y}A{Y}iii:", &function), ANI_OK);

    ani_ref undefinedRef;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);

    ani_class objectCls;
    ASSERT_EQ(env_->FindClass("std.core.Object", &objectCls), ANI_OK);

    ani_fixedarray_ref destBuffer;
    ASSERT_EQ(env_->FixedArray_New_Ref(objectCls, 5U, undefinedRef, &destBuffer), ANI_OK);

    ani_array srcArray;
    ASSERT_EQ(env_->Array_New(5U, undefinedRef, &srcArray), ANI_OK);

    const ani_int start = 0U;
    const ani_int length = 3U;
    const ani_int extra = 0U;

    std::array<ani_value, 5U> args = {};
    args[0U].r = srcArray;    // src A{Y} - wrong type! std.core.Array != FixedArray
    args[1U].r = destBuffer;  // dst A{Y} - valid FixedArray
    args[2U].i = start;
    args[3U].i = length;
    args[4U].i = extra;
    ASSERT_EQ(env_->Function_Call_Void_A(function, args.data()), ANI_PENDING_ERROR);
}

TEST_F(ArrayWrongTypeArgs, copyToFastDestWrongType)
{
    ani_module module;
    env_->FindModule("std.core", &module);

    ani_function function;
    ASSERT_EQ(env_->Module_FindFunction(module, "copyToFast", "A{Y}A{Y}iii:", &function), ANI_OK);

    ani_ref undefinedRef;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);

    ani_class objectCls;
    ASSERT_EQ(env_->FindClass("std.core.Object", &objectCls), ANI_OK);

    ani_fixedarray_ref srcBuffer;
    ASSERT_EQ(env_->FixedArray_New_Ref(objectCls, 5U, undefinedRef, &srcBuffer), ANI_OK);

    ani_array destArray;
    ASSERT_EQ(env_->Array_New(5U, undefinedRef, &destArray), ANI_OK);

    const ani_int start = 0U;
    const ani_int length = 3U;
    const ani_int extra = 0U;

    std::array<ani_value, 5U> args = {};
    args[0U].r = srcBuffer;  // src A{Y} - valid FixedArray
    args[1U].r = destArray;  // dst A{Y} - wrong type! std.core.Array != FixedArray
    args[2U].i = start;
    args[3U].i = length;
    args[4U].i = extra;
    ASSERT_EQ(env_->Function_Call_Void_A(function, args.data()), ANI_PENDING_ERROR);
}

TEST_F(ArrayWrongTypeArgs, reverseImplBufferWrongType)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("std.core.Array", &cls), ANI_OK);

    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "reverseImpl", "A{Y}i:", &method), ANI_OK);

    ani_ref undefinedRef;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);

    // Create a std.core.Array (wrong type for A{Y})
    ani_array wrongBuffer;
    ASSERT_EQ(env_->Array_New(5U, undefinedRef, &wrongBuffer), ANI_OK);

    const ani_int length = 5U;

    std::array<ani_value, 2U> args = {};
    args[0U].r = wrongBuffer;  // buffer A{Y} - wrong type! std.core.Array != FixedArray
    args[1U].i = length;
    ASSERT_EQ(env_->Class_CallStaticMethod_Void_A(cls, method, args.data()), ANI_PENDING_ERROR);
}

TEST_F(ArrayWrongTypeArgs, indexOfImplPrimitiveArrayTypeConfusion)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("std.core.Array", &cls), ANI_OK);

    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "indexOfImpl", "A{Y}iYi:i", &method), ANI_OK);

    ani_ref undefinedRef;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);

    ani_fixedarray_byte primitiveBuf;
    ASSERT_EQ(env_->FixedArray_New_Byte(64U, &primitiveBuf), ANI_OK);

    const ani_int length = 1000U;

    std::array<ani_value, 4U> args = {};
    args[0U].r = primitiveBuf;  // FixedArray<byte> — not a FixedArray<Any> per spec invariance
    args[1U].i = length;        // attacker-controlled length, larger than buffer
    args[2U].r = undefinedRef;
    args[3U].i = 0U;

    ani_int result;
    ASSERT_EQ(env_->Class_CallStaticMethod_Int_A(cls, method, &result, args.data()), ANI_PENDING_ERROR);
}

TEST_F(ArrayWrongTypeArgs, fillImplPrimitiveArrayTypeConfusion)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("std.core.Array", &cls), ANI_OK);

    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "fillImpl", "A{Y}iYii:", &method), ANI_OK);

    ani_ref undefinedRef;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);

    ani_fixedarray_byte primitiveBuf;
    ASSERT_EQ(env_->FixedArray_New_Byte(64U, &primitiveBuf), ANI_OK);

    const ani_int length = 1000U;
    const ani_int start = 0U;
    const ani_int end = length;

    std::array<ani_value, 5U> args = {};
    args[0U].r = primitiveBuf;  // FixedArray<byte> — not a FixedArray<Any>
    args[1U].i = length;
    args[2U].r = undefinedRef;
    args[3U].i = start;
    args[4U].i = end;

    ASSERT_EQ(env_->Class_CallStaticMethod_Void_A(cls, method, args.data()), ANI_PENDING_ERROR);
}

TEST_F(ArrayWrongTypeArgs, reverseImplPrimitiveArrayTypeConfusion)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("std.core.Array", &cls), ANI_OK);

    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "reverseImpl", "A{Y}i:", &method), ANI_OK);

    ani_fixedarray_byte primitiveBuf;
    ASSERT_EQ(env_->FixedArray_New_Byte(64U, &primitiveBuf), ANI_OK);

    const ani_int length = 1000U;

    std::array<ani_value, 2U> args = {};
    args[0U].r = primitiveBuf;  // FixedArray<byte> — not a FixedArray<Any>
    args[1U].i = length;

    ASSERT_EQ(env_->Class_CallStaticMethod_Void_A(cls, method, args.data()), ANI_PENDING_ERROR);
}

TEST_F(ArrayWrongTypeArgs, unshiftInternalPrimitiveArrayTypeConfusion)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("std.core.Array", &cls), ANI_OK);

    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "unshiftInternal", "A{Y}iA{Y}C{std.core.Array}:", &method), ANI_OK);

    ani_ref undefinedRef;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);

    // Build valid values array
    ani_array valuesArray;
    ASSERT_EQ(env_->Array_New(2U, undefinedRef, &valuesArray), ANI_OK);

    // Primitive buffer for arrayHeader A{Y}
    ani_fixedarray_byte primitiveArrayHeader;
    ASSERT_EQ(env_->FixedArray_New_Byte(8U, &primitiveArrayHeader), ANI_OK);

    // Primitive buffer for bufferHeader A{Y}
    ani_fixedarray_byte primitiveBufferHeader;
    ASSERT_EQ(env_->FixedArray_New_Byte(16U, &primitiveBufferHeader), ANI_OK);

    const ani_int selfLen = 1U;

    std::array<ani_value, 4U> args = {};
    args[0U].r = primitiveArrayHeader;  // arrayHeader A{Y} — primitive array!
    args[1U].i = selfLen;
    args[2U].r = primitiveBufferHeader;  // bufferHeader A{Y} — primitive array!
    args[3U].r = valuesArray;

    ASSERT_EQ(env_->Class_CallStaticMethod_Void_A(cls, method, args.data()), ANI_PENDING_ERROR);
}

TEST_F(ArrayWrongTypeArgs, copyToFastPrimitiveArrayTypeConfusion)
{
    ani_module module;
    env_->FindModule("std.core", &module);

    ani_function function;
    ASSERT_EQ(env_->Module_FindFunction(module, "copyToFast", "A{Y}A{Y}iii:", &function), ANI_OK);

    ani_fixedarray_byte srcPrimitive;
    ASSERT_EQ(env_->FixedArray_New_Byte(64U, &srcPrimitive), ANI_OK);

    ani_fixedarray_byte dstPrimitive;
    ASSERT_EQ(env_->FixedArray_New_Byte(64U, &dstPrimitive), ANI_OK);

    const ani_int dstStart = 0U;
    const ani_int srcStart = 0U;
    const ani_int srcEnd = 10U;

    std::array<ani_value, 5U> args = {};
    args[0U].r = srcPrimitive;  // src A{Y} — FixedArray<byte>, not FixedArray<Any>
    args[1U].r = dstPrimitive;  // dst A{Y} — FixedArray<byte>, not FixedArray<Any>
    args[2U].i = dstStart;
    args[3U].i = srcStart;
    args[4U].i = srcEnd;

    ASSERT_EQ(env_->Function_Call_Void_A(function, args.data()), ANI_PENDING_ERROR);
}

TEST_F(ArrayWrongTypeArgs, copyToFastWithBarriersPrimitiveArrayTypeConfusion)
{
    ani_module module;
    env_->FindModule("std.core", &module);

    ani_function function;
    ASSERT_EQ(env_->Module_FindFunction(module, "copyToFastWithBarriers", "A{Y}A{Y}iii:", &function), ANI_OK);

    ani_fixedarray_byte srcPrimitive;
    ASSERT_EQ(env_->FixedArray_New_Byte(64U, &srcPrimitive), ANI_OK);

    ani_fixedarray_byte dstPrimitive;
    ASSERT_EQ(env_->FixedArray_New_Byte(64U, &dstPrimitive), ANI_OK);

    const ani_int dstStart = 0U;
    const ani_int srcStart = 0U;
    const ani_int srcEnd = 10U;

    std::array<ani_value, 5U> args = {};
    args[0U].r = srcPrimitive;  // src A{Y} — FixedArray<byte>, not FixedArray<Any>
    args[1U].r = dstPrimitive;  // dst A{Y} — FixedArray<byte>, not FixedArray<Any>
    args[2U].i = dstStart;
    args[3U].i = srcStart;
    args[4U].i = srcEnd;

    ASSERT_EQ(env_->Function_Call_Void_A(function, args.data()), ANI_PENDING_ERROR);
}

}  // namespace ark::ets::ani::testing
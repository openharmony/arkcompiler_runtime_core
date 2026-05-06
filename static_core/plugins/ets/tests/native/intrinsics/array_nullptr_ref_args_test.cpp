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

class ArrayNullptrRefArgs : public AniTest {};

TEST_F(ArrayNullptrRefArgs, isPlatformArrayNullPtrObj)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("std.core.Array", &cls), ANI_OK);

    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "isPlatformArray", "C{std.core.Object}:z", &method), ANI_OK);

    ani_ref undefinedRef;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);

    ani_boolean result;
    std::array<ani_value, 1U> args = {};
    args[0U].r = undefinedRef;
    ASSERT_EQ(env_->Class_CallStaticMethod_Boolean_A(cls, method, &result, args.data()), ANI_PENDING_ERROR);
}

TEST_F(ArrayNullptrRefArgs, unshiftInternalArrayHeaderNullptr)
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
    for (ani_size i = 0; i < valuesLen; ++i) {
        ASSERT_EQ(env_->Array_Set(valuesArray, i, undefinedRef), ANI_OK);
    }

    std::array<ani_value, 4U> args = {};
    args[0U].r = undefinedRef;
    args[1U].i = selfLen;
    args[2U].r = buffer;
    args[3U].r = valuesArray;
    ASSERT_EQ(env_->Class_CallStaticMethod_Void_A(cls, method, args.data()), ANI_PENDING_ERROR);
}

TEST_F(ArrayNullptrRefArgs, unshiftInternalBufferHeaderNullptr)
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

    ani_fixedarray_ref sourceBuffer;
    ASSERT_EQ(env_->FixedArray_New_Ref(objectCls, selfLen, undefinedRef, &sourceBuffer), ANI_OK);

    const ani_size valuesLen = 2U;

    ani_array valuesArray {};
    ASSERT_EQ(env_->Array_New(valuesLen, undefinedRef, &valuesArray), ANI_OK);
    for (ani_size i = 0; i < valuesLen; ++i) {
        ASSERT_EQ(env_->Array_Set(valuesArray, i, undefinedRef), ANI_OK);
    }

    std::array<ani_value, 4U> args = {};
    args[0U].r = sourceBuffer;
    args[1U].i = selfLen;
    args[2U].r = undefinedRef;
    args[3U].r = valuesArray;
    ASSERT_EQ(env_->Class_CallStaticMethod_Void_A(cls, method, args.data()), ANI_PENDING_ERROR);
}

TEST_F(ArrayNullptrRefArgs, unshiftInternalValuesNullptr)
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

    ani_fixedarray_ref sourceBuffer;
    ASSERT_EQ(env_->FixedArray_New_Ref(objectCls, selfLen, undefinedRef, &sourceBuffer), ANI_OK);

    const ani_size valuesLen = 2U;

    ani_fixedarray_ref buffer;
    ASSERT_EQ(env_->FixedArray_New_Ref(objectCls, valuesLen + selfLen, undefinedRef, &buffer), ANI_OK);

    ani_array valuesArray {};
    ASSERT_EQ(env_->Array_New(valuesLen, undefinedRef, &valuesArray), ANI_OK);
    for (ani_size i = 0; i < valuesLen; ++i) {
        ASSERT_EQ(env_->Array_Set(valuesArray, i, undefinedRef), ANI_OK);
    }

    std::array<ani_value, 4U> args = {};
    args[0U].r = sourceBuffer;
    args[1U].i = selfLen;
    args[2U].r = buffer;
    args[3U].r = undefinedRef;
    ASSERT_EQ(env_->Class_CallStaticMethod_Void_A(cls, method, args.data()), ANI_PENDING_ERROR);
}

TEST_F(ArrayNullptrRefArgs, getBufferNullptr)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("std.core.Array", &cls), ANI_OK);

    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "getBuffer", "C{std.core.Object}:A{Y}", &method), ANI_OK);

    ani_ref undefinedRef;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);

    ani_ref result;
    std::array<ani_value, 1U> args = {};
    args[0U].r = undefinedRef;
    ASSERT_EQ(env_->Class_CallStaticMethod_Ref_A(cls, method, &result, args.data()), ANI_PENDING_ERROR);
}

TEST_F(ArrayNullptrRefArgs, fillImplBufferNullptr)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("std.core.Array", &cls), ANI_OK);

    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "fillImpl", "A{Y}iYii:", &method), ANI_OK);

    ani_ref undefinedRef;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);

    ani_array dummyArray {};
    ASSERT_EQ(env_->Array_New(0U, undefinedRef, &dummyArray), ANI_OK);

    const ani_int length = 5U;
    const ani_int start = 0U;
    const ani_int end = length;

    std::array<ani_value, 5U> args = {};
    args[0U].r = undefinedRef;
    args[1U].i = length;
    args[2U].r = dummyArray;
    args[3U].i = start;
    args[4U].i = end;
    ASSERT_EQ(env_->Class_CallStaticMethod_Void_A(cls, method, args.data()), ANI_PENDING_ERROR);
}

TEST_F(ArrayNullptrRefArgs, reverseImplBufferNullptr)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("std.core.Array", &cls), ANI_OK);

    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "reverseImpl", "A{Y}i:", &method), ANI_OK);

    ani_ref undefinedRef;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);

    const ani_int length = 5U;

    std::array<ani_value, 2U> args = {};
    args[0U].r = undefinedRef;
    args[1U].i = length;
    ASSERT_EQ(env_->Class_CallStaticMethod_Void_A(cls, method, args.data()), ANI_PENDING_ERROR);
}

TEST_F(ArrayNullptrRefArgs, indexOfImplBufferNullptr)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("std.core.Array", &cls), ANI_OK);

    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "indexOfImpl", "A{Y}iYi:i", &method), ANI_OK);

    ani_ref undefinedRef;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);

    const ani_int length = 5U;
    const ani_int fromIndex = 0U;

    ani_array dummyValue {};
    ASSERT_EQ(env_->Array_New(0U, undefinedRef, &dummyValue), ANI_OK);

    std::array<ani_value, 4U> args = {};
    args[0U].r = undefinedRef;
    args[1U].i = length;
    args[2U].r = dummyValue;
    args[3U].i = fromIndex;

    ani_int result;
    ASSERT_EQ(env_->Class_CallStaticMethod_Int_A(cls, method, &result, args.data()), ANI_PENDING_ERROR);
}

TEST_F(ArrayNullptrRefArgs, lastIndexOfImplBufferNullptr)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("std.core.Array", &cls), ANI_OK);

    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "lastIndexOfImpl", "A{Y}iYi:i", &method), ANI_OK);

    ani_ref undefinedRef;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);

    const ani_int length = 5U;
    const ani_int fromIndex = 0U;

    ani_array dummyValue {};
    ASSERT_EQ(env_->Array_New(0U, undefinedRef, &dummyValue), ANI_OK);

    std::array<ani_value, 4U> args = {};
    args[0U].r = undefinedRef;
    args[1U].i = length;
    args[2U].r = dummyValue;
    args[3U].i = fromIndex;

    ani_int result;
    ASSERT_EQ(env_->Class_CallStaticMethod_Int_A(cls, method, &result, args.data()), ANI_PENDING_ERROR);
}

TEST_F(ArrayNullptrRefArgs, copyToFastSourceNullptr)
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

    const ani_int start = 0U;
    const ani_int length = 3U;
    const ani_int extra = 0U;

    std::array<ani_value, 5U> args = {};
    args[0U].r = undefinedRef;
    args[1U].r = destBuffer;
    args[2U].i = start;
    args[3U].i = length;
    args[4U].i = extra;
    ASSERT_EQ(env_->Function_Call_Void_A(function, args.data()), ANI_PENDING_ERROR);
}

TEST_F(ArrayNullptrRefArgs, copyToFastDestNullptr)
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

    const ani_int start = 0U;
    const ani_int length = 3U;
    const ani_int extra = 0U;

    std::array<ani_value, 5U> args = {};
    args[0U].r = srcBuffer;
    args[1U].r = undefinedRef;
    args[2U].i = start;
    args[3U].i = length;
    args[4U].i = extra;
    ASSERT_EQ(env_->Function_Call_Void_A(function, args.data()), ANI_PENDING_ERROR);
}

}  // namespace ark::ets::ani::testing
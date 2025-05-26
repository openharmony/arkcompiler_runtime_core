/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "libpandabase/utils/utils.h"
#include "plugins/ets/runtime/napi/ets_napi.h"
#include <cstdio>
#include <string>

extern "C" {
ETS_EXPORT void ETS_CALL ETS_InterfaceNativeTest_InterfaceNativeTest_callMethod(EtsEnv *env,
                                                                                [[maybe_unused]] ets_class cls)
{
    ets_method method = env->GetStaticp_method(cls, "staticLongMethod", "I:J");
    env->native_interface->CallStaticLongMethod(env, cls, method, static_cast<ets_int>(10));
    return;
}

ETS_EXPORT void ETS_CALL ETS_InterfaceNativeTest_InterfaceNativeTest_testChar(EtsEnv *env, ets_class cls, ets_char c)
{
    ets_value arr[1];
    arr[0].c = c;

    ets_method method = env->GetStaticp_method(cls, "staticVoidMethod", "C:V");
    env->CallStaticVoidMethodArray(cls, method, arr);
    return;
}

ETS_EXPORT ets_int ETS_CALL ETS_InterfaceNativeTest_InterfaceNativeTest_testFloat(EtsEnv *env, ets_class cls,
                                                                                  ets_float f1, ets_float f2,
                                                                                  ets_float f3)
{
    ets_method method = env->Getp_method(cls, "<ctor>", ":V");
    if (method == nullptr) {
        return -1;
    }

    float tmp[3U];
    tmp[0U] = f1;
    tmp[1U] = f2;
    tmp[2U] = f3;
    method = env->GetStaticp_method(cls, "staticFloatMethod", "I:F");
    for (size_t i = 0; i < 3U; i++) {
        ets_float res = env->CallStaticFloatMethod(cls, method, i);
        if (res != tmp[i]) {
            return -1;
        }
    }
    return 0;
}

ETS_EXPORT ets_int ETS_CALL ETS_InterfaceNativeTest_testNewObject(EtsEnv *env, ets_class cls)
{
    ets_method method = env->Getp_method(cls, "<ctor>", "I:V");
    if (method == nullptr) {
        return -1;
    }

    ets_value arr[1];
    arr[0].i = 7_I;
    ets_object etsobj = env->NewObjectArray(cls, method, arr);
    if (etsobj == nullptr) {
        return -1;
    }

    arr[0].i = 0;
    etsobj = env->NewObjectArray(cls, method, arr);
    env->ErrorClear();
    if (etsobj != nullptr) {
        return -1;
    }

    return 0;
}

ETS_EXPORT ets_int ETS_CALL ETS_InterfaceNativeTest_testStrUTFLen(EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                                  ets_string str1, ets_string str2)
{
    ets_size len1 = env->GetStringUTFLength(str1);
    ets_size len2 = env->GetStringUTFLength(str2);
    return len1 + len2;
}

ETS_EXPORT ets_string ETS_CALL ETS_InterfaceNativeTest_testNewStrUTF(EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                                     ets_string str)
{
    ets_boolean isCopy;
    const char *chars = env->GetStringUTFChars(str, &isCopy);
    ets_string etsstr = env->NewStringUTF(chars);
    env->ReleaseStringUTFChars(str, chars);
    return etsstr;
}

ETS_EXPORT ets_int ETS_CALL ETS_InterfaceNativeTest_testGetStrUTF(EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                                  ets_string str)
{
    std::string expected = {"HELLO WORLD!"};
    char buf1[6U] = {};
    char buf2[6U] = {};
    env->GetStringUTFRegion(str, 0, 6U, buf1);
    env->GetStringUTFRegion(str, 6U, 6U, buf2);
    for (size_t i = 0; i < 6U; i++) {
        if (buf1[i] != expected[i] || buf2[i] != expected[i + 6U]) {
            printf("error: buf1[%i] = '%c', buf2[%i] = '%c', expected '%c' and '%c'", i, buf1[i], i, buf2[i],
                   expected[i], expected[i + 6U]);
            return -1;
        }
    }
    return 0;
}

}  // extern "C"

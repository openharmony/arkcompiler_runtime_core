/**
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

#include "ani_gtest.h"

namespace ark::ets::ani::testing {

class NamespaceFindClassTest : public AniTest {};

TEST_F(NamespaceFindClassTest, find_class01)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("namespace_find_class_test.A", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_class result {};
    ASSERT_EQ(env_->Namespace_FindClass(ns, "C", &result), ANI_OK);
    ASSERT_NE(result, nullptr);
}

TEST_F(NamespaceFindClassTest, find_class02)
{
    ani_namespace ns {};

    ASSERT_EQ(env_->FindNamespace("namespace_find_class_test.test", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_namespace ns1 {};
    ASSERT_EQ(env_->Namespace_FindNamespace(ns, "A", &ns1), ANI_OK);
    ASSERT_NE(ns1, nullptr);

    ani_class tmp {};
    ASSERT_EQ(env_->Namespace_FindClass(ns1, "B", &tmp), ANI_OK);
    ASSERT_NE(tmp, nullptr);

    ani_method result {};
    ASSERT_EQ(env_->Class_FindMethod(tmp, "intMethod", "ii:i", &result), ANI_OK);
    ASSERT_NE(result, nullptr);
}

TEST_F(NamespaceFindClassTest, find_class03)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("namespace_find_class_test.test", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_namespace tmp {};
    ASSERT_EQ(env_->Namespace_FindNamespace(ns, "A", &tmp), ANI_OK);
    ASSERT_NE(tmp, nullptr);

    ani_class result {};
    ASSERT_EQ(env_->Namespace_FindClass(tmp, "B", &result), ANI_OK);
    ASSERT_NE(result, nullptr);

    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; i++) {
        ASSERT_EQ(env_->Namespace_FindNamespace(ns, "C", &tmp), ANI_OK);
        ASSERT_NE(tmp, nullptr);
        ASSERT_EQ(env_->Namespace_FindClass(tmp, "D", &result), ANI_OK);
        ASSERT_NE(result, nullptr);
    }
}

TEST_F(NamespaceFindClassTest, find_class04)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("namespace_find_class_test.test", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_namespace tmp {};
    ASSERT_EQ(env_->Namespace_FindNamespace(ns, "A", &tmp), ANI_OK);
    ASSERT_NE(tmp, nullptr);

    ani_namespace tmp1 {};
    ASSERT_EQ(env_->Namespace_FindNamespace(ns, "C", &tmp1), ANI_OK);
    ASSERT_NE(tmp1, nullptr);

    ani_class result {};
    ASSERT_EQ(env_->Namespace_FindClass(tmp1, "E", &result), ANI_OK);
    ASSERT_NE(result, nullptr);

    ASSERT_EQ(env_->Namespace_FindClass(tmp1, "F", &result), ANI_OK);
    ASSERT_NE(result, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(result, "Area;", "ii:i", &method), ANI_NOT_FOUND);

    ASSERT_EQ(env_->Namespace_FindClass(tmp1, "G", &result), ANI_OK);
    ASSERT_NE(result, nullptr);

    ani_static_method method1 {};
    ASSERT_EQ(env_->Class_FindStaticMethod(result, "methodB", "ii:i", &method1), ANI_OK);
    ASSERT_NE(method1, nullptr);

    ASSERT_EQ(env_->Class_FindMethod(result, "methodA", "ii:i", &method), ANI_OK);
    ASSERT_NE(method, nullptr);

    ASSERT_EQ(env_->Class_FindMethod(result, "Area", "ii:i", &method), ANI_OK);
    ASSERT_NE(method, nullptr);
}

TEST_F(NamespaceFindClassTest, find_class05)
{
    ani_class ns {};
    ASSERT_EQ(env_->FindClass("namespace_find_class_test.C", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_namespace ns1 {};
    ASSERT_EQ(env_->FindNamespace("namespace_find_class_test.J", &ns1), ANI_OK);
    ASSERT_NE(ns1, nullptr);

    ani_class result {};
    ASSERT_EQ(env_->Namespace_FindClass(ns1, "C", &result), ANI_OK);
    ASSERT_NE(result, nullptr);
}

TEST_F(NamespaceFindClassTest, find_class07)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("namespace_find_class_test.I", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_namespace tmp {};
    ASSERT_EQ(env_->Namespace_FindNamespace(ns, "A", &tmp), ANI_OK);
    ASSERT_NE(tmp, nullptr);

    ani_class result {};
    ASSERT_EQ(env_->Namespace_FindClass(tmp, "B", &result), ANI_OK);
    ASSERT_NE(result, nullptr);
}

TEST_F(NamespaceFindClassTest, find_class08)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("namespace_find_class_test.J", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_class result {};
    ASSERT_EQ(env_->Namespace_FindClass(ns, "B", &result), ANI_OK);
    ASSERT_NE(result, nullptr);
}

TEST_F(NamespaceFindClassTest, find_class09)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("namespace_find_class_test.J", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_class result {};
    ASSERT_EQ(env_->Namespace_FindClass(ns, "C", &result), ANI_OK);
    ASSERT_NE(result, nullptr);
}

TEST_F(NamespaceFindClassTest, find_class10)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("namespace_find_class_test.A", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_class result {};
    ASSERT_EQ(env_->Namespace_FindClass(ns, "C", &result), ANI_OK);
    ASSERT_NE(result, nullptr);

    ASSERT_EQ(env_->Namespace_FindClass(ns, "A.C", &result), ANI_NOT_FOUND);

    ASSERT_EQ(env_->Namespace_FindClass(ns, "B.C", &result), ANI_OK);
    ASSERT_NE(result, nullptr);
}

TEST_F(NamespaceFindClassTest, find_class11)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("namespace_find_class_test.A", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_class result {};
    ASSERT_EQ(env_->c_api->Namespace_FindClass(nullptr, ns, "C", &result), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Namespace_FindClass(nullptr, "C", &result), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Namespace_FindClass(ns, nullptr, &result), ANI_INVALID_ARGS);

    ASSERT_EQ(env_->Namespace_FindClass(ns, "", &result), ANI_NOT_FOUND);

    ASSERT_EQ(env_->Namespace_FindClass(ns, "CA", &result), ANI_NOT_FOUND);

    ASSERT_EQ(env_->Namespace_FindClass(ns, "B.C", nullptr), ANI_INVALID_ARGS);
}

TEST_F(NamespaceFindClassTest, check_initialization)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("namespace_find_class_test.A", &ns), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("namespace_find_class_test.A"));
    ASSERT_FALSE(IsRuntimeClassInitialized("namespace_find_class_test.A.C"));
    ani_class result {};
    ASSERT_EQ(env_->Namespace_FindClass(ns, "C", &result), ANI_OK);
    ASSERT_FALSE(IsRuntimeClassInitialized("namespace_find_class_test.A.C"));
    ASSERT_FALSE(IsRuntimeClassInitialized("namespace_find_class_test.A"));
}

}  // namespace ark::ets::ani::testing
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
#include <gtest/gtest.h>
#include "abckit_wrapper/file_view.h"
#include "test_util/assert.h"
#include "../../library/logger.h"

using namespace testing::ext;

namespace {
constexpr std::string_view ABC_FILE_PATH = ABCKIT_WRAPPER_ABC_FILE_DIR "ut/module_static.abc";
abckit_wrapper::FileView fileView;
constexpr std::string_view MODULE_NAME = "module_static";
}  // namespace

static std::string GetFullName(const std::string &objectRawName)
{
    return std::string(MODULE_NAME) + "." + objectRawName;
}

template <typename T>
static void AssertSignatureEqual(const std::string &objectRawName, const std::string &rawName,
                                 const std::string &descriptor)
{
    const auto object = fileView.Get<T>(GetFullName(objectRawName));
    ASSERT_TRUE(object.has_value());
    ASSERT_EQ(object.value()->GetRawName(), rawName);
    ASSERT_EQ(object.value()->GetDescriptor(), descriptor);
    ASSERT_EQ(object.value()->GetSignature(), rawName + descriptor);
}

class TestFileView : public ::testing::Test {
public:
    static void SetUpTestSuite()
    {
        ASSERT_SUCCESS(fileView.Init(ABC_FILE_PATH));
    }

    template <typename T>
    static void ValidateModuleObject(const std::string &objectRawName)
    {
        // 1. get module
        const auto &module = fileView.GetModule(MODULE_NAME.data());
        ASSERT_TRUE(module.has_value());

        // 2. build object full name
        const auto objectName = GetFullName(objectRawName);
        LOG_I << "objectName:" << objectName;

        // 3. validate exists object
        ASSERT_TRUE(fileView.GetObject(objectName).has_value());
        ASSERT_TRUE(fileView.Get<T>(objectName).has_value());
        ASSERT_TRUE(module.value()->Get<T>(objectName).has_value());

        // 4. validate not exists object
        ASSERT_FALSE(fileView.Get<T>("aaa.bbb").has_value());
        ASSERT_FALSE(module.value()->Get<T>("bbb.bcc").has_value());
    }
};

/**
 * @tc.name: fileView_namespace_001
 * @tc.desc: test fileView get namespace of module and namespace
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestFileView, fileView_namespace_001, TestSize.Level0)
{
    ValidateModuleObject<abckit_wrapper::Namespace>("Ns1");
    ValidateModuleObject<abckit_wrapper::Namespace>("Ns1.Ns2");
}

/**
 * @tc.name: fileView_class_001
 * @tc.desc: test fileView get class of module and namespace
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestFileView, fileView_class_001, TestSize.Level0)
{
    ValidateModuleObject<abckit_wrapper::Class>("ClassA");
    ValidateModuleObject<abckit_wrapper::Class>("Ns1.ClassB");
}

/**
 * @tc.name: fileView_interface_001
 * @tc.desc: test fileView get interface of module and namespace
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestFileView, fileView_interface_001, TestSize.Level0)
{
    ValidateModuleObject<abckit_wrapper::Class>("Interface1");
    ValidateModuleObject<abckit_wrapper::Class>("Ns1.Interface2");
}

/**
 * @tc.name: fileView_enum_001
 * @tc.desc: test fileView get enum of module and namespace
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestFileView, fileView_enum_001, TestSize.Level0)
{
    ValidateModuleObject<abckit_wrapper::Class>("Color1");
    ValidateModuleObject<abckit_wrapper::Class>("Ns1.Color2");
}
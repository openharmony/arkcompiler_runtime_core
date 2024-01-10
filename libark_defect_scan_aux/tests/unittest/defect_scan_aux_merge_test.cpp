/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include <array>
#include <iostream>

#include <gtest/gtest.h>
#include "defect_scan_aux_api.h"

namespace panda::defect_scan_aux::test {

class DefectScanAuxMergeTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

HWTEST(DefectScanAuxMergeTest, AcrossAbcTest, testing::ext::TestSize.Level0)
{
    std::string name = DEFECT_SCAN_AUX_TEST_MERGE_ABC_DIR "across_abc_test.abc";
    auto abc_file = panda::defect_scan_aux::AbcFile::Open(name);
    ASSERT(abc_file != nullptr);

    auto file_record_size = abc_file->GetFileRecordCount();
    EXPECT_EQ(file_record_size, 3U);
    auto file_record_list = abc_file->GetFileRecordList();
    auto f1_record_name = file_record_list.begin();
    EXPECT_EQ(*f1_record_name, "Lacross_abc_test;");
    auto f2_record_name = std::next(f1_record_name, 1);
    EXPECT_EQ(*f2_record_name, "Ldatabase;");
    auto f3_record_name = std::next(f2_record_name, 1);
    EXPECT_EQ(*f3_record_name, "Luser_input;");

    auto all_class_size = abc_file->GetDefinedClassCount();
    EXPECT_EQ(all_class_size, 3U);
    auto all_func_size = abc_file->GetDefinedFunctionCount();
    EXPECT_EQ(all_func_size, 18U);

    auto class_userinput = abc_file->GetClassByName("Luser_input;UserInput");
    ASSERT(class_userinput != nullptr);
    auto class_userinput_member_func_size = class_userinput->GetMemberFunctionCount();
    EXPECT_EQ(class_userinput_member_func_size, 3U);
    [[maybe_unused]] auto func_setText = class_userinput->GetMemberFunctionByName("Luser_input;setText");
    ASSERT(func_setText->GetClass() == class_userinput);

    auto userinput_func_main_0 = abc_file->GetFunctionByName("Luser_input;func_main_0");
    auto &graph = userinput_func_main_0->GetGraph();
    auto bb_list = graph.GetBasicBlockList();
    EXPECT_EQ(bb_list.size(), 3U);

    std::string inter_name0 = abc_file->GetLocalNameByExportName("DBInterface", "Ldatabase;");
    EXPECT_EQ(inter_name0, "DatabaseInterface");
    auto ex_class = abc_file->GetExportClassByExportName("DBInterface", "Ldatabase;");
    EXPECT_EQ(ex_class->GetClassName(), "Ldatabase;DatabaseInterface");

    std::string inter_name1 = abc_file->GetLocalNameByExportName("getblankInstanceInterface", "Ldatabase;");
    EXPECT_EQ(inter_name1, "getblankInstance1");
    auto ex_func1 = abc_file->GetExportFunctionByExportName("getblankInstanceInterface", "Ldatabase;");
    EXPECT_EQ(ex_func1->GetFunctionName(), "Ldatabase;getblankInstance1");

    auto ex_func2 = abc_file->GetExportFunctionByExportName("default", "Ldatabase;");
    EXPECT_EQ(ex_func2->GetFunctionName(), "Ldatabase;getblankInstance2");

    auto child_class = abc_file->GetClassByName("Ldatabase;DatabaseInterface");
    EXPECT_EQ(child_class->GetParentClassName(), "Ldatabase;Database");
    ASSERT_TRUE(child_class->GetMemberFunctionByName("Ldatabase;getText") == child_class->GetMemberFunctionByIndex(1));

    auto database_func_main_0 = abc_file->GetFunctionByName("Ldatabase;func_main_0");
    auto callee_info = database_func_main_0->GetCalleeInfoByIndex(0);
    ASSERT_TRUE(callee_info->IsCalleeDefinite());
    EXPECT_EQ(callee_info->GetClass(), abc_file->GetClassByName("Ldatabase;Database"));
    EXPECT_EQ(callee_info->GetCallee(), abc_file->GetFunctionByName("Ldatabase;addData"));
}
}  // namespace panda::defect_scan_aux::test

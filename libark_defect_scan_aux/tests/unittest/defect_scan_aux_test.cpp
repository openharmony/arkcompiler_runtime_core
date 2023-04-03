/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "../tests/test_helper.h"
#include "defect_scan_aux_api.h"

namespace panda::defect_scan_aux::test {
class DefectScanAuxTest {
public:
    explicit DefectScanAuxTest(std::string_view abc_filename)
    {
        abc_file_ = AbcFile::Open(abc_filename);
        TestHelper::ExpectTrue(abc_file_ != nullptr);
    }

    ~DefectScanAuxTest() = default;

    const std::unique_ptr<const AbcFile> &GetAbcFileInstance() const
    {
        return abc_file_;
    }

    const Function *CheckFunction(std::string_view func_name) const
    {
        auto func0 = abc_file_->GetFunctionByName(func_name);
        TestHelper::ExpectTrue(func0 != nullptr);
        TestHelper::ExpectEqual(func0->GetFunctionName(), func_name);
        TestHelper::ExpectTrue(func0->GetAbcFileInstance() == abc_file_.get());
        return func0;
    }

    bool ContainDefinedFunction(const Function *par_func, std::string_view func_name) const
    {
        size_t df_cnt0 = par_func->GetDefinedFunctionCount();
        for (size_t i = 0; i < df_cnt0; ++i) {
            auto df = par_func->GetDefinedFunctionByIndex(i);
            if (df->GetFunctionName() == func_name) {
                TestHelper::ExpectTrue(df->GetParentFunction() == par_func);
                return true;
            }
        }
        return false;
    }

    bool ContainMemberFunction(const Class *class0, std::string_view func_name) const
    {
        size_t mf_func_count = class0->GetMemberFunctionCount();
        for (size_t i = 0; i < mf_func_count; ++i) {
            auto mf = class0->GetMemberFunctionByIndex(i);
            if (mf->GetFunctionName() == func_name) {
                TestHelper::ExpectTrue(class0->GetMemberFunctionByName(func_name) == mf);
                return true;
            }
        }

        auto par_class = class0->GetParentClass();
        if (par_class != nullptr) {
            return ContainMemberFunction(par_class, func_name);
        }
        return false;
    }

    const Class *CheckClass(std::string_view class_name) const
    {
        auto *class0 = abc_file_->GetClassByName(class_name);
        TestHelper::ExpectTrue(class0 != nullptr);
        TestHelper::ExpectEqual(class0->GetClassName(), class_name);
        TestHelper::ExpectTrue(class0->GetAbcFileInstance() == abc_file_.get());
        size_t mf_func_count = class0->GetMemberFunctionCount();
        TestHelper::ExpectTrue(mf_func_count >= 1);
        auto mf_func0 = class0->GetMemberFunctionByIndex(0);
        TestHelper::ExpectTrue(abc_file_->GetFunctionByName(class_name) == mf_func0);
        TestHelper::ExpectTrue(class0->GetMemberFunctionByName(class_name) == mf_func0);
        TestHelper::ExpectTrue(mf_func0->GetClass() == class0);
        CheckFunction(class_name);
        return class0;
    }

private:
    std::unique_ptr<const AbcFile> abc_file_ {nullptr};
};
}  // namespace panda::defect_scan_aux::test

// TODO(wangyantian): use the test framework to refactor these codes
using TestHelper = panda::defect_scan_aux::test::TestHelper;
using Inst = panda::defect_scan_aux::Inst;
using InstType = panda::defect_scan_aux::InstType;
using DefectScanAuxTest = panda::defect_scan_aux::test::DefectScanAuxTest;
int main()
{
    constexpr size_t TESTCASE_NUM = 5;
    constexpr std::array<std::string_view, TESTCASE_NUM> TEST_CASE_LIST = {
        "defectscanaux_tests/unittest/module_info_test.abc", "defectscanaux_tests/unittest/define_info_test.abc",
        "defectscanaux_tests/unittest/callee_info_test.abc", "defectscanaux_tests/unittest/debug_info_test.abc",
        "defectscanaux_tests/unittest/graph_test.abc",

    };

    std::cout << "===== [libark_defect_scan_aux] Running Unittest =====" << std::endl;
    {
        constexpr size_t MODULE_INFO_TEST_INDEX = 0;
        std::cout << "    --- Running Testcase: " << TEST_CASE_LIST[MODULE_INFO_TEST_INDEX] << " ---" << std::endl;
        DefectScanAuxTest ds_test(TEST_CASE_LIST[MODULE_INFO_TEST_INDEX]);
        auto &abc_file = ds_test.GetAbcFileInstance();
        size_t def_func_cnt = abc_file->GetDefinedFunctionCount();
        TestHelper::ExpectEqual(def_func_cnt, 8);
        size_t def_class_cnt = abc_file->GetDefinedClassCount();
        TestHelper::ExpectEqual(def_class_cnt, 2);

        // check module info
        TestHelper::ExpectTrue(abc_file->IsModule());
        // check exported class
        std::string inter_name0 = abc_file->GetInternalNameByExportName("UInput");
        TestHelper::ExpectEqual(inter_name0, "UserInput");
        auto ex_class = abc_file->GetExportClassByExportName("UInput");
        TestHelper::ExpectEqual(ex_class->GetClassName(), "#2#UserInput");
        ds_test.CheckClass("#2#UserInput");
        size_t mf_func_count0 = ex_class->GetMemberFunctionCount();
        TestHelper::ExpectEqual(mf_func_count0, 2);
        auto mf_func0_1 = ex_class->GetMemberFunctionByIndex(1);
        TestHelper::ExpectEqual(mf_func0_1->GetFunctionName(), "getText");
        TestHelper::ExpectTrue(mf_func0_1->GetClass() == ex_class);
        ds_test.CheckFunction("getText");
        auto par_class = ex_class->GetParentClass();
        TestHelper::ExpectTrue(par_class != nullptr);
        TestHelper::ExpectEqual(par_class->GetClassName(), "#1#InnerUserInput");
        ds_test.CheckClass("#1#InnerUserInput");
        size_t mf_func_count1 = par_class->GetMemberFunctionCount();
        TestHelper::ExpectEqual(mf_func_count1, 2);
        auto mf_func1_1 = par_class->GetMemberFunctionByIndex(1);
        TestHelper::ExpectEqual(mf_func1_1->GetFunctionName(), "getTextBase");
        TestHelper::ExpectTrue(mf_func1_1->GetClass() == par_class);
        ds_test.CheckFunction("getTextBase");

        // check exported func
        // func3
        std::string inter_name1 = abc_file->GetInternalNameByExportName("exFunc3");
        TestHelper::ExpectEqual(inter_name1, "func3");
        auto ex_func1 = abc_file->GetExportFunctionByExportName("exFunc3");
        TestHelper::ExpectEqual(ex_func1->GetFunctionName(), "func3");
        ds_test.CheckFunction("func3");
        // func1
        std::string inter_name2 = abc_file->GetInternalNameByExportName("default");
        TestHelper::ExpectEqual(inter_name2, "func1");
        auto ex_func2 = abc_file->GetExportFunctionByExportName("default");
        TestHelper::ExpectEqual(ex_func2->GetFunctionName(), "func1");
        ds_test.CheckFunction(inter_name2);
        // func2
        std::string inter_name3 = abc_file->GetInternalNameByExportName("func2");
        TestHelper::ExpectEqual(inter_name3, "func2");
        auto ex_func3 = abc_file->GetExportFunctionByExportName("func2");
        TestHelper::ExpectEqual(ex_func3->GetFunctionName(), "func2");
        ds_test.CheckFunction(inter_name3);

        // GetModuleNameByInternalName
        std::string mod_name0 = abc_file->GetModuleNameByInternalName("var1");
        TestHelper::ExpectEqual(mod_name0, "./mod1");
        std::string mod_name1 = abc_file->GetModuleNameByInternalName("ns");
        TestHelper::ExpectEqual(mod_name1, "./mod2");
        std::string mod_name2 = abc_file->GetModuleNameByInternalName("var3");
        TestHelper::ExpectEqual(mod_name2, "../mod3");
        std::string mod_name3 = abc_file->GetModuleNameByInternalName("localVar4");
        TestHelper::ExpectEqual(mod_name3, "../../mod4");

        // GetImportNameByInternalName
        std::string im_name0 = abc_file->GetImportNameByInternalName("var1");
        TestHelper::ExpectEqual(im_name0, "default");
        std::string im_name1 = abc_file->GetImportNameByInternalName("var3");
        TestHelper::ExpectEqual(im_name1, "var3");
        std::string im_name2 = abc_file->GetImportNameByInternalName("localVar4");
        TestHelper::ExpectEqual(im_name2, "var4");
        // GetImportNameByExportName
        std::string ind_im_name0 = abc_file->GetImportNameByExportName("v");
        TestHelper::ExpectEqual(ind_im_name0, "v5");
        std::string ind_im_name1 = abc_file->GetImportNameByExportName("foo");
        TestHelper::ExpectEqual(ind_im_name1, "foo");
        // GetModuleNameByExportName
        std::string ind_mod_name0 = abc_file->GetModuleNameByExportName("v");
        TestHelper::ExpectEqual(ind_mod_name0, "./mod5");
        std::string ind_mod_name1 = abc_file->GetModuleNameByExportName("foo");
        TestHelper::ExpectEqual(ind_mod_name1, "../../mod7");
        std::cout << "    --- Pass ---" << std::endl << std::endl;
    }

    {
        constexpr size_t DEFINE_INFO_TEST_INDEX = 1;
        std::cout << "    --- Running Testcase: " << TEST_CASE_LIST[DEFINE_INFO_TEST_INDEX] << " ---" << std::endl;
        DefectScanAuxTest ds_test(TEST_CASE_LIST[DEFINE_INFO_TEST_INDEX]);
        auto &abc_file = ds_test.GetAbcFileInstance();
        TestHelper::ExpectTrue(abc_file->IsModule());
        size_t def_func_cnt = abc_file->GetDefinedFunctionCount();
        TestHelper::ExpectEqual(def_func_cnt, 30);
        size_t def_class_cnt = abc_file->GetDefinedClassCount();
        TestHelper::ExpectEqual(def_class_cnt, 10);

        // check each defined func
        // func_main_0
        auto f0 = ds_test.CheckFunction("func_main_0");
        TestHelper::ExpectTrue(f0->GetClass() == nullptr);
        TestHelper::ExpectTrue(f0->GetParentFunction() == nullptr);
        size_t dc_cnt0 = f0->GetDefinedClassCount();
        TestHelper::ExpectEqual(dc_cnt0, 2);
        TestHelper::ExpectEqual(f0->GetDefinedClassByIndex(0)->GetClassName(), "#1#Bar");
        TestHelper::ExpectEqual(f0->GetDefinedClassByIndex(1)->GetClassName(), "#6#ExampleClass1");
        size_t df_cnt0 = f0->GetDefinedFunctionCount();
        TestHelper::ExpectEqual(df_cnt0, 12);
        TestHelper::ExpectTrue(ds_test.ContainDefinedFunction(f0, "func1"));
        TestHelper::ExpectTrue(ds_test.ContainDefinedFunction(f0, "func2"));
        TestHelper::ExpectFalse(ds_test.ContainDefinedFunction(f0, "func3"));
        TestHelper::ExpectTrue(ds_test.ContainDefinedFunction(f0, "func6"));
        TestHelper::ExpectTrue(ds_test.ContainDefinedFunction(f0, "getName"));
        TestHelper::ExpectTrue(ds_test.ContainDefinedFunction(f0, "setName"));
        TestHelper::ExpectTrue(ds_test.ContainDefinedFunction(f0, "func9"));
        TestHelper::ExpectTrue(ds_test.ContainDefinedFunction(f0, "func10"));
        TestHelper::ExpectTrue(ds_test.ContainDefinedFunction(f0, "func17"));
        // func2
        auto f1 = ds_test.CheckFunction("func2");
        TestHelper::ExpectEqual(f1->GetArgCount(), 5);
        size_t df_cnt1 = f1->GetDefinedFunctionCount();
        TestHelper::ExpectEqual(df_cnt1, 2);
        TestHelper::ExpectTrue(ds_test.ContainDefinedFunction(f1, "func4"));
        // func10
        auto f2 = ds_test.CheckFunction("func10");
        TestHelper::ExpectEqual(f2->GetArgCount(), 3);
        size_t dc_cnt2 = f2->GetDefinedClassCount();
        TestHelper::ExpectEqual(dc_cnt2, 2);
        TestHelper::ExpectEqual(f2->GetDefinedClassByIndex(0)->GetClassName(), "#2#Bar");
        TestHelper::ExpectEqual(f2->GetDefinedClassByIndex(1)->GetClassName(), "#3#Bar2");
        size_t df_cnt2 = f2->GetDefinedFunctionCount();
        TestHelper::ExpectEqual(df_cnt2, 7);
        TestHelper::ExpectTrue(ds_test.ContainDefinedFunction(f2, "baseFoo1"));
        TestHelper::ExpectTrue(ds_test.ContainDefinedFunction(f2, "func12"));
        TestHelper::ExpectTrue(ds_test.ContainDefinedFunction(f2, "#4#"));
        TestHelper::ExpectTrue(ds_test.ContainDefinedFunction(f2, "#5#"));
        TestHelper::ExpectTrue(ds_test.ContainDefinedFunction(f2, "func15"));

        // check each defined class
        // #1#Bar
        auto class0 = ds_test.CheckClass("#1#Bar");
        TestHelper::ExpectTrue(class0->GetParentClass() == nullptr);
        TestHelper::ExpectTrue(class0->GetDefineFunction() == f0);
        size_t mf_count0 = class0->GetMemberFunctionCount();
        TestHelper::ExpectEqual(mf_count0, 5);
        TestHelper::ExpectTrue(ds_test.ContainMemberFunction(class0, "func6"));
        TestHelper::ExpectTrue(ds_test.ContainMemberFunction(class0, "getName"));
        TestHelper::ExpectTrue(ds_test.ContainMemberFunction(class0, "setName"));
        TestHelper::ExpectTrue(ds_test.ContainMemberFunction(class0, "func9"));
        // #3#Bar2
        auto class1 = ds_test.CheckClass("#3#Bar2");
        TestHelper::ExpectTrue(class1->GetParentClass() != nullptr);
        TestHelper::ExpectTrue(class1->GetDefineFunction() == abc_file->GetFunctionByName("func10"));
        size_t mf_count1 = class1->GetMemberFunctionCount();
        TestHelper::ExpectEqual(mf_count1, 5);
        TestHelper::ExpectTrue(ds_test.ContainMemberFunction(class1, "baseFoo1"));
        TestHelper::ExpectTrue(ds_test.ContainMemberFunction(class1, "func12"));
        TestHelper::ExpectTrue(ds_test.ContainMemberFunction(class1, "func15"));
        // #8#ExampleClass2
        auto class2 = ds_test.CheckClass("#8#ExampleClass2");
        TestHelper::ExpectTrue(class2->GetParentClass() == abc_file->GetClassByName("#7#ExampleClass1"));
        TestHelper::ExpectFalse(ds_test.ContainMemberFunction(class2, "func17"));
        TestHelper::ExpectTrue(ds_test.ContainMemberFunction(class2, "func19"));
        // #9#ExtendService
        auto class3 = ds_test.CheckClass("#9#ExtendService");
        TestHelper::ExpectTrue(class3->GetParentClass() == nullptr);
        TestHelper::ExpectEqual(class3->GetParentClassName(), "BaseService");
        TestHelper::ExpectEqual(class3->GetParClassExternalModuleName(), "../base/service");
        TestHelper::ExpectTrue(class3->GetParClassGlobalVarName().empty());
        // #10#ExtendPhoneService
        auto class4 = ds_test.CheckClass("#10#ExtendPhoneService");
        TestHelper::ExpectTrue(class4->GetParentClass() == nullptr);
        TestHelper::ExpectEqual(class4->GetParentClassName(), "PhoneService");
        TestHelper::ExpectEqual(class4->GetParClassExternalModuleName(), "../mod1");
        TestHelper::ExpectTrue(class4->GetParClassGlobalVarName().empty());
        // #11#ExtendDataSource
        auto class5 = ds_test.CheckClass("#11#ExtendDataSource");
        TestHelper::ExpectTrue(class5->GetParentClass() == nullptr);
        TestHelper::ExpectEqual(class5->GetParentClassName(), "BasicDataSource");
        TestHelper::ExpectTrue(class5->GetParClassExternalModuleName().empty());
        TestHelper::ExpectEqual(class5->GetParClassGlobalVarName(), "globalvar");
        // #12#ExtendDataItem
        auto class6 = ds_test.CheckClass("#12#ExtendDataItem");
        TestHelper::ExpectTrue(class6->GetParentClass() == nullptr);
        TestHelper::ExpectEqual(class6->GetParentClassName(), "DataItem");
        TestHelper::ExpectTrue(class6->GetParClassExternalModuleName().empty());
        TestHelper::ExpectEqual(class6->GetParClassGlobalVarName(), "globalvar2.Data");
        std::cout << "    --- Pass ---" << std::endl << std::endl;
    }

    {
        constexpr size_t CALLEE_INFO_TEST_INDEX = 2;
        std::cout << "    --- Running Testcase: " << TEST_CASE_LIST[CALLEE_INFO_TEST_INDEX] << " ---" << std::endl;
        DefectScanAuxTest ds_test(TEST_CASE_LIST[CALLEE_INFO_TEST_INDEX]);
        auto &abc_file = ds_test.GetAbcFileInstance();
        TestHelper::ExpectTrue(abc_file->IsModule());
        size_t def_func_cnt = abc_file->GetDefinedFunctionCount();
        TestHelper::ExpectEqual(def_func_cnt, 19);
        size_t def_class_cnt = abc_file->GetDefinedClassCount();
        TestHelper::ExpectEqual(def_class_cnt, 2);

        // check callee info of each func
        // foo
        auto f0 = abc_file->GetFunctionByName("foo");
        size_t ci_cnt0 = f0->GetCalleeInfoCount();
        TestHelper::ExpectEqual(ci_cnt0, 6);
        auto ci0_0 = f0->GetCalleeInfoByIndex(0);
        TestHelper::ExpectTrue(f0->GetCalleeInfoByCallInst(ci0_0->GetCallInst()) == ci0_0);
        TestHelper::ExpectTrue(ci0_0->IsCalleeDefinite());
        TestHelper::ExpectEqual(ci0_0->GetCalleeArgCount(), 1);
        TestHelper::ExpectTrue(ci0_0->GetCaller() == f0);
        TestHelper::ExpectTrue(ci0_0->GetCallee() == abc_file->GetFunctionByName("func2"));
        auto ci0_1 = f0->GetCalleeInfoByIndex(1);
        TestHelper::ExpectEqual(f0->GetCalleeInfoByCallInst(ci0_1->GetCallInst()), ci0_1);
        TestHelper::ExpectFalse(ci0_1->IsCalleeDefinite());
        TestHelper::ExpectEqual(ci0_1->GetCalleeArgCount(), 1);
        TestHelper::ExpectTrue(ci0_1->GetCallee() == nullptr);
        TestHelper::ExpectEqual(ci0_1->GetFunctionName(), "log");
        TestHelper::ExpectEqual(ci0_1->GetGlobalVarName(), "console");
        auto ci0_2 = f0->GetCalleeInfoByIndex(2);
        TestHelper::ExpectFalse(ci0_2->IsCalleeDefinite());
        TestHelper::ExpectEqual(ci0_2->GetCalleeArgCount(), 1);
        TestHelper::ExpectTrue(ci0_2->GetCallee() == nullptr);
        TestHelper::ExpectEqual(ci0_2->GetFunctionName(), "logd");
        TestHelper::ExpectEqual(ci0_2->GetGlobalVarName(), "globalvar.hilog");
        auto ci0_3 = f0->GetCalleeInfoByIndex(3);
        TestHelper::ExpectTrue(ci0_3->IsCalleeDefinite());
        TestHelper::ExpectTrue(ci0_3->GetCallee() == abc_file->GetFunctionByName("func2"));
        auto ci0_4 = f0->GetCalleeInfoByIndex(4);
        TestHelper::ExpectTrue(ci0_4->IsCalleeDefinite());
        TestHelper::ExpectEqual(ci0_4->GetCalleeArgCount(), 2);
        TestHelper::ExpectTrue(ci0_4->GetCallee() == abc_file->GetFunctionByName("func1"));
        auto ci0_5 = f0->GetCalleeInfoByIndex(5);
        TestHelper::ExpectFalse(ci0_5->IsCalleeDefinite());
        TestHelper::ExpectEqual(ci0_5->GetFunctionName(), "bar");
        // foo1
        auto f1 = abc_file->GetFunctionByName("foo1");
        size_t ci_cnt1 = f1->GetCalleeInfoCount();
        TestHelper::ExpectEqual(ci_cnt1, 3);
        auto ci1_0 = f1->GetCalleeInfoByIndex(0);
        TestHelper::ExpectTrue(ci1_0->IsCalleeDefinite());
        TestHelper::ExpectTrue(ci1_0->GetCallee() == abc_file->GetFunctionByName("fn"));
        auto ci1_1 = f1->GetCalleeInfoByIndex(1);
        TestHelper::ExpectTrue(ci1_1->IsCalleeDefinite());
        TestHelper::ExpectTrue(ci1_1->GetCallee() == abc_file->GetFunctionByName("fn"));
        auto ci1_2 = f1->GetCalleeInfoByIndex(2);
        TestHelper::ExpectFalse(ci1_2->IsCalleeDefinite());
        TestHelper::ExpectEqual(ci1_2->GetFunctionName(), "bind");
        // #2#ColorPoint
        auto f2 = abc_file->GetFunctionByName("#2#ColorPoint");
        size_t ci_cnt2 = f2->GetCalleeInfoCount();
        TestHelper::ExpectEqual(ci_cnt2, 1);
        auto ci2_0 = f2->GetCalleeInfoByIndex(0);
        TestHelper::ExpectTrue(ci2_0->IsCalleeDefinite());
        TestHelper::ExpectTrue(ci2_0->GetClass() == abc_file->GetClassByName("#1#Point"));
        TestHelper::ExpectTrue(ci2_0->GetCallee() == abc_file->GetFunctionByName("#1#Point"));
        // func6
        auto f3 = abc_file->GetFunctionByName("func6");
        size_t ci_cnt3 = f3->GetCalleeInfoCount();
        TestHelper::ExpectEqual(ci_cnt3, 1);
        auto ci3_0 = f3->GetCalleeInfoByIndex(0);
        TestHelper::ExpectFalse(ci3_0->IsCalleeDefinite());
        TestHelper::ExpectEqual(ci3_0->GetFunctionName(), "bar");
        TestHelper::ExpectEqual(ci3_0->GetExternalModuleName(), "./mod2");
        // func7
        auto f4 = abc_file->GetFunctionByName("func7");
        size_t ci_cnt4 = f4->GetCalleeInfoCount();
        TestHelper::ExpectEqual(ci_cnt4, 2);
        auto ci4_0 = f4->GetCalleeInfoByIndex(0);
        TestHelper::ExpectFalse(ci4_0->IsCalleeDefinite());
        TestHelper::ExpectEqual(ci4_0->GetCalleeArgCount(), 2);
        TestHelper::ExpectEqual(ci4_0->GetFunctionName(), "sum");
        TestHelper::ExpectEqual(ci4_0->GetExternalModuleName(), "../../mod1");
        auto ci4_1 = f4->GetCalleeInfoByIndex(1);
        TestHelper::ExpectFalse(ci4_1->IsCalleeDefinite());
        TestHelper::ExpectEqual(ci4_1->GetCalleeArgCount(), 2);
        TestHelper::ExpectEqual(ci4_1->GetFunctionName(), "sub");
        TestHelper::ExpectEqual(ci4_1->GetExternalModuleName(), "../../mod1");
        // callMemberFunc1
        auto f5 = abc_file->GetFunctionByName("callMemberFunc1");
        size_t ci_cnt5 = f5->GetCalleeInfoCount();
        TestHelper::ExpectEqual(ci_cnt5, 2);
        auto ci5_0 = f5->GetCalleeInfoByIndex(0);
        TestHelper::ExpectTrue(ci5_0->IsCalleeDefinite());
        TestHelper::ExpectTrue(ci5_0->GetClass() != nullptr);
        TestHelper::ExpectEqual(ci5_0->GetClass(), abc_file->GetClassByName("#1#Point"));
        TestHelper::ExpectTrue(ci5_0->GetCallee() != nullptr);
        TestHelper::ExpectEqual(ci5_0->GetCallee(), abc_file->GetFunctionByName("getCoordinateX"));
        auto ci5_1 = f5->GetCalleeInfoByIndex(1);
        TestHelper::ExpectTrue(ci5_1->IsCalleeDefinite());
        TestHelper::ExpectTrue(ci5_1->GetClass() != nullptr);
        TestHelper::ExpectEqual(ci5_1->GetClass(), abc_file->GetClassByName("#1#Point"));
        TestHelper::ExpectTrue(ci5_1->GetCallee() != nullptr);
        TestHelper::ExpectEqual(ci5_1->GetCallee(), abc_file->GetFunctionByName("setCoordinateX"));
        // callMemberFunc2
        auto f6 = abc_file->GetFunctionByName("callMemberFunc2");
        size_t ci_cnt6 = f6->GetCalleeInfoCount();
        TestHelper::ExpectEqual(ci_cnt6, 2);
        auto ci6_0 = f6->GetCalleeInfoByIndex(0);
        TestHelper::ExpectTrue(ci6_0->IsCalleeDefinite());
        TestHelper::ExpectTrue(ci6_0->GetClass() != nullptr);
        TestHelper::ExpectEqual(ci6_0->GetClass(), abc_file->GetClassByName("#1#Point"));
        TestHelper::ExpectTrue(ci6_0->GetCallee() != nullptr);
        TestHelper::ExpectEqual(ci6_0->GetCallee(), abc_file->GetFunctionByName("plus"));
        auto ci6_1 = f6->GetCalleeInfoByIndex(1);
        TestHelper::ExpectFalse(ci6_1->IsCalleeDefinite());
        TestHelper::ExpectEqual(ci6_1->GetFunctionName(), "sub");
        TestHelper::ExpectEqual(ci6_1->GetExternalModuleName(), "");
        TestHelper::ExpectEqual(ci6_1->GetGlobalVarName(), "");
        // callExClassMemberFunc1
        auto f7 = abc_file->GetFunctionByName("callExClassMemberFunc1");
        size_t ci_cnt7 = f7->GetCalleeInfoCount();
        TestHelper::ExpectEqual(ci_cnt7, 1);
        auto ci7_0 = f7->GetCalleeInfoByIndex(0);
        TestHelper::ExpectFalse(ci7_0->IsCalleeDefinite());
        TestHelper::ExpectTrue(ci7_0->GetClass() == nullptr);
        TestHelper::ExpectEqual(ci7_0->GetFunctionName(), "makePhoneCall");
        TestHelper::ExpectEqual(ci7_0->GetExternalModuleName(), "../../mod1");
        TestHelper::ExpectEqual(ci7_0->GetGlobalVarName(), "");
        std::cout << "    --- Pass ---" << std::endl << std::endl;
    }

    {
        constexpr size_t DEBUG_INFO_TEST_INDEX = 3;
        std::cout << "    --- Running Testcase: " << TEST_CASE_LIST[DEBUG_INFO_TEST_INDEX] << " ---" << std::endl;
        DefectScanAuxTest ds_test(TEST_CASE_LIST[DEBUG_INFO_TEST_INDEX]);
        auto &abc_file = ds_test.GetAbcFileInstance();
        TestHelper::ExpectFalse(abc_file->IsModule());

        // check debug info, whether the line number obtained from call inst is correct
        auto f0 = abc_file->GetFunctionByName("foo");
        TestHelper::ExpectEqual(f0->GetCalleeInfoCount(), 3);
        auto ci0_0 = f0->GetCalleeInfoByIndex(0);
        // callarg0
        TestHelper::ExpectEqual(abc_file->GetLineNumberByInst(f0, ci0_0->GetCallInst()), 34);
        auto ci0_1 = f0->GetCalleeInfoByIndex(1);
        // callspread
        TestHelper::ExpectEqual(abc_file->GetLineNumberByInst(f0, ci0_1->GetCallInst()), 38);
        auto ci0_2 = f0->GetCalleeInfoByIndex(2);
        // callirange
        TestHelper::ExpectEqual(abc_file->GetLineNumberByInst(f0, ci0_2->GetCallInst()), 40);
        // ctor of Data
        auto f1 = abc_file->GetFunctionByName("#2#Data");
        TestHelper::ExpectEqual(f1->GetCalleeInfoCount(), 1);
        auto ci1_0 = f1->GetCalleeInfoByIndex(0);
        // supercall
        TestHelper::ExpectEqual(abc_file->GetLineNumberByInst(f1, ci1_0->GetCallInst()), 60);
        // bar
        auto f2 = abc_file->GetFunctionByName("bar");
        TestHelper::ExpectEqual(f2->GetCalleeInfoCount(), 2);
        auto ci2_0 = f2->GetCalleeInfoByIndex(0);
        // callithisrange
        TestHelper::ExpectEqual(abc_file->GetLineNumberByInst(f2, ci2_0->GetCallInst()), 70);
        auto ci2_1 = f2->GetCalleeInfoByIndex(1);
        // callithisrange
        TestHelper::ExpectEqual(abc_file->GetLineNumberByInst(f2, ci2_1->GetCallInst()), 75);
        std::cout << "    --- Pass ---" << std::endl << std::endl;
    }

    {
        constexpr size_t GRAPH_TEST_INDEX = 4;
        std::cout << "    --- Running Testcase: " << TEST_CASE_LIST[GRAPH_TEST_INDEX] << " ---" << std::endl;
        DefectScanAuxTest ds_test(TEST_CASE_LIST[GRAPH_TEST_INDEX]);
        auto f0 = ds_test.CheckFunction("foo");

        // check api of graph
        auto &graph = f0->GetGraph();
        auto bb_list = graph.GetBasicBlockList();
        TestHelper::ExpectEqual(bb_list.size(), 8);
        TestHelper::ExpectEqual(bb_list.front(), graph.GetStartBasicBlock());
        TestHelper::ExpectEqual(bb_list.back(), graph.GetEndBasicBlock());
        std::unordered_map<InstType, uint32_t> inst_cnt_table;
        graph.VisitAllInstructions([&inst_cnt_table](const Inst &inst) {
            auto type = inst.GetType();
            inst_cnt_table.insert_or_assign(type, inst_cnt_table[type] + 1);
        });
        uint32_t newobj_cnt = inst_cnt_table[InstType::NEWOBJRANGE_IMM8_IMM8_V8] +
                              inst_cnt_table[InstType::NEWOBJRANGE_IMM16_IMM8_V8] +
                              inst_cnt_table[InstType::WIDE_NEWOBJRANGE_PREF_IMM16_V8];
        TestHelper::ExpectEqual(newobj_cnt, 2);
        uint32_t ldlex_cnt = inst_cnt_table[InstType::LDLEXVAR_IMM4_IMM4] +
                             inst_cnt_table[InstType::LDLEXVAR_IMM8_IMM8] +
                             inst_cnt_table[InstType::WIDE_LDLEXVAR_PREF_IMM16_IMM16];
        TestHelper::ExpectEqual(ldlex_cnt, 7);
        TestHelper::ExpectEqual(
            inst_cnt_table[InstType::LDOBJBYNAME_IMM8_ID16] + inst_cnt_table[InstType::LDOBJBYNAME_IMM16_ID16], 4);
        TestHelper::ExpectEqual(inst_cnt_table[InstType::CALLTHIS1_IMM8_V8_V8], 3);
        TestHelper::ExpectEqual(inst_cnt_table[InstType::CALLTHIS2_IMM8_V8_V8_V8], 1);
        TestHelper::ExpectEqual(inst_cnt_table[InstType::DEFINEFUNC_IMM8_ID16_IMM8] +
                                    inst_cnt_table[InstType::DEFINEFUNC_IMM16_ID16_IMM8],
                                1);
        TestHelper::ExpectEqual(inst_cnt_table[InstType::CALLARG0_IMM8], 1);
        TestHelper::ExpectEqual(inst_cnt_table[InstType::CALLARG1_IMM8_V8], 1);
        TestHelper::ExpectEqual(inst_cnt_table[InstType::LDEXTERNALMODULEVAR_IMM8] +
                                    inst_cnt_table[InstType::WIDE_LDEXTERNALMODULEVAR_PREF_IMM16],
                                1);
        TestHelper::ExpectEqual(inst_cnt_table[InstType::GETMODULENAMESPACE_IMM8] +
                                    inst_cnt_table[InstType::WIDE_GETMODULENAMESPACE_PREF_IMM16],
                                0);
        TestHelper::ExpectEqual(inst_cnt_table[InstType::OPCODE_PARAMETER], 5);

        // check api of basic block
        auto bb0 = graph.GetStartBasicBlock();
        TestHelper::ExpectEqual(bb0.GetPredBlocks().size(), 0);
        TestHelper::ExpectEqual(bb0.GetSuccBlocks().size(), 1);
        auto bb1 = bb0.GetSuccBlocks()[0];
        TestHelper::ExpectEqual(bb1.GetPredBlocks().size(), 1);
        TestHelper::ExpectEqual(bb1.GetPredBlocks()[0], bb0);
        auto bb1_succ_bb = bb1.GetSuccBlocks();
        TestHelper::ExpectEqual(bb1_succ_bb.size(), 2);
        TestHelper::ExpectTrue(
            (bb1_succ_bb[0].GetSuccBlocks().size() == 2 && bb1_succ_bb[1].GetSuccBlocks().size() == 1) ||
            (bb1_succ_bb[0].GetSuccBlocks().size() == 1 && bb1_succ_bb[1].GetSuccBlocks().size() == 2));
        auto bb2 = graph.GetEndBasicBlock();
        auto bb2_pred_bb = bb2.GetPredBlocks();
        TestHelper::ExpectEqual(bb2_pred_bb.size(), 2);
        auto bb3 = bb2_pred_bb[0];
        auto bb4 = bb2_pred_bb[1];
        if (bb3.GetPredBlocks().size() < bb4.GetPredBlocks().size()) {
            std::swap(bb3, bb4);
        }
        TestHelper::ExpectEqual(bb3.GetPredBlocks().size(), 2);
        TestHelper::ExpectEqual(bb4.GetPredBlocks().size(), 1);
        TestHelper::ExpectEqual(bb4.GetPredBlocks()[0], bb1);

        // check api of inst
        size_t ci_cnt = f0->GetCalleeInfoCount();
        TestHelper::ExpectEqual(ci_cnt, 6);
        auto ci0 = f0->GetCalleeInfoByIndex(ci_cnt - 1);
        auto call_inst0 = ci0->GetCallInst();
        auto call_inst0_ins = call_inst0.GetInputInsts();
        TestHelper::ExpectEqual(call_inst0_ins.size(), 4);
        auto call_inst0_in1_type = call_inst0_ins[1].GetType();
        TestHelper::ExpectEqual(call_inst0_in1_type, InstType::CALLARG0_IMM8);
        TestHelper::ExpectEqual(call_inst0_ins[1].GetUserInsts().size(), 1);
        TestHelper::ExpectEqual(call_inst0_ins[1].GetUserInsts()[0], call_inst0);
        auto call_inst0_in2_type = call_inst0_ins[2].GetType();
        TestHelper::ExpectEqual(call_inst0_in2_type, InstType::OPCODE_PARAMETER);
        TestHelper::ExpectEqual(call_inst0_ins[2].GetArgIndex(), 4);
        auto param1_usrs = call_inst0_ins[2].GetUserInsts();
        TestHelper::ExpectTrue(std::find(param1_usrs.begin(), param1_usrs.end(), call_inst0) != param1_usrs.end());
        auto call_inst0_in3_type = call_inst0_ins[3].GetType();
        TestHelper::ExpectTrue(call_inst0_in3_type == InstType::LDOBJBYNAME_IMM8_ID16 ||
                                call_inst0_in3_type == InstType::LDOBJBYNAME_IMM16_ID16);
        TestHelper::ExpectEqual(call_inst0_ins[3].GetUserInsts().size(), 1);
        TestHelper::ExpectEqual(call_inst0_ins[3].GetUserInsts()[0], call_inst0);

        auto ci1 = f0->GetCalleeInfoByIndex(ci_cnt - 2);
        auto call_inst1 = ci1->GetCallInst();
        auto call_inst1_ins = call_inst1.GetInputInsts();
        TestHelper::ExpectEqual(call_inst1_ins.size(), 2);
        auto call_inst1_in0_type = call_inst1_ins[1].GetType();
        TestHelper::ExpectTrue(call_inst1_in0_type == InstType::LDEXTERNALMODULEVAR_IMM8 ||
                                call_inst1_in0_type == InstType::WIDE_LDEXTERNALMODULEVAR_PREF_IMM16);
        TestHelper::ExpectEqual(call_inst1_ins[1].GetUserInsts().size(), 2);
        TestHelper::ExpectTrue((call_inst1_ins[1].GetUserInsts()[0] == call_inst1) ||
                               (call_inst1_ins[1].GetUserInsts()[1] == call_inst1));
        auto phi_inst = call_inst1_ins[0];
        TestHelper::ExpectEqual(phi_inst.GetType(), InstType::OPCODE_PHI);
        auto phi_inst_ins = phi_inst.GetInputInsts();
        TestHelper::ExpectEqual(phi_inst_ins.size(), 2);
        auto phi_inst_in0 = phi_inst_ins[0];
        auto phi_inst_in1 = phi_inst_ins[1];
        if (phi_inst_in0.GetType() != InstType::OPCODE_PARAMETER) {
            std::swap(phi_inst_in0, phi_inst_in1);
        }
        TestHelper::ExpectEqual(phi_inst_in0.GetType(), InstType::OPCODE_PARAMETER);
        TestHelper::ExpectEqual(phi_inst_in0.GetArgIndex(), 3);
        auto param0_usrs = phi_inst_in0.GetUserInsts();
        TestHelper::ExpectTrue(std::find(param0_usrs.begin(), param0_usrs.end(), phi_inst) != param0_usrs.end());
        TestHelper::ExpectEqual(phi_inst_in1.GetType(), InstType::ADD2_IMM8_V8);
        auto add2_inst_ins = phi_inst_in1.GetInputInsts();
        TestHelper::ExpectEqual(add2_inst_ins.size(), 2);
        TestHelper::ExpectEqual(add2_inst_ins[0].GetType(), InstType::OPCODE_PARAMETER);
        TestHelper::ExpectEqual(add2_inst_ins[1].GetType(), InstType::OPCODE_PARAMETER);
        std::cout << "    --- Pass ---" << std::endl;
    }
    std::cout << "===== [libark_defect_scan_aux] Unittest Pass =====" << std::endl;

    return 0;
}
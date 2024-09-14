/*
 * Copyright (c) 2024 Shenzhen Kaihong Digital Industry Development Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at

 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cstddef>
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "compiler_options.h"
#include "graph_test.h"
#include "optimizer/ir/graph.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/optimizations/lowering.h"
#include "optimizer/optimizations/move_constants.h"
#include "optimizer/optimizations/regalloc/reg_alloc.h"
#include "optimizer/optimizations/vn.h"
#include "optimizer/pass_manager.h"
#include "optimizer/pass_manager_statistics.h"
#include "reg_acc_alloc.h"

using namespace testing::ext;

namespace panda::compiler {
class PassManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}

    void SetUp()
    {
        option_enable_ir_stats_ = options.IsCompilerEnableIrStats();
        option_print_stats_ = options.IsCompilerPrintStats();
        option_reset_local_allocator_ = options.IsCompilerResetLocalAllocator();
        option_dump_final_ = options.IsCompilerDumpFinal();
        option_dump_life_intervals_ = options.IsCompilerDumpLifeIntervals();
        option_dump_stats_csv_ = options.GetCompilerDumpStatsCsv();
        option_dump_folder_ = options.GetCompilerDumpFolder();
    }

    void TearDown()
    {
        CleanDumpFiles();
        RestoreCompilerOptions();
    }

    static void CleanDumpFiles()
    {
        std::filesystem::remove(GRAPH_TEST_ABC_DIR "pass_manager_test_002.csv");
        std::filesystem::remove_all(GRAPH_TEST_ABC_DIR "/pass_manager_test_003");
        std::filesystem::remove_all(GRAPH_TEST_ABC_DIR "/pass_manager_test_004");
        std::filesystem::remove_all(GRAPH_TEST_ABC_DIR "/pass_manager_test_005");
    }

    void RestoreCompilerOptions()
    {
        options.SetCompilerEnableIrStats(option_enable_ir_stats_);
        options.SetCompilerPrintStats(option_print_stats_);
        options.SetCompilerResetLocalAllocator(option_reset_local_allocator_);
        options.SetCompilerDumpFinal(option_dump_final_);
        options.SetCompilerDumpLifeIntervals(option_dump_life_intervals_);
        options.SetCompilerDumpStatsCsv(option_dump_stats_csv_);
        options.SetCompilerDumpFolder(option_dump_folder_);
    }

    bool option_enable_ir_stats_;
    bool option_print_stats_;
    bool option_reset_local_allocator_;
    bool option_dump_final_;
    bool option_dump_life_intervals_;
    std::string option_dump_stats_csv_;
    std::string option_dump_folder_;

    GraphTest graph_test_;
};

/**
 * @tc.name: pass_manager_test_001
 * @tc.desc: Verify the PrintStatistics function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PassManagerTest, pass_manager_test_001, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "graphTest.abc";
    const char *test_method_name = "loop1";
    bool status = false;
    options.SetCompilerEnableIrStats(false);
    options.SetCompilerPrintStats(true);
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        graph->RunPass<Cleanup>();
        graph->RunPass<Lowering>();

        std::stringstream ss;
        auto old_buf = std::cerr.rdbuf();
        std::cerr.rdbuf(ss.rdbuf());

        graph->GetPassManager()->Finalize();

        std::cerr.rdbuf(old_buf);

        std::string line;
        std::vector<std::string> lines;
        while (std::getline(ss, line)) {
            lines.emplace_back(line);
        }
        constexpr size_t ID_PASSNAME_COLUMN_WIDTH = 40;
        EXPECT_EQ(lines[0], "  ID Pass Name                         :       IR mem   Local mem     Time,us");
        EXPECT_EQ(lines[1], "-----------------------------------------------------------------------------");
        EXPECT_EQ(lines[2].substr(0, ID_PASSNAME_COLUMN_WIDTH), "   0 IrBuilder                         :");
        EXPECT_EQ(lines[3].substr(0, ID_PASSNAME_COLUMN_WIDTH), "   1   RPO                             :");
        EXPECT_EQ(lines[4].substr(0, ID_PASSNAME_COLUMN_WIDTH), "   2   DominatorTree                   :");
        EXPECT_EQ(lines[5].substr(0, ID_PASSNAME_COLUMN_WIDTH), "   3   LoopAnalysis                    :");
        EXPECT_EQ(lines[6].substr(0, ID_PASSNAME_COLUMN_WIDTH), "   4   LoopAnalysis                    :");
        EXPECT_EQ(lines[7].substr(0, ID_PASSNAME_COLUMN_WIDTH), "   5 Cleanup                           :");
        EXPECT_EQ(lines[8].substr(0, ID_PASSNAME_COLUMN_WIDTH), "   6 Lowering                          :");
        EXPECT_EQ(lines[9], "-----------------------------------------------------------------------------");
        EXPECT_EQ(lines[10].substr(0, ID_PASSNAME_COLUMN_WIDTH), "                                  TOTAL:");
        EXPECT_EQ(lines[11], "PBC instruction number : 13");
        
        status = true;
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: pass_manager_test_002
 * @tc.desc: Verify the DumpStatisticsCsv function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PassManagerTest, pass_manager_test_002, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "graphTest.abc";
    std::string csv_file = GRAPH_TEST_ABC_DIR "pass_manager_test_002.csv";
    const char *test_method_name = "loop1";
    bool status = false;
    options.SetCompilerPrintStats(false);
    options.SetCompilerDumpStatsCsv(csv_file);
    options.SetCompilerEnableIrStats(true);
    options.SetCompilerResetLocalAllocator(false);
    graph_test_.TestBuildGraphFromFile(pfile,
        [&test_method_name, &status, &csv_file](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        graph->RunPass<MoveConstants>();
        graph->RunPass<bytecodeopt::RegAccAlloc>();
        RegAlloc(graph);

        graph->GetPassManager()->Finalize();

        std::ifstream csv(csv_file);
        std::stringstream ss;
        ss << csv.rdbuf();

        std::string line;
        std::vector<std::string> lines;
        while (std::getline(ss, line)) {
            lines.emplace_back(line);
        }

        constexpr size_t PASSNAME_COLUMN_START = sizeof("\"L_GLOBAL;::#*#loop1\"");
        const auto get_first_two_columns = [](std::string &line) {
            return line.substr(0, line.find(',', PASSNAME_COLUMN_START));
        };
        EXPECT_EQ(get_first_two_columns(lines[0]), "\"L_GLOBAL;::#*#loop1\",IrBuilder");
        EXPECT_EQ(get_first_two_columns(lines[1]), "\"L_GLOBAL;::#*#loop1\",RPO");
        EXPECT_EQ(get_first_two_columns(lines[2]), "\"L_GLOBAL;::#*#loop1\",DominatorTree");
        EXPECT_EQ(get_first_two_columns(lines[3]), "\"L_GLOBAL;::#*#loop1\",LoopAnalysis");
        EXPECT_EQ(get_first_two_columns(lines[4]), "\"L_GLOBAL;::#*#loop1\",LoopAnalysis");
        EXPECT_EQ(get_first_two_columns(lines[5]), "\"L_GLOBAL;::#*#loop1\",MoveConstants");
        EXPECT_EQ(get_first_two_columns(lines[6]), "\"L_GLOBAL;::#*#loop1\",RegAccAlloc");
        EXPECT_EQ(get_first_two_columns(lines[7]), "\"L_GLOBAL;::#*#loop1\",Cleanup");
        EXPECT_EQ(get_first_two_columns(lines[8]), "\"L_GLOBAL;::#*#loop1\",RegAllocGraphColoring");
        EXPECT_EQ(get_first_two_columns(lines[9]), "\"L_GLOBAL;::#*#loop1\",LivenessAnalysis");

        status = true;
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: pass_manager_test_003
 * @tc.desc: Verify the DumpGraph function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PassManagerTest, pass_manager_test_003, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "graphTest.abc";
    std::string outdir = GRAPH_TEST_ABC_DIR "/pass_manager_test_003";
    const char *test_method_name = "loop1";
    bool status = false;
    options.SetCompilerDumpFinal(false);
    options.SetCompilerDumpFolder(outdir);
    graph_test_.TestBuildGraphFromFile(pfile,
        [&test_method_name, &status, &outdir](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        options.SetCompilerDump(true);
        graph->RunPass<ValNum>();
        graph->RunPass<Cleanup>();
        options.SetCompilerDump(false);
        
        EXPECT_TRUE(std::filesystem::exists(outdir + "/846_pass_0006_L_GLOBAL_____loop1_GVN.ir"));
        EXPECT_TRUE(std::filesystem::exists(outdir + "/846_pass_0007_L_GLOBAL_____loop1_Cleanup.ir"));

        status = true;
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: pass_manager_test_004
 * @tc.desc: Verify the DumpLifeIntervals function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PassManagerTest, pass_manager_test_004, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "graphTest.abc";
    std::string outdir = GRAPH_TEST_ABC_DIR "/pass_manager_test_004";
    const char *test_method_name = "loop1";
    bool status = false;
    options.SetCompilerDumpFolder(outdir);
    graph_test_.TestBuildGraphFromFile(pfile,
        [&test_method_name, &status, &outdir](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        options.SetCompilerDumpLifeIntervals(true);
        graph->RunPass<bytecodeopt::RegAccAlloc>();
        RegAlloc(graph);
        options.SetCompilerDumpLifeIntervals(false);

        EXPECT_TRUE(std::filesystem::exists(outdir + "/854_pass_0008_L_GLOBAL_____loop1_RegAllocGraphColoring.li"));

        status = true;
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: pass_manager_test_005
 * @tc.desc: Verify the GetFileName function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PassManagerTest, pass_manager_test_005, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "graphTest.abc";
    std::string outdir = GRAPH_TEST_ABC_DIR "/pass_manager_test_005";
    const char *test_method_name = "loop1";
    bool status = false;
    options.SetCompilerDumpFolder(outdir);
    graph_test_.TestBuildGraphFromFile(pfile,
        [&test_method_name, &status, &outdir](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        auto filename1 = graph->GetPassManager()->GetFileName("test", ".ir");
        EXPECT_EQ(filename1, outdir + "/862_pass_0005_L_GLOBAL_____loop1_test.ir");

        GraphMode mode = graph->GetMode();
        mode.SetOsr(true);
        graph->SetMode(mode);
        auto filename2 = graph->GetPassManager()->GetFileName("test", ".ir");
        EXPECT_EQ(filename2, outdir + "/862_pass_0005_L_GLOBAL_____loop1_OSR_test.ir");
        mode.SetOsr(false);
        graph->SetMode(mode);

        auto child_graph = graph->CreateChildGraph(graph->GetMethod());
        auto filename3 = child_graph->GetPassManager()->GetFileName("child", ".ir");
        EXPECT_EQ(filename3, outdir + "/862_pass_0005_inlined_L_GLOBAL_____loop1_child.ir");

        auto filename4 = graph->GetPassManager()->GetFileName(nullptr, ".ir");
        EXPECT_EQ(filename4, outdir + "/862_L_GLOBAL_____loop1.ir");

        status = true;
    });
    EXPECT_TRUE(status);
}
}  // namespace panda::compiler

/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include <cstring>
#include <gtest/gtest.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <memory>

#include "compiler_options.h"
#include "constants.h"
#include "graph_checker.h"
#include "graph_visitor.h"
#include "graph_test.h"
#include "inst.h"
#include "locations.h"
#include "mem/pool_manager.h"

using namespace testing::ext;

namespace panda::compiler {

class GraphCheckerTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};

    GraphTest graphTest_;
};

/**
 * @tc.name: graph_checker_test_001
 * @tc.desc: Verify the Check function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(GraphCheckerTest, graph_checker_test_001, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "styleTryCatch.abc";
    const char *test_method_name = "func6";
    bool status = false;
    graphTest_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);

        graph->InvalidateAnalysis<LoopAnalyzer>();
        EXPECT_TRUE(graph->RunPass<LoopAnalyzer>());
        GraphChecker gChecker(graph);
        gChecker.Check();
    });
    EXPECT_TRUE(status);
}
}
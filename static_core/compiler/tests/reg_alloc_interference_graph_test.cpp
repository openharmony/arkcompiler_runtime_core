/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "unit_test.h"
#include "optimizer/optimizations/regalloc/interference_graph.h"
#include <utility>
#include <algorithm>

namespace panda::compiler {
class RegAllocInterferenceTest : public GraphTest {};

namespace {
constexpr unsigned DEFAULT_CAPACITY1 = 10;
unsigned TEST_EDGES1[2U][2U] = {{0U, 1U}, {7U, 4U}};  // NOLINT(modernize-avoid-c-arrays)
auto IS_IN_SET = [](unsigned a, unsigned b) {
    for (size_t i = 0; i < 2U; i++) {  // NOLINT(modernize-loop-convert)
        if ((a == TEST_EDGES1[i][0U] && b == TEST_EDGES1[i][1U]) ||
            (b == TEST_EDGES1[i][0U] && a == TEST_EDGES1[i][1U])) {
            return true;
        }
    }
    return false;
};
}  // namespace

// NOLINTBEGIN(readability-magic-numbers)
TEST_F(RegAllocInterferenceTest, Basic)
{
    GraphMatrix matrix(GetLocalAllocator());
    matrix.SetCapacity(DEFAULT_CAPACITY1);
    EXPECT_FALSE(matrix.AddEdge(0U, 1U));
    EXPECT_FALSE(matrix.AddEdge(7U, 4U));
    for (unsigned i = 0; i < DEFAULT_CAPACITY1; i++) {
        for (unsigned j = 0; j < DEFAULT_CAPACITY1; j++) {
            ASSERT_EQ(matrix.HasEdge(i, j), IS_IN_SET(i, j));
        }
    }
    EXPECT_GE(matrix.GetCapacity(), DEFAULT_CAPACITY1);
}

TEST_F(RegAllocInterferenceTest, BasicAfinity)
{
    GraphMatrix matrix(GetLocalAllocator());
    matrix.SetCapacity(DEFAULT_CAPACITY1);
    EXPECT_FALSE(matrix.AddAffinityEdge(0U, 1U));
    EXPECT_FALSE(matrix.AddAffinityEdge(7U, 4U));
    for (unsigned i = 0; i < DEFAULT_CAPACITY1; i++) {
        for (unsigned j = 0; j < DEFAULT_CAPACITY1; j++) {
            EXPECT_EQ(matrix.HasAffinityEdge(i, j), IS_IN_SET(i, j));
        }
    }
    EXPECT_GE(matrix.GetCapacity(), DEFAULT_CAPACITY1);
}

TEST_F(RegAllocInterferenceTest, BasicGraph)
{
    InterferenceGraph gr(GetLocalAllocator());
    gr.Reserve(DEFAULT_CAPACITY1);

    EXPECT_EQ(gr.Size(), 0U);
    auto *node1 = gr.AllocNode();
    EXPECT_EQ(gr.Size(), 1U);
    EXPECT_EQ(node1->GetNumber(), 0U);

    auto *node2 = gr.AllocNode();
    EXPECT_EQ(gr.Size(), 2U);
    EXPECT_EQ(node2->GetNumber(), 1U);
    EXPECT_NE(node1, node2);
}

TEST_F(RegAllocInterferenceTest, GraphChordal)
{
    InterferenceGraph gr(GetLocalAllocator());
    gr.Reserve(DEFAULT_CAPACITY1);

    EXPECT_TRUE(gr.IsChordal());

    gr.AllocNode();
    EXPECT_TRUE(gr.IsChordal());

    gr.AllocNode();
    EXPECT_TRUE(gr.IsChordal());

    gr.AllocNode();
    EXPECT_TRUE(gr.IsChordal());

    gr.AddEdge(0U, 1U);
    EXPECT_TRUE(gr.IsChordal());

    gr.AddEdge(1U, 2U);
    EXPECT_TRUE(gr.IsChordal());

    gr.AddEdge(0U, 2U);
    EXPECT_TRUE(gr.IsChordal());

    // Make nonchordal
    gr.AllocNode();
    gr.AllocNode();
    gr.AddEdge(3U, 2U);
    gr.AddEdge(3U, 4U);
    gr.AddEdge(0U, 4U);
    EXPECT_FALSE(gr.IsChordal());
}

namespace {
const unsigned DEFAULT_CAPACITY2 = 5;
const unsigned DEFAULT_EDGES2 = 6;
// NOLINTNEXTLINE(modernize-avoid-c-arrays)
::std::pair<unsigned, unsigned> TEST_EDGES2[DEFAULT_EDGES2] = {{0U, 1U}, {1U, 2U}, {2U, 0U},
                                                               {0U, 3U}, {2U, 3U}, {3U, 4U}};

// To prevent adding "remove edge" interfaces to main code, edge removing is simulated via building new graph without
// it.
InterferenceGraph BuildSubgraph(InterferenceGraph &orig_gr, ArenaAllocator *alloc,
                                ::std::pair<unsigned, unsigned> *edges, unsigned count, ArenaVector<unsigned> &peo,
                                unsigned peo_count)
{
    InterferenceGraph gr(alloc);
    gr.Reserve(orig_gr.Size());

    for (unsigned i = 0; i < count; i++) {
        auto x = edges[i].first;   // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto y = edges[i].second;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        for (unsigned j = 0; j < peo_count; j++) {
            if (x == peo[j] || y == peo[j]) {
                continue;
            }
        }
        gr.AddEdge(x, y);
    }

    return gr;
}
}  // namespace

TEST_F(RegAllocInterferenceTest, LexBFSSimple)
{
    InterferenceGraph gr(GetLocalAllocator());
    gr.Reserve(2U);

    gr.AllocNode();
    gr.AllocNode();
    gr.AddEdge(0U, 1U);

    auto peo = gr.LexBFS();
    EXPECT_EQ(peo.size(), 2U);
    EXPECT_EQ(peo[0U], 0U);
    EXPECT_EQ(peo[1U], 1U);
}

TEST_F(RegAllocInterferenceTest, LexBFS)
{
    InterferenceGraph gr(GetLocalAllocator());
    gr.Reserve(DEFAULT_CAPACITY2);

    gr.AllocNode();
    gr.AllocNode();
    gr.AllocNode();
    gr.AllocNode();
    gr.AllocNode();
    for (auto &test_edge : TEST_EDGES2) {
        auto x = test_edge.first;
        auto y = test_edge.second;
        gr.AddEdge(x, y);
    }

    auto peo = gr.LexBFS();
    EXPECT_EQ(peo.size(), DEFAULT_CAPACITY2);
    std::reverse(peo.begin(), peo.end());

    for (unsigned i = 0; i < (DEFAULT_CAPACITY2 - 1L); i++) {
        auto gr2 = BuildSubgraph(gr, GetLocalAllocator(), TEST_EDGES2, DEFAULT_EDGES2, peo, i);
        EXPECT_TRUE(gr2.IsChordal());
    }
}

TEST_F(RegAllocInterferenceTest, AssignColorsSimple)
{
    InterferenceGraph gr(GetLocalAllocator());
    gr.Reserve(DEFAULT_CAPACITY2);

    auto *nd0 = gr.AllocNode();
    auto *nd1 = gr.AllocNode();
    auto *nd2 = gr.AllocNode();
    auto *nd3 = gr.AllocNode();
    auto *nd4 = gr.AllocNode();
    for (auto &test_edge : TEST_EDGES2) {
        auto x = test_edge.first;
        auto y = test_edge.second;
        gr.AddEdge(x, y);
    }

    EXPECT_TRUE(gr.AssignColors<32U>(3U, 0U));
    EXPECT_NE(nd0->GetColor(), nd1->GetColor());
    EXPECT_NE(nd0->GetColor(), nd2->GetColor());
    EXPECT_NE(nd0->GetColor(), nd3->GetColor());

    EXPECT_NE(nd2->GetColor(), nd1->GetColor());
    EXPECT_NE(nd2->GetColor(), nd3->GetColor());

    EXPECT_NE(nd4->GetColor(), nd3->GetColor());
}

TEST_F(RegAllocInterferenceTest, AssignColors)
{
    const unsigned default_capacity = 11;
    const unsigned default_edges = 12;
    const unsigned default_aedges = 4;
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    ::std::pair<unsigned, unsigned> test_edges[default_edges] = {{0U, 1U}, {1U, 2U}, {2U, 0U},  {0U, 3U},
                                                                 {2U, 3U}, {3U, 4U}, {6U, 5U},  {5U, 7U},
                                                                 {6U, 7U}, {9U, 8U}, {9U, 10U}, {8U, 10U}};
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    ::std::pair<unsigned, unsigned> test_aedges[default_aedges] = {{3U, 6U}, {6U, 9U}, {2U, 5U}, {7U, 8U}};

    InterferenceGraph gr(GetLocalAllocator());
    gr.Reserve(default_capacity);

    auto *nd0 = gr.AllocNode();
    auto *nd1 = gr.AllocNode();
    auto *nd2 = gr.AllocNode();
    auto *nd3 = gr.AllocNode();
    auto *nd4 = gr.AllocNode();
    auto *nd5 = gr.AllocNode();
    auto *nd6 = gr.AllocNode();
    auto *nd7 = gr.AllocNode();
    auto *nd8 = gr.AllocNode();
    auto *nd9 = gr.AllocNode();
    auto *nd10 = gr.AllocNode();

    for (auto &test_edge : test_edges) {
        auto x = test_edge.first;
        auto y = test_edge.second;
        gr.AddEdge(x, y);
    }
    for (auto &test_edge : test_aedges) {
        auto x = test_edge.first;
        auto y = test_edge.second;
        gr.AddAffinityEdge(x, y);
    }
    auto &bias0 = gr.AddBias();
    auto &bias1 = gr.AddBias();
    auto &bias2 = gr.AddBias();

    nd3->SetBias(0U);
    nd6->SetBias(0U);
    nd9->SetBias(0U);
    nd2->SetBias(1U);
    nd5->SetBias(1U);
    nd7->SetBias(2U);
    nd8->SetBias(2U);

    EXPECT_TRUE(gr.AssignColors<32U>(3U, 0U));

    // Check nodes inequality
    EXPECT_NE(nd0->GetColor(), nd1->GetColor());
    EXPECT_NE(nd0->GetColor(), nd2->GetColor());
    EXPECT_NE(nd0->GetColor(), nd3->GetColor());

    EXPECT_NE(nd2->GetColor(), nd1->GetColor());
    EXPECT_NE(nd2->GetColor(), nd3->GetColor());

    EXPECT_NE(nd4->GetColor(), nd3->GetColor());

    EXPECT_NE(nd5->GetColor(), nd6->GetColor());
    EXPECT_NE(nd7->GetColor(), nd6->GetColor());
    EXPECT_NE(nd5->GetColor(), nd7->GetColor());

    EXPECT_NE(nd8->GetColor(), nd9->GetColor());
    EXPECT_NE(nd8->GetColor(), nd10->GetColor());
    EXPECT_NE(nd9->GetColor(), nd10->GetColor());

    // Check biases work
    EXPECT_EQ(nd3->GetColor(), nd6->GetColor());
    EXPECT_EQ(nd9->GetColor(), nd6->GetColor());

    EXPECT_EQ(nd2->GetColor(), nd5->GetColor());
    EXPECT_EQ(nd7->GetColor(), nd8->GetColor());

    // Check biases values
    EXPECT_NE(bias0.color, bias1.color);
    EXPECT_NE(bias0.color, bias2.color);
}
// NOLINTEND(readability-magic-numbers)

}  // namespace panda::compiler

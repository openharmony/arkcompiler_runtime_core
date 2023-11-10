/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef COMPILER_TESTS_UNIT_TEST_H
#define COMPILER_TESTS_UNIT_TEST_H

#include "macros.h"
#include "optimizer/ir/ir_constructor.h"
#include "gtest/gtest.h"
#include "mem/arena_allocator.h"
#include "mem/pool_manager.h"
#include <numeric>
#include <unordered_map>
#include "compiler.h"
#include "compiler_logger.h"
#include "graph_comparator.h"
#include "include/runtime.h"
#include "libpandafile/file_item_container.h"

namespace panda::compiler {
struct RuntimeInterfaceMock : public compiler::RuntimeInterface {
    // Only one exact CALLEE may exist in fake runtime
    MethodPtr GetMethodById(MethodPtr /* unused */, MethodId id) const override
    {
        saved_callee_id_ = id;
        return reinterpret_cast<MethodPtr>(CALLEE);
    }

    MethodId GetMethodId(MethodPtr method) const override
    {
        return (reinterpret_cast<uintptr_t>(method) == CALLEE) ? saved_callee_id_ : 0;
    }

    uint64_t GetUniqMethodId(MethodPtr method) const override
    {
        return (reinterpret_cast<uintptr_t>(method) == CALLEE) ? saved_callee_id_ : 0;
    }

    BinaryFilePtr GetBinaryFileForMethod(MethodPtr /* unused */) const override
    {
        static constexpr uintptr_t FAKE_FILE = 0xdeadf;
        return reinterpret_cast<void *>(FAKE_FILE);
    }

    DataType::Type GetMethodReturnType(MethodPtr /* unused */) const override
    {
        return return_type_;
    }

    DataType::Type GetMethodReturnType(MethodPtr /* unused */, MethodId /* unused */) const override
    {
        return return_type_;
    }

    DataType::Type GetMethodTotalArgumentType(MethodPtr /* unused */, size_t index) const override
    {
        if (arg_types_ == nullptr || index >= arg_types_->size()) {
            return DataType::NO_TYPE;
        }
        return arg_types_->at(index);
    }

    size_t GetMethodTotalArgumentsCount(MethodPtr /* unused */) const override
    {
        if (arg_types_ == nullptr) {
            return args_count_;
        }
        return arg_types_->size();
    }

    size_t GetMethodArgumentsCount(MethodPtr /* unused */) const override
    {
        return args_count_;
    }

    size_t GetMethodRegistersCount(MethodPtr /* unused */) const override
    {
        return vregs_count_;
    }

    DataType::Type GetFieldType(PandaRuntimeInterface::FieldPtr field) const override
    {
        return field_types_ == nullptr ? DataType::NO_TYPE : (*field_types_)[field];
    }

private:
    static constexpr uintptr_t METHOD = 0xdead;
    static constexpr uintptr_t CALLEE = 0xdeadc;

    size_t args_count_ {0};
    size_t vregs_count_ {0};
    mutable unsigned saved_callee_id_ {0};
    DataType::Type return_type_ {DataType::NO_TYPE};
    ArenaVector<DataType::Type> *arg_types_ {nullptr};
    ArenaUnorderedMap<PandaRuntimeInterface::FieldPtr, DataType::Type> *field_types_ {nullptr};

    friend class GraphTest;
    friend class GraphCreator;
    friend class InstGenerator;
};

class CommonTest : public ::testing::Test {
public:
    CommonTest()
    {
#if defined(PANDA_TARGET_ARM64) || defined(PANDA_TARGET_32)
        // We have issue with QEMU - so reduce memory heap
        panda::mem::MemConfig::Initialize(32_MB, 64_MB, 200_MB, 32_MB, 0, 0);  // NOLINT(readability-magic-numbers)
#else
        panda::mem::MemConfig::Initialize(32_MB, 64_MB, 256_MB, 32_MB, 0, 0);  // NOLINT(readability-magic-numbers)
#endif
        PoolManager::Initialize();
        allocator_ = new ArenaAllocator(SpaceType::SPACE_TYPE_COMPILER);
        object_allocator_ = new ArenaAllocator(SpaceType::SPACE_TYPE_OBJECT);
        local_allocator_ = new ArenaAllocator(SpaceType::SPACE_TYPE_COMPILER);
        builder_ = new IrConstructor();
    }
    ~CommonTest() override;

    NO_COPY_SEMANTIC(CommonTest);
    NO_MOVE_SEMANTIC(CommonTest);

    ArenaAllocator *GetAllocator() const
    {
        return allocator_;
    }

    ArenaAllocator *GetObjectAllocator() const
    {
        return object_allocator_;
    }

    ArenaAllocator *GetLocalAllocator() const
    {
        return local_allocator_;
    }

    Arch GetArch() const
    {
        return arch_;
    }

    Graph *CreateEmptyGraph(bool is_osr = false) const
    {
        return GetAllocator()->New<Graph>(GetAllocator(), GetLocalAllocator(), arch_, is_osr);
    }

    Graph *CreateEmptyGraph(Arch arch) const
    {
        return GetAllocator()->New<Graph>(GetAllocator(), GetLocalAllocator(), arch, false);
    }

    Graph *CreateGraphStartEndBlocks(bool is_dynamic = false) const
    {
        auto graph = GetAllocator()->New<Graph>(GetAllocator(), GetLocalAllocator(), arch_, is_dynamic, false);
        graph->CreateStartBlock();
        graph->CreateEndBlock();
        return graph;
    }
    Graph *CreateDynEmptyGraph() const
    {
        return GetAllocator()->New<Graph>(GetAllocator(), GetLocalAllocator(), arch_, true, false);
    }
    Graph *CreateEmptyBytecodeGraph() const
    {
        return GetAllocator()->New<Graph>(GetAllocator(), GetLocalAllocator(), Arch::NONE, false, true);
    }
    Graph *CreateEmptyFastpathGraph(Arch arch) const
    {
        auto graph = GetAllocator()->New<Graph>(GetAllocator(), GetLocalAllocator(), arch, false);
        graph->SetMode(GraphMode::FastPath());
        return graph;
    }

    BasicBlock *CreateEmptyBlock(Graph *graph) const
    {
        auto block = graph->GetAllocator()->New<BasicBlock>(graph);
        graph->AddBlock(block);
        return block;
    }

    ArenaVector<BasicBlock *> GetBlocksById(Graph *graph, std::vector<size_t> &&ids) const
    {
        ArenaVector<BasicBlock *> blocks(graph->GetAllocator()->Adapter());
        for (auto id : ids) {
            blocks.push_back(&BB(id));
        }
        return blocks;
    }

    bool CheckInputs(Inst &inst, std::initializer_list<size_t> list) const
    {
        if (inst.GetInputs().Size() != list.size()) {
            return false;
        }
        auto it2 = list.begin();
        for (auto it = inst.GetInputs().begin(); it != inst.GetInputs().end(); ++it, ++it2) {
            if (it->GetInst() != &INS(*it2)) {
                return false;
            }
        }
        return true;
    }

    bool CheckUsers(Inst &inst, std::initializer_list<int> list) const
    {
        std::unordered_map<int, size_t> users_map;
        for (auto l : list) {
            ++users_map[l];
        }
        for (auto &user : inst.GetUsers()) {
            EXPECT_EQ(user.GetInst()->GetInput(user.GetIndex()).GetInst(), &inst);
            if (users_map[user.GetInst()->GetId()]-- == 0) {
                return false;
            }
        }
        auto rest = std::accumulate(users_map.begin(), users_map.end(), 0, [](int a, auto &x) { return a + x.second; });
        EXPECT_EQ(rest, 0);
        return rest == 0;
    }

protected:
    IrConstructor *builder_;  // NOLINT(misc-non-private-member-variables-in-classes)

private:
    ArenaAllocator *allocator_;
    ArenaAllocator *object_allocator_;
    ArenaAllocator *local_allocator_;
#ifdef PANDA_TARGET_ARM32
    Arch arch_ {Arch::AARCH32};
#else
    Arch arch_ {Arch::AARCH64};
#endif
};

class GraphTest : public CommonTest {
public:
    GraphTest() : graph_(CreateEmptyGraph())
    {
        graph_->SetRuntime(&runtime_);
        runtime_.field_types_ =
            graph_->GetAllocator()->New<ArenaUnorderedMap<PandaRuntimeInterface::FieldPtr, DataType::Type>>(
                graph_->GetAllocator()->Adapter());
    }
    ~GraphTest() override = default;

    NO_MOVE_SEMANTIC(GraphTest);
    NO_COPY_SEMANTIC(GraphTest);

    Graph *GetGraph() const
    {
        return graph_;
    }

    void SetGraphArch(Arch arch)
    {
        graph_ = CreateEmptyGraph(arch);
        graph_->SetRuntime(&runtime_);
    }

    void ResetGraph()
    {
        graph_ = CreateEmptyGraph();
        graph_->SetRuntime(&runtime_);
    }

    void SetNumVirtRegs(size_t num)
    {
        runtime_.vregs_count_ = num;
        graph_->SetVRegsCount(std::max(graph_->GetVRegsCount(), runtime_.vregs_count_ + runtime_.args_count_ + 1));
    }

    void SetNumArgs(size_t num)
    {
        runtime_.args_count_ = num;
        graph_->SetVRegsCount(std::max(graph_->GetVRegsCount(), runtime_.vregs_count_ + runtime_.args_count_ + 1));
    }

    void RegisterFieldType(PandaRuntimeInterface::FieldPtr field, DataType::Type type)
    {
        (*runtime_.field_types_)[field] = type;
    }

protected:
    RuntimeInterfaceMock runtime_;  // NOLINT(misc-non-private-member-variables-in-classes)
    Graph *graph_ {nullptr};        // NOLINT(misc-non-private-member-variables-in-classes)
};

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PandaRuntimeTest : public ::testing::Test, public PandaRuntimeInterface {
public:
    PandaRuntimeTest();
    ~PandaRuntimeTest() override;

    NO_MOVE_SEMANTIC(PandaRuntimeTest);
    NO_COPY_SEMANTIC(PandaRuntimeTest);

    static void Initialize(int argc, char **argv);

    Graph *CreateGraph()
    {
        auto graph = GetAllocator()->New<Graph>(GetAllocator(), GetLocalAllocator(), arch_);
        graph->SetRuntime(this);
        return graph;
    }

    Graph *CreateGraphOsr()
    {
        Graph *graph = GetAllocator()->New<Graph>(GetAllocator(), GetLocalAllocator(), arch_, true);
        graph->SetRuntime(this);
        return graph;
    }

    // this method is needed to create a graph with a working dump
    Graph *CreateGraphWithDefaultRuntime()
    {
        auto *graph = GetAllocator()->New<Graph>(GetAllocator(), GetLocalAllocator(), arch_);
        graph->SetRuntime(GetDefaultRuntime());
        return graph;
    }

    Graph *CreateGraphDynWithDefaultRuntime()
    {
        auto *graph = GetAllocator()->New<Graph>(GetAllocator(), GetLocalAllocator(), arch_);
        graph->SetRuntime(GetDefaultRuntime());
        graph->SetDynamicMethod();
#ifndef NDEBUG
        graph->SetDynUnitTestFlag();
#endif
        return graph;
    }

    Graph *CreateGraphDynStubWithDefaultRuntime()
    {
        auto *graph = GetAllocator()->New<Graph>(GetAllocator(), GetLocalAllocator(), arch_);
        graph->SetRuntime(GetDefaultRuntime());
        graph->SetDynamicMethod();
        graph->SetDynamicStub();
#ifndef NDEBUG
        graph->SetDynUnitTestFlag();
#endif
        return graph;
    }

    // this method is needed to create a graph with a working dump
    Graph *CreateGraphOsrWithDefaultRuntime()
    {
        auto *graph = GetAllocator()->New<Graph>(GetAllocator(), GetLocalAllocator(), arch_, true);
        graph->SetRuntime(GetDefaultRuntime());
        return graph;
    }

    ArenaAllocator *GetAllocator()
    {
        return allocator_;
    }

    ArenaAllocator *GetLocalAllocator()
    {
        return local_allocator_;
    }

    virtual Graph *GetGraph()
    {
        return graph_;
    }

    auto GetClassLinker()
    {
        return panda::Runtime::GetCurrent()->GetClassLinker();
    }

    void EnableLogs(Logger::Level level = Logger::Level::DEBUG)
    {
        Logger::EnableComponent(Logger::Component::COMPILER);
        Logger::SetLevel(level);
    }

    const char *GetExecPath() const
    {
        return exec_path_;
    }

    static RuntimeInterface *GetDefaultRuntime();

protected:
    IrConstructor *builder_;  // NOLINT(misc-non-private-member-variables-in-classes)

private:
    Graph *graph_ {nullptr};
    ArenaAllocator *allocator_ {nullptr};
    ArenaAllocator *local_allocator_ {nullptr};
    static inline const char *exec_path_ {nullptr};
    Arch arch_ {RUNTIME_ARCH};
};

// NOLINTNEXTLINE(fuchsia-multia-inheritance,cppcoreguidelines-special-member-functions)
class AsmTest : public PandaRuntimeTest {
public:
    std::unique_ptr<const panda_file::File> ParseToFile(const char *source, const char *file_name = "test.pb");
    bool Parse(const char *source, const char *file_name = "test.pb");
    Graph *BuildGraph(const char *method_name, Graph *graph = nullptr);
    void CleanUp(Graph *graph);

    AsmTest() = default;
    ~AsmTest() override = default;
    NO_MOVE_SEMANTIC(AsmTest);
    NO_COPY_SEMANTIC(AsmTest);

    template <bool WITH_CLEANUP = false>
    bool ParseToGraph(const char *source, const char *method_name, Graph *graph = nullptr)
    {
        if (!Parse(source)) {
            return false;
        }
        if (graph == nullptr) {
            graph = GetGraph();
        }
        if (BuildGraph(method_name, graph) == nullptr) {
            return false;
        }
        if constexpr (WITH_CLEANUP) {
            CleanUp(graph);
        }
        return true;
    }
};

struct TmpFile {
    explicit TmpFile(const char *file_name) : file_name_(file_name) {}
    ~TmpFile()
    {
        ASSERT(file_name_ != nullptr);
        remove(file_name_);
    }

    NO_MOVE_SEMANTIC(TmpFile);
    NO_COPY_SEMANTIC(TmpFile);

    const char *GetFileName() const
    {
        return file_name_;
    }

private:
    const char *file_name_ {nullptr};
};
}  // namespace panda::compiler

#endif  // COMPILER_TESTS_UNIT_TEST_H

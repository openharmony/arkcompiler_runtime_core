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

#ifndef COMPILER_COMPILE_METHOD_H_
#define COMPILER_COMPILE_METHOD_H_

#include "compiler_options.h"
#include "mem/arena_allocator.h"
#include "mem/code_allocator.h"
#include "include/method.h"
#include "utils/arch.h"

namespace panda::compiler {
class Graph;
class RuntimeInterface;

class JITStats {
public:
    explicit JITStats(mem::InternalAllocatorPtr internal_allocator)
        : internal_allocator_(internal_allocator), stats_list_(internal_allocator->Adapter())
    {
    }
    NO_MOVE_SEMANTIC(JITStats);
    NO_COPY_SEMANTIC(JITStats);
    ~JITStats()
    {
        DumpCsv();
    }
    void SetCompilationStart();
    void EndCompilationWithStats(const std::string &method_name, bool is_osr, size_t bc_size, size_t code_size);
    void ResetCompilationStart();

private:
    void DumpCsv(char sep = ',');
    struct Entry {
        PandaString method_name;
        bool is_osr;
        size_t bc_size;
        size_t code_size;
        uint64_t time;
    };

private:
    mem::InternalAllocatorPtr internal_allocator_;
    uint64_t start_time_ {};
    std::vector<Entry, typename mem::AllocatorAdapter<Entry>> stats_list_;
};

bool JITCompileMethod(RuntimeInterface *runtime, Method *method, bool is_osr, CodeAllocator *code_allocator,
                      ArenaAllocator *allocator, ArenaAllocator *local_allocator,
                      ArenaAllocator *gdb_debug_info_allocator, JITStats *jit_stats);
bool CompileInGraph(RuntimeInterface *runtime, Method *method, bool is_osr, ArenaAllocator *allocator,
                    ArenaAllocator *local_allocator, bool is_dynamic, Arch *arch, const std::string &method_name,
                    Graph **graph, JITStats *jit_stats = nullptr);
bool CheckMethodInLists(const std::string &method_name);
}  // namespace panda::compiler

#endif  // COMPILER_COMPILE_METHOD_H_

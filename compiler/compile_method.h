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

bool JITCompileMethod(RuntimeInterface *runtime, Method *method, bool is_osr, CodeAllocator *code_allocator,
                      ArenaAllocator *allocator, ArenaAllocator *local_allocator,
                      ArenaAllocator *gdb_debug_info_allocator);
bool CompileInGraph(RuntimeInterface *runtime, Method *method, bool is_osr, ArenaAllocator *allocator,
                    ArenaAllocator *local_allocator, bool is_dynamic, Arch *arch, const std::string &method_name,
                    Graph **graph);
bool CheckMethodInLists(const std::string &method_name);
}  // namespace panda::compiler

#endif  // COMPILER_COMPILE_METHOD_H_

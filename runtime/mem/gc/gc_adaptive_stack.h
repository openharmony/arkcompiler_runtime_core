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
#ifndef PANDA_RUNTIME_MEM_GC_GC_ADAPTIVE_STACK_H
#define PANDA_RUNTIME_MEM_GC_GC_ADAPTIVE_STACK_H

#include "runtime/mem/gc/gc_root_type.h"
#include "runtime/include/mem/panda_containers.h"

namespace panda::mem {

class GC;

enum class GCWorkersTaskTypes {
    TASK_EMPTY,
    TASK_MARKING,
    TASK_REMARK,
    TASK_REGION_COMPACTING,
    TASK_ROOT_COLLECTION,
    TASK_MOVE_YOUNG_ALIVE_OBJECTS,
    TASK_INIT_REFS_FROM_REMSETS,
};

/*
 * Adaptive stack with GC workers support.
 * It will try to pop objects from the source stack and push
 * it to the destination stack. if the destination stack reaches the limit,
 * we will create a new task for worker.
 * If stack limit is equal to zero, it means that the destination stack is unlimited.
 */
class GCAdaptiveStack {
public:
    explicit GCAdaptiveStack(GC *gc, size_t stack_size_limit = 0, size_t new_task_stack_size_limit = 0,
                             GCWorkersTaskTypes task = GCWorkersTaskTypes::TASK_EMPTY,
                             PandaStack<const ObjectHeader *> *stack_src = nullptr);

    ~GCAdaptiveStack();
    NO_COPY_SEMANTIC(GCAdaptiveStack);
    DEFAULT_MOVE_SEMANTIC(GCAdaptiveStack);

    /**
     * This method should be used when we find new object by field from another object.
     * @param from_object from which object we found object by reference, nullptr for roots
     * @param object object which will be added to the stack
     */
    void PushToStack(const ObjectHeader *from_object, const ObjectHeader *object);

    /**
     * This method should be used when we find new object as a root
     * @param root_type type of the root which we found
     * @param object object which will be added to the stack
     */
    void PushToStack(RootType root_type, const ObjectHeader *object);

    /**
     * \brief Pop an object from source stack.
     * If the source stack is empty, we will swap it with destination stack
     * and return an object from it.
     */
    const ObjectHeader *PopFromStack();
    /**
     * \brief Check that destination or source stack has at least one object.
     */
    bool Empty();
    /**
     * \brief Returns the sum of destination and source stacks sizes.
     */
    size_t Size();

    /**
     * \brief Set the src stack pointer to nullptr.
     * Should be used if we decide to free it not on destructor of this instance.
     */
    PandaStack<const ObjectHeader *> *MoveStacksPointers();

    /**
     * \brief Returns true if stack supports parallel workers, false otherwise
     */
    bool IsWorkersTaskSupported();

    /**
     * \brief Remove all elements from stack
     */
    void Clear();

private:
    /**
     * \brief Add new object into destination stack.
     * If we set the limit for stack, we will create a new task for
     * GC workers with it.
     */
    void PushToStack(const ObjectHeader *element);

private:
    PandaStack<const ObjectHeader *> *stack_src_;
    PandaStack<const ObjectHeader *> *stack_dst_;
    size_t stack_size_limit_ {0};
    size_t initial_stack_size_limit_ {0};
    size_t new_task_stack_size_limit_ {0};
    GCWorkersTaskTypes task_type_;
    GC *gc_;
};

}  // namespace panda::mem

#endif  // PANDA_RUNTIME_MEM_GC_GC_H

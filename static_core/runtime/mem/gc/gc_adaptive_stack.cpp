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

#include "runtime/mem/gc/gc_adaptive_stack.h"
#include "runtime/mem/gc/gc.h"
#include "runtime/mem/gc/workers/gc_workers_task_pool.h"

namespace panda::mem {

GCAdaptiveStack::GCAdaptiveStack(GC *gc, size_t stack_size_limit, size_t new_task_stack_size_limit,
                                 GCWorkersTaskTypes task, uint64_t time_limit_for_new_task_creation,
                                 PandaDeque<ObjectHeader *> *stack_src)
    : stack_size_limit_(stack_size_limit),
      new_task_stack_size_limit_(new_task_stack_size_limit),
      time_limit_for_new_task_creation_(time_limit_for_new_task_creation),
      task_type_(task),
      gc_(gc)
{
    initial_stack_size_limit_ = stack_size_limit_;
    auto allocator = gc_->GetInternalAllocator();
    ASSERT((stack_size_limit == 0) || (gc->GetWorkersPool() != nullptr));
    if (stack_src != nullptr) {
        stack_src_ = stack_src;
    } else {
        stack_src_ = allocator->template New<PandaDeque<ObjectHeader *>>(allocator->Adapter());
    }
    stack_dst_ = allocator->template New<PandaDeque<ObjectHeader *>>(allocator->Adapter());
}

GCAdaptiveStack::~GCAdaptiveStack()
{
    gc_->GetInternalAllocator()->Delete(stack_src_);
    gc_->GetInternalAllocator()->Delete(stack_dst_);
}

bool GCAdaptiveStack::Empty()
{
    return stack_src_->empty() && stack_dst_->empty();
}

size_t GCAdaptiveStack::Size()
{
    return stack_src_->size() + stack_dst_->size();
}

PandaDeque<ObjectHeader *> *GCAdaptiveStack::MoveStacksPointers()
{
    ASSERT(stack_src_ != nullptr);
    PandaDeque<ObjectHeader *> *return_value = stack_src_;
    stack_src_ = nullptr;
    return return_value;
}

void GCAdaptiveStack::PushToStack(const ObjectHeader *from_object, ObjectHeader *object)
{
    LOG(DEBUG, GC) << "Add object to stack: " << GetDebugInfoAboutObject(object)
                   << " accessed from object: " << from_object;
    ValidateObject(from_object, object);
    PushToStack(object);
}

void GCAdaptiveStack::PushToStack(RootType root_type, ObjectHeader *object)
{
    LOG(DEBUG, GC) << "Add object to stack: " << GetDebugInfoAboutObject(object)
                   << " accessed as a root: " << root_type;
    ValidateObject(root_type, object);
    PushToStack(object);
}

void GCAdaptiveStack::PushToStack(ObjectHeader *element)
{
    ASSERT_PRINT(IsAddressInObjectsHeap(element), element);
    if ((stack_size_limit_ > 0) && ((stack_dst_->size() + 1) == stack_size_limit_)) {
        // Try to create a new task only once
        // Create a new stack and send a new task to GC
        LOG(DEBUG, GC) << "GCAdaptiveStack: Try to add new task " << GCWorkersTaskTypesToString(task_type_)
                       << " for worker";
        ASSERT(gc_->GetWorkersPool() != nullptr);
        ASSERT(task_type_ != GCWorkersTaskTypes::TASK_EMPTY);
        auto allocator = gc_->GetInternalAllocator();
        // New tasks will be created with the same new_task_stack_size_limit_ and stack_size_limit_
        auto *new_stack = allocator->New<GCAdaptiveStack>(gc_, new_task_stack_size_limit_, new_task_stack_size_limit_,
                                                          task_type_, time_limit_for_new_task_creation_, stack_dst_);
        if (gc_->GetWorkersPool()->AddTask(GCMarkWorkersTask(task_type_, new_stack))) {
            LOG(DEBUG, GC) << "GCAdaptiveStack: Successfully add new task " << GCWorkersTaskTypesToString(task_type_)
                           << " for worker";
            stack_dst_ = allocator->template New<PandaDeque<ObjectHeader *>>(allocator->Adapter());
        } else {
            // We will try to create a new task later
            stack_size_limit_ += stack_size_limit_;
            LOG(DEBUG, GC) << "GCAdaptiveStack: Failed to add new task " << GCWorkersTaskTypesToString(task_type_)
                           << " for worker";
            [[maybe_unused]] auto src_stack = new_stack->MoveStacksPointers();
            ASSERT(src_stack == stack_dst_);
            allocator->Delete(new_stack);
        }
        if (IsHighTaskCreationRate()) {
            stack_size_limit_ += stack_size_limit_;
        }
    }
    stack_dst_->push_back(element);
}

bool GCAdaptiveStack::IsHighTaskCreationRate()
{
    if (created_tasks_ == 0) {
        start_time_ = panda::os::time::GetClockTimeInThreadCpuTime();
    }
    created_tasks_++;
    if (tasks_for_time_check_ == created_tasks_) {
        uint64_t cur_time = panda::os::time::GetClockTimeInThreadCpuTime();
        ASSERT(cur_time >= start_time_);
        uint64_t total_time_consumed = cur_time - start_time_;
        uint64_t one_task_consumed = total_time_consumed / created_tasks_;
        LOG(DEBUG, GC) << "Created " << created_tasks_ << " tasks in " << Timing::PrettyTimeNs(total_time_consumed);
        if (one_task_consumed < time_limit_for_new_task_creation_) {
            created_tasks_ = 0;
            start_time_ = 0;
            return true;
        }
    }
    return false;
}

ObjectHeader *GCAdaptiveStack::PopFromStack()
{
    if (stack_src_->empty()) {
        ASSERT(!stack_dst_->empty());
        auto temp = stack_src_;
        stack_src_ = stack_dst_;
        stack_dst_ = temp;
        if (stack_size_limit_ != 0) {
            // We may increase the current limit, so return it to the default value
            stack_size_limit_ = initial_stack_size_limit_;
        }
    }
    ASSERT(!stack_src_->empty());
    ObjectHeader *element = stack_src_->back();
    stack_src_->pop_back();
    return element;
}

GCAdaptiveStack::MarkedObjects GCAdaptiveStack::TraverseObjects(const GCAdaptiveStack::ObjectVisitor &visitor)
{
    // We need this to avoid allocation of new stack and fragmentation
    static constexpr size_t BUCKET_SIZE = 16;
    auto allocator = gc_->GetInternalAllocator();
    MarkedObjects marked_objects;
    PandaDeque<ObjectHeader *> *tail_marked_objects =
        allocator->template New<PandaDeque<ObjectHeader *>>(allocator->Adapter());
    if (stack_src_->empty()) {
        std::swap(stack_src_, stack_dst_);
    }
    while (!Empty()) {
        [[maybe_unused]] auto stack_src_size = stack_src_->size();
        for (auto *object : *stack_src_) {
            visitor(object);
            // visitor mustn't pop from stack
            ASSERT(stack_src_size == stack_src_->size());
        }
        if (LIKELY(stack_src_->size() > BUCKET_SIZE)) {
            marked_objects.push_back(stack_src_);
            stack_src_ = allocator->template New<PandaDeque<ObjectHeader *>>(allocator->Adapter());
        } else {
            tail_marked_objects->insert(tail_marked_objects->end(), stack_src_->begin(), stack_src_->end());
            stack_src_->clear();
        }
        std::swap(stack_src_, stack_dst_);
    }
    if (!tail_marked_objects->empty()) {
        marked_objects.push_back(tail_marked_objects);
    } else {
        allocator->Delete(tail_marked_objects);
    }
    return marked_objects;
}

bool GCAdaptiveStack::IsWorkersTaskSupported()
{
    return task_type_ != GCWorkersTaskTypes::TASK_EMPTY;
}

void GCAdaptiveStack::Clear()
{
    *stack_src_ = PandaDeque<ObjectHeader *>();
    *stack_dst_ = PandaDeque<ObjectHeader *>();
}

}  // namespace panda::mem

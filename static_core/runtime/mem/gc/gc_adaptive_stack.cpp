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

namespace ark::mem {

GCAdaptiveStack::GCAdaptiveStack(GC *gc, size_t stackSizeLimit, size_t newTaskStackSizeLimit, GCWorkersTaskTypes task,
                                 uint64_t timeLimitForNewTaskCreation, PandaDeque<ObjectHeader *> *stackSrc)
    : stackSizeLimit_(stackSizeLimit),
      newTaskStackSizeLimit_(newTaskStackSizeLimit),
      timeLimitForNewTaskCreation_(timeLimitForNewTaskCreation),
      taskType_(task),
      gc_(gc)
{
    initialStackSizeLimit_ = stackSizeLimit_;
    auto allocator = gc_->GetInternalAllocator();
    ASSERT((stackSizeLimit == 0) || (gc->GetWorkersTaskPool() != nullptr));
    if (stackSrc != nullptr) {
        stackSrc_ = stackSrc;
    } else {
        stackSrc_ = allocator->template New<PandaDeque<ObjectHeader *>>(allocator->Adapter());
    }
    stackDst_ = allocator->template New<PandaDeque<ObjectHeader *>>(allocator->Adapter());
}

GCAdaptiveStack::~GCAdaptiveStack()
{
    gc_->GetInternalAllocator()->Delete(stackSrc_);
    gc_->GetInternalAllocator()->Delete(stackDst_);
}

bool GCAdaptiveStack::Empty()
{
    return stackSrc_->empty() && stackDst_->empty();
}

size_t GCAdaptiveStack::Size()
{
    return stackSrc_->size() + stackDst_->size();
}

PandaDeque<ObjectHeader *> *GCAdaptiveStack::MoveStacksPointers()
{
    ASSERT(stackSrc_ != nullptr);
    PandaDeque<ObjectHeader *> *returnValue = stackSrc_;
    stackSrc_ = nullptr;
    return returnValue;
}

void GCAdaptiveStack::PushToStack(const ObjectHeader *fromObject, ObjectHeader *object)
{
    LOG(DEBUG, GC) << "Add object to stack: " << GetDebugInfoAboutObject(object)
                   << " accessed from object: " << fromObject;
    ValidateObject(fromObject, object);
    PushToStack(object);
}

void GCAdaptiveStack::PushToStack(RootType rootType, ObjectHeader *object)
{
    LOG(DEBUG, GC) << "Add object to stack: " << GetDebugInfoAboutObject(object) << " accessed as a root: " << rootType;
    ValidateObject(rootType, object);
    PushToStack(object);
}

void GCAdaptiveStack::PushToStack(ObjectHeader *element)
{
    ASSERT_PRINT(IsAddressInObjectsHeap(element), element);
    if ((stackSizeLimit_ > 0) && ((stackDst_->size() + 1) == stackSizeLimit_)) {
        // Try to create a new task only once
        // Create a new stack and send a new task to GC
        LOG(DEBUG, GC) << "GCAdaptiveStack: Try to add new task " << GCWorkersTaskTypesToString(taskType_)
                       << " for worker";
        ASSERT(gc_->GetWorkersTaskPool() != nullptr);
        ASSERT(taskType_ != GCWorkersTaskTypes::TASK_EMPTY);
        auto allocator = gc_->GetInternalAllocator();
        // New tasks will be created with the same new_task_stack_size_limit_ and stack_size_limit_
        auto *newStack = allocator->New<GCAdaptiveStack>(gc_, newTaskStackSizeLimit_, newTaskStackSizeLimit_, taskType_,
                                                         timeLimitForNewTaskCreation_, stackDst_);
        if (gc_->GetWorkersTaskPool()->AddTask(GCMarkWorkersTask(taskType_, newStack))) {
            LOG(DEBUG, GC) << "GCAdaptiveStack: Successfully add new task " << GCWorkersTaskTypesToString(taskType_)
                           << " for worker";
            stackDst_ = allocator->template New<PandaDeque<ObjectHeader *>>(allocator->Adapter());
        } else {
            // We will try to create a new task later
            stackSizeLimit_ += stackSizeLimit_;
            LOG(DEBUG, GC) << "GCAdaptiveStack: Failed to add new task " << GCWorkersTaskTypesToString(taskType_)
                           << " for worker";
            [[maybe_unused]] auto srcStack = newStack->MoveStacksPointers();
            ASSERT(srcStack == stackDst_);
            allocator->Delete(newStack);
        }
        if (IsHighTaskCreationRate()) {
            stackSizeLimit_ += stackSizeLimit_;
        }
    }
    stackDst_->push_back(element);
}

bool GCAdaptiveStack::IsHighTaskCreationRate()
{
    if (createdTasks_ == 0) {
        startTime_ = ark::os::time::GetClockTimeInThreadCpuTime();
    }
    createdTasks_++;
    if (tasksForTimeCheck_ == createdTasks_) {
        uint64_t curTime = ark::os::time::GetClockTimeInThreadCpuTime();
        ASSERT(curTime >= startTime_);
        uint64_t totalTimeConsumed = curTime - startTime_;
        uint64_t oneTaskConsumed = totalTimeConsumed / createdTasks_;
        LOG(DEBUG, GC) << "Created " << createdTasks_ << " tasks in " << Timing::PrettyTimeNs(totalTimeConsumed);
        if (oneTaskConsumed < timeLimitForNewTaskCreation_) {
            createdTasks_ = 0;
            startTime_ = 0;
            return true;
        }
    }
    return false;
}

ObjectHeader *GCAdaptiveStack::PopFromStack()
{
    if (stackSrc_->empty()) {
        ASSERT(!stackDst_->empty());
        auto temp = stackSrc_;
        stackSrc_ = stackDst_;
        stackDst_ = temp;
        if (stackSizeLimit_ != 0) {
            // We may increase the current limit, so return it to the default value
            stackSizeLimit_ = initialStackSizeLimit_;
        }
    }
    ASSERT(!stackSrc_->empty());
    ObjectHeader *element = stackSrc_->back();
    stackSrc_->pop_back();
    return element;
}

GCAdaptiveStack::MarkedObjects GCAdaptiveStack::TraverseObjects(const GCAdaptiveStack::ObjectVisitor &visitor)
{
    // We need this to avoid allocation of new stack and fragmentation
    static constexpr size_t BUCKET_SIZE = 16;
    auto allocator = gc_->GetInternalAllocator();
    MarkedObjects markedObjects;
    PandaDeque<ObjectHeader *> *tailMarkedObjects =
        allocator->template New<PandaDeque<ObjectHeader *>>(allocator->Adapter());
    if (stackSrc_->empty()) {
        std::swap(stackSrc_, stackDst_);
    }
    while (!Empty()) {
        [[maybe_unused]] auto stackSrcSize = stackSrc_->size();
        for (auto *object : *stackSrc_) {
            visitor(object);
            // visitor mustn't pop from stack
            ASSERT(stackSrcSize == stackSrc_->size());
        }
        if (LIKELY(stackSrc_->size() > BUCKET_SIZE)) {
            markedObjects.push_back(stackSrc_);
            stackSrc_ = allocator->template New<PandaDeque<ObjectHeader *>>(allocator->Adapter());
        } else {
            tailMarkedObjects->insert(tailMarkedObjects->end(), stackSrc_->begin(), stackSrc_->end());
            stackSrc_->clear();
        }
        std::swap(stackSrc_, stackDst_);
    }
    if (!tailMarkedObjects->empty()) {
        markedObjects.push_back(tailMarkedObjects);
    } else {
        allocator->Delete(tailMarkedObjects);
    }
    return markedObjects;
}

bool GCAdaptiveStack::IsWorkersTaskSupported()
{
    return taskType_ != GCWorkersTaskTypes::TASK_EMPTY;
}

void GCAdaptiveStack::Clear()
{
    *stackSrc_ = PandaDeque<ObjectHeader *>();
    *stackDst_ = PandaDeque<ObjectHeader *>();
}

}  // namespace ark::mem

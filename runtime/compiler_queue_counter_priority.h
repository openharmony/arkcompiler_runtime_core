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
#ifndef PANDA_RUNTIME_COMPILER_QUEUE_COUNTER_PRIORITY_H_
#define PANDA_RUNTIME_COMPILER_QUEUE_COUNTER_PRIORITY_H_

#include <algorithm>
#include <cstring>

#include "libpandabase/utils/time.h"
#include "runtime/compiler_queue_interface.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/method-inl.h"

namespace panda {

class CompilationQueueElement {
public:
    explicit CompilationQueueElement(CompilerTask task) : context_(task)
    {
        timestamp_ = time::GetCurrentTimeInMillis();
        counter_ = task.GetMethod()->GetHotnessCounter();
    }

    uint64_t GetCounter() const
    {
        // Do not return method->counter as it can changed during sorting
        return counter_;
    }

    const CompilerTask &GetContext() const
    {
        return context_;
    }

    CompilerTask &GetContext()
    {
        return context_;
    }

    uint64_t GetTimestamp() const
    {
        return timestamp_;
    }

    void UpdateCounter(uint64_t new_counter)
    {
        counter_ = new_counter;
    }

private:
    uint64_t timestamp_;
    uint64_t counter_;
    CompilerTask context_;
};

/** The counter priority queue sorts all tasks (methods) by its hotness counters.
 *  It extracts the most hot method for compilation (the greater counter).
 *  It also has a limit of tasks for compilation. If the task is too old, it is expired and removed from the queue.
 *  Note, in case of skip the method due to length limit, its hotness counter is reset.
 *  Max length and life time is configured.
 *  This queue is thread unsafe (should be used under lock).
 */
class CompilerPriorityCounterQueue : public CompilerQueueInterface {
public:
    explicit CompilerPriorityCounterQueue(mem::InternalAllocatorPtr allocator, uint64_t max_length,
                                          uint64_t task_life_span)
        : allocator_(allocator), queue_(allocator->Adapter())
    {
        max_length_ = max_length;
        task_life_span_ = task_life_span;
        queue_name_ = "";
        SetQueueName("priority counter compilation queue");
    }

    CompilerTask GetTask() override
    {
        UpdateQueue();
        if (queue_.empty()) {
            LOG(DEBUG, COMPILATION_QUEUE) << "Empty " << queue_name_ << ", return nothing";
            return CompilerTask();
        }
        sort(queue_.begin(), queue_.end(), comparator_);
        auto element = queue_.back();
        auto task = element->GetContext();
        queue_.pop_back();
        allocator_->Delete(element);
        LOG(DEBUG, COMPILATION_QUEUE) << "Extract a task from a " << queue_name_ << ": " << GetTaskDescription(task);
        return task;
    }

    // NOLINTNEXTLINE(google-default-arguments)
    void AddTask(CompilerTask ctx, [[maybe_unused]] size_t priority = 0) override
    {
        UpdateQueue();
        if (queue_.size() >= max_length_) {
            // Not sure if it is possible to exceed the size more than one
            // Reset the counter of the rejected method;
            ctx.GetMethod()->ResetHotnessCounter();
            ctx.GetMethod()->AtomicSetCompilationStatus(Method::WAITING,
                                                        ctx.IsOsr() ? Method::COMPILED : Method::NOT_COMPILED);
            // Maybe, replace the other method in queue?
            LOG(DEBUG, COMPILATION_QUEUE) << "Skip adding the task " << GetTaskDescription(ctx)
                                          << " due to limit of tasks (" << max_length_ << ") in a " << queue_name_;
            return;
        }
        auto element = allocator_->New<CompilationQueueElement>(ctx);
        // Sorting will be in Get function
        queue_.push_back(element);
        LOG(DEBUG, COMPILATION_QUEUE) << "Add an element to a " << queue_name_ << ": " << GetTaskDescription(ctx);
    }

    void Finalize() override
    {
        for (auto e : queue_) {
            allocator_->Delete(e);
        }
        queue_.clear();
        LOG(DEBUG, COMPILATION_QUEUE) << "Clear a " << queue_name_;
    }

protected:
    size_t GetQueueSize() override
    {
        return queue_.size();
    }

    virtual bool UpdateCounterAndCheck(CompilationQueueElement *element)
    {
        // The only way to update counter
        element->UpdateCounter(element->GetContext().GetMethod()->GetHotnessCounter());
        uint64_t cur_stamp = time::GetCurrentTimeInMillis();
        return (cur_stamp - element->GetTimestamp() >= task_life_span_);
    }

    void SetQueueName(char const *name)
    {
        queue_name_ = name;
    }

private:
    class Comparator {
    public:
        bool operator()(CompilationQueueElement *a, CompilationQueueElement *b) const
        {
            if (a->GetCounter() == b->GetCounter()) {
                // Use method name just in case?
                if (a->GetTimestamp() == b->GetTimestamp()) {
                    // The only way is a name
                    // Again, as we pull from the end return reversed compare
                    return strcmp(reinterpret_cast<const char *>(a->GetContext().GetMethod()->GetName().data),
                                  reinterpret_cast<const char *>(b->GetContext().GetMethod()->GetName().data)) > 0;
                }
                // Low time is high priority (pull from the end)
                return a->GetTimestamp() > b->GetTimestamp();
            }
            // First, handle a method with higher hotness counter
            return (a->GetCounter() < b->GetCounter());
        }
    };

    void UpdateQueue()
    {
        // Remove expired tasks
        for (auto it = queue_.begin(); it != queue_.end();) {
            auto element = *it;
            // We should update the counter inside as the queue choose the semantic of the counter by itself
            if (UpdateCounterAndCheck(element)) {
                LOG(DEBUG, COMPILATION_QUEUE) << "Remove an expired element from a " << queue_name_ << ": "
                                              << GetTaskDescription(element->GetContext());
                auto ctx = element->GetContext();
                ctx.GetMethod()->AtomicSetCompilationStatus(Method::WAITING,
                                                            ctx.IsOsr() ? Method::COMPILED : Method::NOT_COMPILED);
                allocator_->Delete(element);
                it = queue_.erase(it);
            } else {
                ++it;
            }
        }
    }
    mem::InternalAllocatorPtr allocator_;
    PandaVector<CompilationQueueElement *> queue_;
    Comparator comparator_;
    uint64_t max_length_;
    // In milliseconds
    uint64_t task_life_span_;
    const char *queue_name_;
};

}  // namespace panda

#endif  // PANDA_RUNTIME_COMPILER_QUEUE_COUNTER_PRIORITY_H_

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

#include "lock_free_queue.h"

namespace panda::tooling::sampler {

void LockFreeQueue::Push(const FileInfo &data)
{
    Node *new_node = new Node(std::make_unique<FileInfo>(data), nullptr);

    for (;;) {
        // Atomic with acquire order reason: to sync with push in other threads
        Node *tail = tail_.load(std::memory_order_acquire);
        ASSERT(tail != nullptr);
        // Atomic with acquire order reason: to sync with push in other threads
        Node *next = tail->next.load(std::memory_order_acquire);
        // Atomic with acquire order reason: to sync with push in other threads
        Node *tail2 = tail_.load(std::memory_order_acquire);
        if (tail == tail2) {
            if (next == nullptr) {
                if (tail->next.compare_exchange_weak(next, new_node)) {
                    tail_.compare_exchange_strong(tail, new_node);
                    ++size_;
                    return;
                }
            } else {
                Node *new_tail = next;
                tail_.compare_exchange_strong(tail, new_tail);
            }
        }
    }
}

// NOLINTNEXTLINE(performance-unnecessary-value-param)
void LockFreeQueue::Pop(FileInfo &ret)
{
    for (;;) {
        // Atomic with acquire order reason: to sync with push in other threads
        Node *head = head_.load(std::memory_order_acquire);
        // Atomic with acquire order reason: to sync with push in other threads
        Node *tail = tail_.load(std::memory_order_acquire);
        // Atomic with acquire order reason: to sync with push in other threads
        Node *next = head->next.load(std::memory_order_acquire);
        // Atomic with acquire order reason: to sync with push in other threads
        Node *head2 = head_.load(std::memory_order_acquire);
        if (head == head2) {
            if (head == tail) {
                ASSERT(next != nullptr);
                Node *new_tail = next;
                tail_.compare_exchange_strong(tail, new_tail);
            } else {
                if (next == nullptr) {
                    continue;
                }
                ASSERT(next->data != nullptr);
                ret = *(next->data);
                Node *new_head = next;
                if (head_.compare_exchange_weak(head, new_head)) {
                    delete head;
                    --size_;
                    return;
                }
            }
        }
    }
}

bool LockFreeQueue::FindValue(uintptr_t data) const
{
    // Atomic with acquire order reason: to sync with push in other threads
    Node *head = head_.load(std::memory_order_acquire);
    // Atomic with acquire order reason: to sync with push in other threads
    Node *tail = tail_.load(std::memory_order_acquire);

    for (;;) {
        ASSERT(head != nullptr);
        // Should be only dummy Node, but keep that check
        if (head->data == nullptr) {
            if (head == tail) {
                // here all nodes were checked
                break;
            }
            head = head->next;
            continue;
        }

        if (head->data->ptr == data) {
            return true;
        }

        if (head == tail) {
            // here all nodes were checked
            break;
        }
        head = head->next;
    }

    return false;
}

}  // namespace panda::tooling::sampler

/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "common_components/heap/allocator/treap.h"

#include "common_interfaces/heap/region_desc.h"

namespace ark::common_vm {
bool Treap::MergeInsertInternal(uint32_t idx, uint32_t num, bool refreshRegionDesc)
{
    //     +-------------+       +--------------+
    //     | parent node |  n--> | current node |
    //     |             |       |              |
    // pn ---> TreapNode* l; -------> |              |
    //     |   TreapNode* r;  |       |              |
    //     +-------------+       +--------------+
    // suppose current node is parent node's left child, then
    // n points to the current node,
    // pn points to the 'l' field in the parent node
    TreapNode *current = root_;     // root is current node
    TreapNode **pCurrent = &root_;  // pointer to the 'root' field in this tree
    // stack of pn recording how to go from root to the current node
    LocalDeque<TreapNode **> pnStack(sud_);  // this uses another deque as container
    uint32_t m = idx + num;
    // this loop insert the new node (idx, num) at the proper place
    do {
        if (current == nullptr) {
            current = new (nodeAllocator_.Allocate()) TreapNode(idx, num, refreshRegionDesc);
            ASSERT_PRINT(current != nullptr, "fail to allocate a new node");
            *pCurrent = current;
            IncTotalCount(num);
            break;
        }
        MergeResult res = MergeAt(*current, idx, num, refreshRegionDesc);
        if (res == MergeResult::MERGE_SUCCESS) {
            break;
        } else if (UNLIKELY(res == MergeResult::MERGE_ERROR)) {
            return false;
        }
        // MergeResult::MERGE_MISS: (idx, num) cannot be connected to n
        if (m < current->GetIndex()) {
            // should insert the node into left subtree
            pnStack.Push(pCurrent);
            pCurrent = &(current->l);
            current = current->l;
        } else if (idx > current->GetIndex() + current->GetCount()) {
            // should insert the node into right subtree
            pnStack.Push(pCurrent);
            pCurrent = &(current->r);
            current = current->r;
        } else {
            // something clashes
            ASSERT_PRINT(false, "merge insertion failed");
            return false;
        }
    } while (true);

    // this loop bubbles the inserted node up the tree to satisfy heap property
    while (!pnStack.Empty()) {
        pCurrent = pnStack.Top();
        pnStack.Pop();
        current = *pCurrent;
        ASSERT_PRINT(current, "merge insertion bubbling failed case 1");
        if (m < current->GetIndex()) {
            // (idx, num) was inserted into n's left subtree, do rotate l, if needed
            if (current->GetCount() < current->l->GetCount()) {
                *pCurrent = RotateLeftChild(*current);
                CTREE_CHECK_PARENT_AND_RCHILD(*pCurrent);
            } else {
                break;
            }
        } else if (idx > current->GetIndex() + current->GetCount()) {
            // (idx, num) was inserted into n's right subtree, do rotate r, if needed
            if (current->GetCount() < current->r->GetCount()) {
                *pCurrent = RotateRightChild(*current);
                CTREE_CHECK_PARENT_AND_LCHILD(*pCurrent);
            } else {
                break;
            }
        } else {
            ASSERT_PRINT(false, "merge insertion bubbling failed: case 2");
            return false;
        }
    }
    return true;
}

#ifdef DEBUG_CARTESIAN_TREE
void Treap::DumpTree(const char *msg) const
{
    if (Empty()) {
        return;
    }

    LOG(DEBUG, GC) << "dump " << msg << " " << this << " in graphviz .dot:";
    LOG(DEBUG, GC) << "digraph tree" << this << " {";
    Treap::Iterator it(*const_cast<Treap *>(this));
    auto node = it.Next();
    while (node != nullptr) {
        LOG(DEBUG, GC) << "c-tree " << this << " N" << node << " [label=\"" << node << ":" << node->GetIndex() << "+"
                       << node->GetCount() << "=" << node->GetIndex() + node->GetCount() << "\"]";

        if (node->l != nullptr) {
            LOG(DEBUG, GC) << "c-tree " << this << " N" << node << " -> N" << node->l;
        }

        LOG(DEBUG, GC) << "c-tree " << this << " N" << node << " -> D" << node << " [style=invis]";
        LOG(DEBUG, GC) << "c-tree " << this << " D" << node << " [width=0, style=invis]";

        if (node->r != nullptr) {
            LOG(DEBUG, GC) << "c-tree " << this << " N" << node << " -> N" << node->r;
        }

        node = it.Next();
    }
    LOG(DEBUG, GC) << "}";
}
#endif

void Treap::TreapNode::RefreshFreeRegionDesc()
{
    uint32_t idx = GetIndex();
    uint32_t cnt = GetCount();
    RegionDesc::InitFreeRegion(idx, cnt);
}

void Treap::TreapNode::ReleaseMemory()
{
    uint32_t idx = GetIndex();
    uint32_t cnt = GetCount();
    RegionDesc::ReleaseUnits(idx, cnt);
}
}  // namespace ark::common_vm

/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd.
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

#include "runtime/include/panda_vm.h"

#include "runtime/lock_order_graph.h"

namespace panda {
void UpdateMonitorsForThread(PandaMap<ManagedThread::ThreadId, Monitor::MonitorId> &entering_monitors,
                             PandaMap<Monitor::MonitorId, PandaSet<ManagedThread::ThreadId>> &entered_monitors,
                             MTManagedThread *thread)
{
    auto thread_id = thread->GetId();
    auto entering_monitor = thread->GetEnteringMonitor();
    if (entering_monitor != nullptr) {
        entering_monitors[thread_id] = entering_monitor->GetId();
    }
    for (auto entered_monitor_id : thread->GetVM()->GetMonitorPool()->GetEnteredMonitorsIds(thread)) {
        entered_monitors[entered_monitor_id].insert(thread_id);
    }
}

bool LockOrderGraph::CheckForTerminationLoops(const PandaList<MTManagedThread *> &threads,
                                              const PandaList<MTManagedThread *> &daemon_threads,
                                              MTManagedThread *current)
{
    PandaMap<ThreadId, bool> nodes;
    PandaMap<ThreadId, ThreadId> edges;
    PandaMap<ThreadId, MonitorId> entering_monitors;
    PandaMap<MonitorId, PandaSet<ThreadId>> entered_monitors;
    for (auto thread : threads) {
        if (thread == current) {
            continue;
        }

        auto thread_id = thread->GetId();
        auto status = thread->GetStatus();
        if (status == ThreadStatus::NATIVE) {
            nodes[thread_id] = true;
        } else {
            if (status != ThreadStatus::IS_BLOCKED) {
                LOG(DEBUG, RUNTIME) << "Thread " << thread_id << " has changed its status during graph construction";
                return false;
            }
            nodes[thread_id] = false;
        }
        LOG(DEBUG, RUNTIME) << "LockOrderGraph node: " << thread_id << ", is NATIVE = " << nodes[thread_id];
        UpdateMonitorsForThread(entering_monitors, entered_monitors, thread);
    }
    for (auto thread : daemon_threads) {
        auto thread_id = thread->GetId();
        nodes[thread_id] = true;
        LOG(DEBUG, RUNTIME) << "LockOrderGraph node: " << thread_id << ", in termination loop";
        UpdateMonitorsForThread(entering_monitors, entered_monitors, thread);
    }

    for (const auto &[from_thread_id, entering_monitor_id] : entering_monitors) {
        for (const auto to_thread_id : entered_monitors[entering_monitor_id]) {
            // We can only wait for a single monitor here.
            if (edges.count(from_thread_id) != 0) {
                LOG(DEBUG, RUNTIME) << "Graph has been changed during its construction. Previous edge "
                                    << from_thread_id << " -> " << edges[from_thread_id]
                                    << " cannot be overwritten with " << from_thread_id << " -> " << to_thread_id;
                return false;
            }
            edges[from_thread_id] = to_thread_id;
            LOG(DEBUG, RUNTIME) << "LockOrderGraph edge: " << from_thread_id << " -> " << to_thread_id;
        }
    }
    return LockOrderGraph(nodes, edges).CheckForTerminationLoops();
}

bool LockOrderGraph::CheckForTerminationLoops() const
{
    // This function returns true, if the following conditions are satisfied for each node:
    // the node belongs to a loop (i.e., there is a deadlock with corresponding threads), or
    // this is a terminating node (thread with NATIVE status), or
    // there is a path to a loop or to a terminating node.
    PandaSet<ThreadId> nodes_in_deadlocks = {};
    for (auto const node_elem : nodes_) {
        auto node = node_elem.first;
        if (nodes_in_deadlocks.count(node) != 0) {
            // If this node belongs to some previously found loop, we ignore it.
            continue;
        }
        if (nodes_.at(node)) {
            // This node is terminating, ignore it.
            nodes_in_deadlocks.insert(node);
            continue;
        }

        // explored_nodes contains nodes reachable from the node chosen in the outer loop.
        PandaSet<ThreadId> explored_nodes = {node};
        // front contains nodes which have not been explored yet.
        PandaList<ThreadId> front = {node};
        // On each iteration of the loop we take next unexplored node from the front and find all reachable nodes from
        // it. If we find already explored node then there is a loop and we save it in nodes_in_deadlocks. Also we
        // detect paths leading to nodes_in_deadlocks and to termination nodes.
        while (!front.empty()) {
            auto i = front.begin();
            while (i != front.end()) {
                ThreadId current_node = *i;
                i = front.erase(i);
                if (edges_.count(current_node) == 0) {
                    // No transitions from this node.
                    continue;
                }
                auto next_node = edges_.at(current_node);
                // There is a rare case, in which a monitor may be entered recursively in a
                // daemon thread. If a runtime calls DeregisterSuspendedThreads exactly when
                // the daemon thread sets SetEnteringMonitor, then we create an edge from a thread
                // to itself, i.e. a self-loop and, thus, falsely flag this situation as a deadlock.
                // So here we ignore this self-loop as a false loop.
                if (next_node == current_node) {
                    continue;
                }
                if (explored_nodes.count(next_node) != 0 || nodes_in_deadlocks.count(next_node) != 0 ||
                    nodes_.at(next_node)) {
                    // Loop or path to another loop or to terminating node was found
                    nodes_in_deadlocks.merge(explored_nodes);
                    front.clear();
                    break;
                }
                explored_nodes.insert(next_node);
                front.push_back(next_node);
            }
        }
    }
    return nodes_in_deadlocks.size() == nodes_.size();
}
}  // namespace panda

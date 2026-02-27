/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

package std.containers;

export namespace containers.stackless {

type CallbackT = (arg: undefined) => void;
type OptionalNode = Node | undefined;

class Node {
    constructor(resolver: CallbackT) {
        this.resolver = resolver;
    }
    public static exchange(oldNode: OptionalNode, newNode: OptionalNode): OptionalNode {
        let tmpNode = oldNode;
        oldNode = newNode;
        return tmpNode;
    }

    public resolver: CallbackT;
    public next: OptionalNode = undefined;
}

class WaitersList {
    constructor() {
        this.tail_ = new AtomicReference<OptionalNode>(undefined);
    }

    /**
    * creates promise and pushes to the queue
    * caller should await on this promise
    */
    public async suspend(): Promise<void> {
        let pResolver : CallbackT | undefined = undefined;
        let promise = new Promise<void>((resolve) => {
            pResolver = resolve;
        })
        let node = new Node(pResolver!);
        node.next = this.tail_.load();
        while (this.tail_.compareAndSwap(node.next, node) != node.next) {
            node.next = this.tail_.load();
        }
        return promise;
    }

    /**
    * resolves promise and pops from the queue
    */
    public resume(): void {
        while (this.head_ == undefined && !this.restockQueue()) {
        }
        let node = this.head_;
        this.head_ = this.head_!.next;
        node!.resolver(undefined);
    }

    /**
    * takes list from tail_, reverses it and puts it into head_
    */
    private restockQueue(): boolean {
        let tail = this.tail_.exchange(undefined);
        while (tail != undefined) {
            let newHead = tail;
            tail = tail!.next;
            newHead.next = this.head_;
            this.head_ = newHead;
        }
        return this.head_ != undefined;
    }

    private tail_ : AtomicReference<OptionalNode>;
    private head_ : OptionalNode = undefined;
}

}

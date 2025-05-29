/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

/*---
desc: Issue 21379. Issues with Partial
tags: []
---*/


interface Filler {
    push(value: long)
}

class StaticWorker0 implements Filler {
    private tasks: BlockingQueue<long>;
    constructor () {
        this.tasks = new LinkedBlockingQueue<long>()
    }

    push(value: long) {
        this.tasks.push(value)
    }
}

class StaticWorker1 implements Filler {
    private tasks: BlockingQueue<long> = new LinkedBlockingQueue<long>()
    push(value: long) {
        this.tasks.push(value)
    }
}

function main() {
    let worker0: Filler = new StaticWorker0
    worker0.push(42)
    let worker1: Filler = new StaticWorker1
    worker1.push(42)
}


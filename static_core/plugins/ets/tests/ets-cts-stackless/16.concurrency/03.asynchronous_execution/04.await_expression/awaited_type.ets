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

{% for c in cases %}
/*---
desc: Awaited type of resolved Promise<{{c.type}}>.
---*/

const value: {{c.type}} = {{c.value}}

// Promise resolve
async function returnsPromise(): Promise<{{c.type}}> {
    return value;
}

async function startTest(){
    const promiseResult = await returnsPromise();
    arktest.assertTrue(promiseResult instanceof Awaited<{{c.type}}>);
}

function main() {
    startTest().catch(arktest.crashProgram);
}
{% endfor %}
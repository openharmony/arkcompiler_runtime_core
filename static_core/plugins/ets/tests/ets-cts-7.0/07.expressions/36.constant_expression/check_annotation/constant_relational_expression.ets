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
desc: >-
    Constant expression with relational/equality operator as annotation initializer.
---*/

@interface MyAnno {
    id: boolean
    idN: boolean
}

const a: {{c.type}} = {{c.left}}
const b: {{c.type}} = {{c.right}}
const myId: boolean = a {{c.op}} b
const myIdN: boolean = {{c.expected}}

@MyAnno({id: myId, idN: myIdN})
class NewClass{}

function main() {
    arktest.assertEQ(myId, myIdN)
}

{% endfor %}

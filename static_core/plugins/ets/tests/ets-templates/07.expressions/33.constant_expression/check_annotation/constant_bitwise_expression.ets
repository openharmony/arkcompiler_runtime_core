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

{% for c in cases %}
/*---
desc: 
    Constant expression with number conversion as annotation initializer.
tags: [compile-only]
---*/

@interface MyAnno {
    id: int
    idN: number
}

const value: {{c.type}} = {{c.value}}
const myId: {{c.type}} = (value {{c.operation}}) {{c.asChar}}

@MyAnno({id: myId{{c.asInt}}, idN: myId})
class NewClass{}

{% endfor %}

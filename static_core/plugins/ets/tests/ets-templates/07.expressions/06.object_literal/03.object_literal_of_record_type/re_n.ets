/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
    Accessing a non-existent key in a Record with a generic key type
    returns undefined and throws NullPointerError when forcefully unwrapped
---*/

{{c.decl}}

function main() {
    {{c.use|indent}}
    let expectedException = (e: Error): boolean => {return (e instanceof NullPointerError)}
    arktest.expectThrow(() => { {{c.call}} }, expectedException)
}
{% endfor %}

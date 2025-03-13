/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
desc: Correct int literals with bases 16, 10, 8 and 2
---*/

{% for c in cases %}
let v{{loop.index}} = {{c['lit']}}
let b{{loop.index}} = new Int({{c['lit']}})
{% endfor %}

function main(): void {
{% for c in cases %}
    assertEQ(v{{loop.index}}, {{c['val']}})
    assertEQ(b{{loop.index}}.intValue(), {{c['val']}})
{% endfor %}
}

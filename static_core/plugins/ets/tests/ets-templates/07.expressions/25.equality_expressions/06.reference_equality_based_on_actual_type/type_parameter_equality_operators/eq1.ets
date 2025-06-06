/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
{%- for c in cases %}

/*---
desc: A value of type Object can be compared to a value of any reference type.
---*/

function eq<P>(p: P, q: Object, ps: string, qs: string): string {
    if (p == q && q == p) {
        return ''
    } else {
        return '\n' + ps + ' == ' + qs + ', expected true, got false'
    }
}
function ne<P>(p: P, q: Object, ps: string, qs: string): string {
    if (p != q || q != p) {
        return '\n' + ps + ' != ' + qs + ', expected false, got true'
    } else {
        return ''
    }
}

{{c.decl}}

function main() {
    let failed: string = ''
{%- for p in c['case1'] %}
{%- for q in c['case2'] %}
    failed += eq<{{c.type}}>({{p}}, {{q}}, "{{p|replace('\\', '\\\\')}}", "{{q|replace('\\', '\\\\')}}")
    failed += ne<{{c.type}}>({{p}}, {{q}}, "{{p|replace('\\', '\\\\')}}", "{{q|replace('\\', '\\\\')}}")
{%- endfor %}
{%- endfor %}
    assertTrue(failed == '', 'Failed operations: ' + failed)
}
{%- endfor %}

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
assertion: If an overload set is formed from explicit overloads only, then the order 
           of the overload set is the same as the order of the entities listed.
desc: Overload Resolution for Overload Declarations - explicit function overload with Any/Object types.
---*/

function singleArgAny(a: Any): int { return 1 }

function singleArgObject(a: Object): int { return 2 }

function multipleArgsAny(a: double, b: Any, c: string): int { return 1 }

function multipleArgsObject(a: double, b: Object, c: string): int { return 2 }

function optionalArgAny(a: double, b?: Any): int { return 1 }

function optionalArgObject(a: double, b?: Object): int { return 2 }

overload foo { {{c.overloadList}} };

function main() {
    {% for call in c.calls %}
    arktest.assertEQ({{call.call}}, {{call.result}})
    {% endfor %}
}

{% endfor %}

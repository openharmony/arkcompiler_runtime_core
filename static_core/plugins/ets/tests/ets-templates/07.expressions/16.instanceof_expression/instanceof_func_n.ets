/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
{% for t in c['rhs_types'] %}
{% set rt = c.func_type %}
{% set
response = "Syntax error ESY18871: Type parameter is erased from type '" ~ rt ~ "' when used in instanceof expression."
%}

/*---
desc: >-
    Check instanceof behaviour for function types with type erasure (15.12).
    Parameter types are erased to Any, return type to never.
    Signature structure (mandatory/optional/rest pattern) allows subtype to
    have less or equal number of required parameters and less or equal number of
    (required + optional) parameters, and requires supertype to have rest parameter
    if subtype has it. instanceof checks match by function subtyping
tags: [compile-only, negative]
expected_out: "{{response}}"
---*/

{{c.decl or ""}}

type T = {{c.func_type}}
let func: T
func = {{c.func}}

function main() {
    let probe = func instanceof {{t.type}}
}
{% endfor %}
{% endfor %}

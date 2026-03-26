/*---
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
---*/

{% for case in cases %}

/*---
desc: >-
    If no constraint is specified, then the constraint is Type Any, i.e., the lacking 
    explicit constraint effectively means extends Any. As a consequence, the type parameter 
    is not compatible with Type Object, and has neither methods nor fields available for use.
    If type parameter T has type constraint S, then the actual type of the generic 
    instantiation must be a subtype of S (see Subtyping). If the constraint S is a 
    non-nullish type (see Nullish Types), then T is also non-nullish.
    {{case.desc}}
tags: [compile-only, negative]
expected_out: {{ case.expected_error | tojson }}
---*/

{{case.code}}

{% endfor %}

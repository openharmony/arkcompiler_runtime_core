/*---
Copyright (c) 2025-2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

{% for c in cases %}
/*---
desc: Ambient Namespaces usage - mismatching decl and impl files
files: ['./namespace_declarations_n_{{loop.index0}}.ets']
tags: ['negative']
{% set ark = ['runtime-with-old-libs.ark', 'runtime-with-new-libs.ark'] -%}
{% set verifier = ['compile-old-libs.verifier', 'verify-new-libs'] -%}
{% if c.runtime_only is defined -%}
{% set steps = ark -%}
{% else -%}
{% set steps = verifier + ark -%}
{% endif -%}
negative_steps: {{steps}}
expected_error:
{% for step in steps -%}
{% set err = (c.err2 if c.err2 is defined else c.err) if step in ark else c.err %}
  {{step}}:{% for line in err %}
    - "{{line}}"
{% endfor %}{% endfor -%}
---*/

import { {{c.import}} } from './namespace_decl_d/namespace_declarations_n_{{loop.index0}}.d'

{{c.decl}}

function usage() {
  {{c.use|indent}}
}

function main() {
  usage()
}

{% endfor %}

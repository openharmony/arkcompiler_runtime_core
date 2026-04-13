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
desc: Ambient declarations with imports - mismatching decl and impl files
files: ['./declarations_n.ets']
tags: [negative]
negative_steps: [compile-old-libs.verifier, runtime-with-old-libs.ark, verify-new-libs, runtime-with-new-libs.ark]
expected_error:
    compile-old-libs.verifier: |-
      {{c.err}}
    runtime-with-old-libs.ark: |-
      {{c.err}}
    verify-new-libs: |-
      {{c.err}}
    runtime-with-new-libs.ark: |-
      {{c.err}}
---*/

{{c.decl}}

function usage() {
  {{c.use|indent}}
}

function main() {
  usage()
}

{% endfor %}

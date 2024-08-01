/*---
Copyright (c) 2024 Huawei Device Co., Ltd.
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
desc: >-
    When declared in a scope, names of local classes and interfaces have access to entities visible within this scope,
    and capture the entities they use from this scope.
    Function/method parameters and local variables can be used and thus captured.
---*/

{{c.decl}}

{{c.check}}

{% endfor %}

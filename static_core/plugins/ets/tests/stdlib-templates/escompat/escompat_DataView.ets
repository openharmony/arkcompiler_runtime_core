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

{% for item in escompat_DataView_instance %}
/*---
desc:  {function: instance_{{.item.object_type}}_setget_{{.item.target_element_type}}}
---*/

{% include 'utils/test_escompat_dataview.j2' %}

{% endfor %}

{% for item in escompat_DataView_method %}
/*---
desc:  {function: instance_{{.item.object_type}}_{{.item.suffix}}}
---*/

{% include 'utils/test_escompat_dataview_method.j2' %}

{% endfor %}

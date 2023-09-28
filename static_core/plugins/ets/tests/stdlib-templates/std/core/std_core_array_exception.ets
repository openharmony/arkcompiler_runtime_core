{%- for item in std_core_array_copyTo_exception %}
/*---
desc:  {function: {{.item.method_name}}_{{.item.method_signature_desc.values()|join("_")}}}
---*/
{% include 'utils/test_core_array_lib_exception.j2' with context %}
{%- endfor %}

{%- for item in std_core_array_copyOf_2_exception %}
/*---
desc:  {function: {{.item.method_name}}_{{.item.method_signature_desc.values()|join("_")}}}
---*/
{% include 'utils/test_core_array_lib_exception.j2' with context %}
{%- endfor %}

{%- for item in std_core_array_copyOf_exception %}
/*---
desc:  {function: {{.item.method_name}}_{{.item.method_signature_desc.values()|join("_")}}}
---*/
{% include 'utils/test_core_array_lib_exception.j2' with context %}
{%- endfor %}

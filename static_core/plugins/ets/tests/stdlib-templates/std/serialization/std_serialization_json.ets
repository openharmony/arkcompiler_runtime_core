{% for item in std_serialization_json_static %}
/*---
desc:  {function: {{.item.method_name}}_{{.item.method_signature_desc.values()|join("_")}}}
---*/
{% include 'utils/test_serialization_json_lib.j2' %}
{% endfor %}


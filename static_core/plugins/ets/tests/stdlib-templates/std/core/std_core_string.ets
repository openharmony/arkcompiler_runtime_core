{% for item in std_core_string_instance %}
{% set signature_desc = '_' + '_'.join(item.param_types.values()).replace('[]', '_array') if item.param_types else '' %}
/*---
desc:  {function: {{.item.object_type}}_{{.item.method_name}}{{.signature_desc}}}
---*/
{% include 'utils/test_instance_method.j2' %}
{% include 'utils/test_verifier_lib.j2' with context %} 
{% endfor %}

{% for item in std_core_string_static %}
/*---
desc:  {function: {{.item.object_type}}_{{.item.method_name}}}
---*/
{% include 'utils/test_static_method.j2' %}
{% include 'utils/test_verifier_lib.j2' with context %} 
{% endfor %}

{% for item in std_core_regexp_instance %}
/*---
desc:  {function: {{.item.object_type}}_{{.item.method_name}}}
---*/
{% include 'utils/test_instance_method.j2' %}
{% include 'utils/test_verifier_lib.j2' with context %} 
{% endfor %}

// {% for item in std_core_regexp_static %}
// /*---
// desc:  {function: {{.item.object_type}}_{{.item.method_name}}}
// ---*/
// {% include 'utils/test_static_method.j2' %}
// {% include 'utils/test_verifier_lib.j2' with context %} 
// {% endfor %}

{% for item in std_core_constructor %}

/*---
desc:  {function: {{.item.object_type}}_{{.item.method_name}}}
---*/

{% include 'utils/test_constructor.j2' %}
{% include 'utils/test_verifier_lib.j2' with context %} 

{% endfor %}

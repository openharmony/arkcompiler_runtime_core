{% for item in std_core_typedarrays_instance %}
/*---
desc:  {function: {{.item.object_type}}_{{.item.objectType}}}
---*/

{% include 'utils/test_core_typedarray.j2' %}


{% endfor %}


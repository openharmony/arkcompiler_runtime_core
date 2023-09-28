{% for id in identifiers %}
{% for item in default_value %}
/*---
desc: Interface fields without types
tags: [compile-only]
params: "{{id}}  = {{item.value}}"
---*/

interface Name {
  {{id}} = {{item.value}};
}

{% endfor %}
{% endfor %}

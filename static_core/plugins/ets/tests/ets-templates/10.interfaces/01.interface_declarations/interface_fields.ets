{% for id in identifiers %}
{% for item in default_value %}
/*---
desc: Interface fields with specified types
tags: [compile-only]
params: "{{id}}: {{item.type}} = {{item.value}}"
---*/

interface Name {
  {{id}}: {{item.type}} = {{item.value}}
}

{% endfor %}
{% endfor %}


/*---
desc: Several interface fields at the same time
tags: [compile-only]
---*/

interface Name {
  {% for item in default_value %}
    {{item.type}}Field: {{item.type}} = {{item.value}};
  {% endfor %}
}

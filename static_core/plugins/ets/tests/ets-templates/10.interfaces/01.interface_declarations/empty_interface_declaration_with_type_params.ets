{% for id in identifiers %}
{% for params in identifiersList %}
/*---
desc: Empty interface declaration without type parameters
tags: [compile-only]
params: interface {{id}}
---*/

interface {{id}}<{{params}}> {
}

{% endfor %}
{% endfor %}
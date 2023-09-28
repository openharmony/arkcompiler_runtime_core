{% for id in identifiers %}
/*---
desc: Empty interface declaration without type parameters
tags: [compile-only]
params: interface {{id}}
---*/

interface A {}

interface {{id}} extends A {
}

{% endfor %}
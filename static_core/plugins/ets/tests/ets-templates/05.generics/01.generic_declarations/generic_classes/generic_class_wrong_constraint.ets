{% for wc in wrong_constaints %}
{% for cm in top_level_class_modifiers %}
/*---
desc: generic parameters with wrong constraint in extends
tags: [compile-only, negative]
params: >
    {{wc}}
    {{cm}}

---*/

{{cm}} class Point<T extends {{wc}}> {}

function main(): void {}

{% endfor %}
{% endfor %}


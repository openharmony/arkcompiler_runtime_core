{% for sd in self_dependency %}
{% for cm in top_level_class_modifiers %}
/*---
desc: self-dependency is prohibited
tags: [compile-only, negative]
params: >
    {{sd}}
    {{cm}}

---*/

{{cm}} class T<{{sd}}> {}

function main(): void {}

{% endfor %}
{% endfor %}


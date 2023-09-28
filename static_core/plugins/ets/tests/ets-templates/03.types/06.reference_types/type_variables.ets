{% for d in declarations %}

/*---
desc: Class literal
---*/

class A {}

{{d}}

function main(): void {}

{% endfor %}

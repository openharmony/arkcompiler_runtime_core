{% for cm in class_members %}

/*---
desc: Class members appears twice
tags: [negative, compile-only]
---*/

final class A {
  {{cm}}
  {{cm}}
}

function main(): void {}
{% endfor %}

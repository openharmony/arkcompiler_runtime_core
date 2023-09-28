{% for cm in class_members %}
/*---
desc: class members
---*/

class A {
  {{cm}}
}

function main(): void {}
{% endfor %}

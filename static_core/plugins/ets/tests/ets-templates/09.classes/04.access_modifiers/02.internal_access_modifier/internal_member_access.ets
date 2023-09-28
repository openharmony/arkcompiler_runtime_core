{% for member in members %}
/*---
desc: Check members with internal access modifier.
assert: Member or constructor with private modifier can be accessed only eithin their declaring class.
params: "{{member.body}}"
---*/

class A {
  internal {{member.body}}
}

function main(): void {
  let instance: A = new A();

  instance.{{member.call}};
}

{% endfor %}
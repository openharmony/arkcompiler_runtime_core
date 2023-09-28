{% for member in members %}
/*---
desc: Check members with private access modifier out class.
assert: Member or constructor with private modifier can be accessed only within their declaring class.
params: "{{member.body}}"
tags: [negative, compile-only]
---*/

class A {
  private {{member.body}}
}

function main(): void {
  let instance: A = new A();

  instance.{{member.call}};
}

{% endfor %}

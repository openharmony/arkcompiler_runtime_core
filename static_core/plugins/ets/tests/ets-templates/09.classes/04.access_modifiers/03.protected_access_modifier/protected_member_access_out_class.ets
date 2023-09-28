{% for member in members %}
/*---
desc: Check members with protected access modifier out class.
assert: Member or constructor with protected modifier can be accessed only within their declaring class and classes derived from their declaring class.
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

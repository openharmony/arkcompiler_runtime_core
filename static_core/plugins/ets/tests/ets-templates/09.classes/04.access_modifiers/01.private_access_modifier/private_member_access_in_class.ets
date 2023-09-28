{% for member in members %}
/*---
desc: Check members with private access modifier in class.
assert: Member or constructor with private modifier can be accessed only eithin their declaring class.
params: "{{member.body}}"
---*/

class A {
  private {{member.body}}

  foo(): {{member.type}} {
    return this.{{member.call}};
  }
}

function main(): void {
  let instance: A = new A();

  assert instance.foo() == {{member.value}};
}

{% endfor %}
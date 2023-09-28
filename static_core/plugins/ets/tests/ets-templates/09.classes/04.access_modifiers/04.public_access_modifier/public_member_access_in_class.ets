{% for member in members %}
/*---
desc: Check member with public access modifier in class.
assert: Member or constructor with public modifier can be accessed everywhere.
params: "{{member.body}}"
---*/

public class A {
  public {{member.body}}

  foo(): {{member.type}} {
    return this.{{member.call}};
  }
}

function main(): void {
  let instance: A = new A();

  assert instance.foo() == {{member.value}};
}

{% endfor %}
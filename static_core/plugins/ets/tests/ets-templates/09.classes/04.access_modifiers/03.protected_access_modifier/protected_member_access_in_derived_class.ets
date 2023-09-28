{% for member in members %}
/*---
desc: Check members with protected access modifier in derived class.
assert: Member or constructor with protected modifier can be accessed only within their declaring class and classes derived from their declaring class.
params: "{{member.body}}"
---*/

class A {
  protected {{member.body}}
}

final class B extends A {
  call(): {{member.type}} {
    return this.{{member.call}};
  }
}

function main(): void {
  let instance: B = new B();

  assert instance.call() == {{member.value}};
}

{% endfor %}

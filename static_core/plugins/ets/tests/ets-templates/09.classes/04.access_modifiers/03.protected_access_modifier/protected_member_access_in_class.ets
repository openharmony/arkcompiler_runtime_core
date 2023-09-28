{% for member in members %}
/*---
desc: Check members with protected access modifier in class.
assert: Member or constructor with protected modifier can be accessed only within their declaring class and classes derived from their declaring class.
params: "{{member.body}}"
---*/

class A {
  protected {{member.body}}

  public call(): {{member.type}} {
    return this.{{member.call}};
  }
}


function main(): void {
  let instance: A = new A();

  assert instance.call() == {{member.value}};
}

{% endfor %}
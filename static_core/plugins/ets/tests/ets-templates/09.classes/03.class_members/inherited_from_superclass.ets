{% for cm in class_members %}

/*---
desc: class members
assert: Members inherited from its direct superclass.
---*/

class A {
  {{cm.member}}
}

final class B extends A {
  public fee(): int {
    return {{cm.call}}
  }
}

function main(): void {
    let x: B = new B();
    
    assert x.fee() == {{cm.result}};
}

{% endfor %}

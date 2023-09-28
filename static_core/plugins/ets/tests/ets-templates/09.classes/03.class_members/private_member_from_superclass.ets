{% for cm in class_members %}

/*---
desc: class members
assert: Members of a class that are declared private are not inherited by superclasses of that class.
tags: [negative, compile-only]
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

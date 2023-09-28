{% for mod in modifiers %}

/*---
desc: Overriding method of superclass.
assert: An instance method mC declared in or inherited by class C, overrides from C another method mA declared in class A, if all of the following are true. C is a subclass of A. C does not inherit mA. The signature of mC is a subsignature (link to signatures) of the signature of mA.
params: {{mod}}
---*/

class A {
  {{mod}} m(): int {
    return 1;
  }
}

final class C extends A {
  override m(): int {
    return 5;
  }
}

function main(): void {
  let instance: C = new C();

  assert instance.m() == 5;
}

{% endfor %}

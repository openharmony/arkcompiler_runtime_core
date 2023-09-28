{% for mod in modifiers %}

/*---
desc: Inheritance method of superclass.
assert: A class C inherits from its direct superclass all concrete methods m (both static and instance) of the superclass for which all of the following are true. m is a member of the direct superclass of C. m is public, protected, or declared with package access in the same package as C. No method declared in C has a signature that is a subsignature of the signature of m.
params: {{mod}}
---*/

class A {
  {{mod}} foo(): int {
    return 1;
  }
}

class B extends A {
  fee(): int {
    return this.foo();
  }
}

function main(): void {
  let instance: B = new B();

  assert instance.fee() == 1;
}

{% endfor %}

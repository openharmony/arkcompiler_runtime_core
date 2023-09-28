{% for member in members %}

/*---
desc: Declare a field with the name that is already used for another field or a method or a member type in this declaration.
assert: It is a compile-time error for the body of a class declaration to declare a field with the name that is already used for another field or a method or a member type in this declaration.
params: mod
tags: [compile-only, negative]
---*/

class A {
  {{member}}

  x: int;
}

{% endfor %}

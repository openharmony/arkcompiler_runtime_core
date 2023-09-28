{% for mod in modifiers %}

/*---
desc: The same field modifier appears more than once in a field declaration.
assert: It is a compile-time error if the same field modifier appears more than once in a field declaration.
params: mod
tags: [compile-only, negative]
---*/

class A {
  {{mod}} i: int;
}

{% endfor %}

{% for case in cases %}
/*---
desc: >-
  A compile-time error if the chain of conversions contains
  two parameterized types that are not in the subtype relation.
params: Assignment of '{{case.src_type|safe}}' to '{{case.dst_type|safe}}'
tags: [compile-only, negative]
---*/

class X {}
class Y extends X {}

class A<T> {}
class Aout<out T> {}
class Ain<in T> {}
class B<T> extends A<T> {}
class Bout<out T> extends Aout<T> {}
class Bin<in T> extends Ain<T> {}

function main(): int {
  let src: {{case.src_type|safe}} = new {{case.src_type|safe}}();
  let dst: {{case.dst_type|safe}} = src;  // testing this assignment
  if (dst !== src) {
      return 1;
  }
  return 0;
}
{% endfor %}

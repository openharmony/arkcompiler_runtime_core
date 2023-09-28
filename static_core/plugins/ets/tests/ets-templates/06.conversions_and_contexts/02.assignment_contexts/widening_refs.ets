{% for case in cases %}
/*---
desc: Assignment contexts allow the use a widening reference conversion.
params: Assignment of '{{case.src_type|safe}}' to '{{case.dst_type|safe}}'
---*/
interface I {}
class A implements I {}
class B extends A {}
class C extends B {}

{{case.defs|safe}}

function main(): int {
  let src: {{case.src_type|safe}} = new {{case.src_class|safe}}();
  let dst: {{case.dst_type|safe}} = src;  // this assignment is tested
  if (dst !== src) {
      return 1;
  }
  return 0;
}
{% endfor %}

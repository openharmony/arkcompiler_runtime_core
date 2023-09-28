{% for case in cases %}
/*---
desc: >-
    This reference value is then converted to type string by string conversion. The
    conversion is performed as if by an invocation of the toString method of the
    referenced object with no arguments.
---*/

interface I {}
interface J extends I {}
class A implements I {
  public override toString(): string {
    return super.toString() + " :> A";
  }
}
class B extends A implements J {
  public override toString(): string {
    return super.toString() + " :> B";
  }
}

function main(): int {
  {%- for t in case['types'] %}
  let src_{{loop.index}}: {{t.type}} = new {{t.type}}();
  let dst_{{loop.index}}: string = "" + src_{{loop.index}};
  let exp_{{loop.index}}: string = src_{{loop.index}}.toString();
  if (!dst_{{loop.index}}.equals(exp_{{loop.index}})) {
      return 1;
  }
  {% endfor %}
  return 0;
}
{% endfor %}

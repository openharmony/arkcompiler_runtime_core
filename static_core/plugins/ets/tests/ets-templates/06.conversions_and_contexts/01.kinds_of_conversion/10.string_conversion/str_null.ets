{% for t in cases %}
/*---
desc: >-
    This reference value is then converted to type String by string conversion. The
    conversion is performed as if by an invocation of the toString method of the
    referenced object with no arguments.
params: null
---*/

{{t.defs}}

function main(): int {
  let src: {{t.type}} = {{t.val}};
  let dst: string = "" + src;
  if (!dst.equals("null")) {
      return 1;
  }
  return 0;
}
{% endfor %}

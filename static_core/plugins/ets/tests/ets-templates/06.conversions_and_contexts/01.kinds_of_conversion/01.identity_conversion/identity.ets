{% for t in cases %}
/*---
desc: A conversion from a type to that same type is permitted for any type.
---*/

{{t.defs}}

function main(): int {
  let src: {{t.type}} = {{t.expr|safe}};
  let dst: {{t.type}} = src
  if (src {{t.op}} dst) {
      return 1;
  }
  return 0;
}
{% endfor %}

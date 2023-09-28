{% for case in cases %}
/*---
desc: Assignment contexts allow the use of the widening primitive conversion.
params: Conversion from '{{case.src_type}}' to '{{case.dst_type}}'
---*/

function main(): int {
  let orig: {{case.src_type}} = {{case.src_val|safe}};
  let src: {{case.src_type}} = orig;
  let dst: {{case.dst_type}} = src;   // testing this assignment
  if (dst != {{case.dst_val}}) {
      return 1;
  }
  return 0;
}
{% endfor %}

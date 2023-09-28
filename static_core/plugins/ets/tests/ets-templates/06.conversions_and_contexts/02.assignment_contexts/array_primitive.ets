{% for case in cases %}
/*---
desc: Assignment contexts does not allow the assignment for arrays if one has elements of a primitive types
params: Assignment from '{{case.src_type}}' to '{{case.dst_type}}'
tags:
- negative
- compile-only
---*/

function main(): void {
  let src_{{loop.index}}: {{case.src_type}}[] = new {{case.src_type}}[1];
  let dst_{{loop.index}}: {{case.dst_type}}[] = new {{case.dst_type}}[1];
  dst_{{loop.index}} = src_{{loop.index}};
  return;
  assert false;  // CTE: unreachable statement
}
{% endfor %}
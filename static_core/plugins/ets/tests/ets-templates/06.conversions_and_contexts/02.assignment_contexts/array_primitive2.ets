{% for case in cases %}
/*---
desc: Assignment contexts allow the assignment for arrays if they are same primitive types
params: Assignment from '{{case.src_type}}' to '{{case.dst_type}}'
tags:
- compile-only
---*/

function main(): void {
  let src_{{loop.index}}: {{case.src_type}}[] = new {{case.src_type}}[1];
  let dst_{{loop.index}}: {{case.dst_type}}[] = src_{{loop.index}};
  dst_{{loop.index}} = src_{{loop.index}};
  return;
}
{% endfor %}
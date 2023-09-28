{% for case in cases %}
/*---
desc: Assignment contexts allow the use a widening reference conversion followed by an unboxing conversion.
params: Assignment of '{{case.type}}' to '{{case.ptype}}'
tags: {{case.tags}}
---*/

function foo<T extends {{case.type}}>(p: T): {{case.ptype}} {
  let src: T = p;
  let dst: {{case.ptype}} = src;
  return dst;
}

function main(): int {
  let res: {{case.ptype}} = foo(new {{case.type}}({{case.val|safe}}));
  if (res != {{case.res|safe}}) {
      return 1;
  }
  return 0;
}
{% endfor %}

{% for c in cases %}
/*---
desc: Casting contexts allow the use of the identity conversion.
---*/

enum Color { Red, Green, Blue };
type Foo = (p: Char) => char;

function main(): int {
  {%- for t in c['types'] %}
  let src{{loop.index}}: {{t.type}} = {{t.expr|safe}};
  let dst{{loop.index}}: {{t.type}} = src{{loop.index}} as {{t.type}};
  if (src{{loop.index}} {{t.op}} dst{{loop.index}}) {
      return 1;
  }
  {%- endfor %}
  return 0;
}
{% endfor %}

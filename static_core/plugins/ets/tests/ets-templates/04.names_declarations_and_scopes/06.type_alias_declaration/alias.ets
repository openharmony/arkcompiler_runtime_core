{% for t in cases %}
/*---
desc: >-
    Type aliases provide names for anonymous types (array, function or nullable types)
    or alternative names for existing types. Type aliases allow one to use meaningful
    and concise notation.
params: '{{t.aval|safe}} to {{t.aname}}'
---*/
{{t.defs}}

type {{t.aname}} = {{t.aval|safe}};

function main(): int {
  let v: {{t.aname}} = {{t.val|safe}};
  let q: {{t.aval|safe}} = v;
  if (v {{t.op}} q) {
      return 1;
  }
  return 0;
}
{% endfor %}

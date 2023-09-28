{% for wid in widening_primitives %}

/*---
desc: Widening primitive type {{wid.origin}}
---*/

let origin: {{wid.origin}} = 0;
function main(): void {
{%- for t in wid['dest'] %}
  let x{{t}}: {{t}} = origin;
{%- endfor %}
}

{% endfor %}
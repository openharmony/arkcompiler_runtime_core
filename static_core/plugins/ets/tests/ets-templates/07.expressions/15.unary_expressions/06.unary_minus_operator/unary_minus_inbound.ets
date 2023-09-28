{% for inb in unary_minus_inbound %}

/*---
desc: Unary minus operator (inbound)
params: {{inb.type}} {{inb.origin}} => {{inb.dest}}
---*/

function main(): void {
  let a: {{inb.type}} = {{inb.origin}};
  assert(-a == {{inb.dest}});
}

{% endfor %}
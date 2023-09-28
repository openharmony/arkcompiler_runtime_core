{% for lc in logical_complement %}
/*---
desc: Logical complement operator
params: {{lc.origin}} => {{lc.dest}}
---*/

function main(): void {
  let a = {{lc.origin}};
  assert(!a == {{lc.dest}});
}

{% endfor %}
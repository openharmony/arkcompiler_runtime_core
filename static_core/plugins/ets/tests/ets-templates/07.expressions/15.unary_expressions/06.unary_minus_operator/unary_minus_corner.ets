{% for un in unary_minus_corner %}

/*---
desc: Unary minus operator (corner cases)
params: {{un.origin}} => {{un.dest}}
---*/

function main(): void {
  let a = {{un.origin}};
  assert(-a == {{un.dest}});
}

{% endfor %}

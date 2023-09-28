{% for onb in unary_minus_onbound %}

/*---
desc: Unary minus operator (onbound) (these are special cases for numeric overflow behavior. Special attention wanted).
params: {{onb.type}} {{onb.origin}} => {{onb.dest}}
---*/

function main(): void {
  let a: {{onb.type}} = {{onb.origin}};
  assert(-a == {{onb.dest}});
}

{% endfor %}

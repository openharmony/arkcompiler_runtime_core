{% for conv in boxing_nan %}

/*---
desc: Boxing conversion for floating-point types (NaN)
---*/

let origin: {{conv.origin}} = 0.0 / 0.0;
let dest:   {{conv.dest}};

function main(): void {
  dest = origin;
  assert(dest.isNaN());
}

{% endfor %}

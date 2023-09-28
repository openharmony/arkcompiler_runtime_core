{% for conv in boxing_conversions %}

/*---
desc: Boxing conversion for types
---*/

let origin: {{conv.origin}} = {{conv.value}};
let dest:   {{conv.dest}};

function main(): void {
  dest = origin;
  assert(dest.unboxed() == origin);
}

{% endfor %}

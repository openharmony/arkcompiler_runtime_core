{% for conv in unboxing_conversions %}

/*---
desc: Unboxing conversion for types
params: {{conv.origin}}, {{conv.value}}, {{conv.dest}}
---*/

let origin: {{conv.origin}} = {{conv.value}};

let dest:   {{conv.dest}};

function main(): void {
  dest = origin;
  assert(dest == origin.unboxed());
}

{% endfor %}

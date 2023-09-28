{% for i in default_value %}

/*---
desc: Integer type {{i.type}} initialization.
params: {{i.type}}, {{i.value}}
---*/

let x: {{i.type}} = {{i.value}};
function main(): void {
  let y: {{i.type}} = {{i.value}};

  assert x == {{i.value}};
  assert y == {{i.value}}
}

{% endfor %}
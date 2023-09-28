{% for i in default_value %}

/*---
desc: Default values for primitive type {{i.type}} 
params: {{i.type}}, {{i.value}}
---*/


function main(): void {
  let x: {{i.type}};
  assert(x == {{i.value}});
}

{% endfor %}
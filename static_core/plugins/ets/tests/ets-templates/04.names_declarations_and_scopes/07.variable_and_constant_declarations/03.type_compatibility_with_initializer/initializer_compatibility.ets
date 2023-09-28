{% for i in type_and_initializer %}

/*---
desc: Type {{i.type}} compatibility with initializer {{i.value}}
---*/

{{i.typedecl}}

function main(): void {
  let a: {{i.type}} = {{i.value}};
}

{% endfor %}

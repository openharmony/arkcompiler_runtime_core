{% for i in default_value %}

/*---
desc: Predefined type name {{i}} using in name of the field.
params: {{i}}
tags: [negative, compile-only]
---*/

let {{i}}: {{i}};
function main(): void {
  let {{i}}: {{i}};
}

{% endfor %}

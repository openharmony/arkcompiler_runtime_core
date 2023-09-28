{% for wc in wrong_constraints %}
{% for tog in type_with_one_generic %}
{% for bd in bounds %}

/*---
desc: Self-dependency is prohibited
tags: [negative, compile-only]
params: >
  {{wc}}
  {{tog}}
  {{bd}}
---*/

{{tog.decl}}

function main(): void {
    let x = {{tog.type}}<{{bd}}{{wc}}>{{tog.curly_if_need}};
}

{% endfor %}
{% endfor %}
{% endfor %}

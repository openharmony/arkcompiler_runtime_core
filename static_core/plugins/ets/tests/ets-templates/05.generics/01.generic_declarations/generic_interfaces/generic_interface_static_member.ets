{% for stm in static_members_with_type_param %}

/*---
desc: wrong usage of type variable in static member
tags: [compile-only, negative]
params: >
  {{stm}}
---*/

interface I<X> {
	{{stm}}
}

function main(): void {}

{% endfor %}


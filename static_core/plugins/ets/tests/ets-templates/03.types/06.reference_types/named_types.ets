{% for named in named_types %}

/*---
desc: Named type {{named}}
params: {{named}}
---*/

{{named}}

function main(): void {}

{% endfor %}
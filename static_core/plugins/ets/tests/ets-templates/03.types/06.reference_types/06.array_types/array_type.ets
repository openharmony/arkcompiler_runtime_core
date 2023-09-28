{% for item in array_literal %}

/*---
desc: An array is an object which contains elements of a similar data type
params: {{item.array}}
---*/

function main(): void {
    let x: {{item.type}}[] = {{item.array}};
    assert {{item.assert}}
}
{% endfor %}

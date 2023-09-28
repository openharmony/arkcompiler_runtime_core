{% for c in cases %}
/*---
desc: An array is an object which contains elements of a similar data type
---*/

{{c.decl}}

function main(): void {
    let arr:{{c.type}} = {{c.vals}}
    arr[0] = {{c.val}}
    assert {{c.assert}}
}
{% endfor %}

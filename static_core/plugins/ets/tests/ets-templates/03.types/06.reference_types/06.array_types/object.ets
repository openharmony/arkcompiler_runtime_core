{% for c in cases %}
/*---
desc: As an object an array is assignable to a variable of type Object
---*/

{{c.decl}}

function foo(p: {{c.type}}): Object {
    return p as Object;
}

function main(): void {
    let arr: {{c.type}} = {{c.vals}}
    let obj: Object = foo(arr)
    assert obj === arr
    assert (obj as {{c.type}}){{c.assert}}
}
{% endfor %}

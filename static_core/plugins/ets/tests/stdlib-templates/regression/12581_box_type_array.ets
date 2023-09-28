{% for item in bug_12581_box_type %}
/*---
desc:  {function: {{.item.box_type_desc}}}
---*/

const ARRAY_SIZE = {{.item.box_type_array_size}};

interface Iface<T> {
    getBoxObjectArray() : T[];
}

class BoxTypeArrayProvider<T> implements Iface<T> {
    override getBoxObjectArray(): T[] {
        return new T[ARRAY_SIZE];
    }
}

function test{{.item.box_type}}Wrapper() : int {
    let arrayProvider = new BoxTypeArrayProvider<{{.item.box_type}}>();
    let {{.item.box_type|lower}}Array = arrayProvider.getBoxObjectArray();
    let inputData : {{.item.box_type}}[] = {{.item.array_data}};
    let expectedData : {{.item.box_type}}[] = {{.item.expected_data}};
    if ({{.item.box_type|lower}}Array.length != ARRAY_SIZE) return 1;
    for (let i  = 0; i< {{.item.box_type|lower}}Array.length; i++) {
        if ({{.item.box_type|lower}}Array[i] instanceof {{.item.box_type|capitalize}}) return 1;
    }
    for (let i = 0; i< inputData.length; i++) {
        {{.item.box_type|lower}}Array[i] = new {{.item.box_type}}(inputData[i]);
    }
    for (let i = 0; i< inputData.length; i++) {
        if ({{.item.box_type|lower}}Array[i] != expectedData[i]) return 1;
    }

    return 0;
}

function main() : int {
    return test{{.item.box_type}}Wrapper();
}

{% endfor %}

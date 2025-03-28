/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import {
    AValue,
    CValue,
    jsAvalue,
    jsCvalue,
    jsFn,
} from "js"
import {
    ADeclared,
    BDeclared,
    CDeclared,
    DDeclared,
    asJsvalue,
    createError,
} from "dynamic_import_with_decl"


class A {}
class B extends BDeclared {}
class C extends A {}


function errLog(op: String) {
    console.println("Bug operation: " + op);
}



/*
 * =====================================
 * Tests <object type>InstanceofObject
 * =====================================
 */

function TestObjectInstanceofObject(): boolean {
    let o = new Object();
    if (!(o instanceof Object)) {
        errLog("o instanceof Object");
        return false;
    }
    return true;
}

function TestEtstypeInstanceofObject(): boolean {
    let a = new A();
    if (!(a instanceof Object)) {
        errLog("a instanceof Object");
        return false;
    }
    return true;
}

function TestJsvalueInstanceofObject(): boolean {
    // Check js object as left argument
    if (!(jsAvalue instanceof Object)) {
        errLog("js_avalue instanceof Object");
        return false;
    }
    let jsAvalueX = asJsvalue(jsAvalue);
    if (!(jsAvalueX instanceof Object)) {
        errLog("js_avalue_x instanceof Object");
        return false;
    }

    // Check ets object as left argumet
    let a = new A();
    let aJsvalue: JSValue = asJsvalue(a);
    if (!(aJsvalue instanceof Object)) {
        errLog("a_jsvalue instanceof Object");
        return false;
    }
    return true;
}

function TestDynvalueInstanceofObject(): boolean {
    let avalue: AValue = new AValue();
    if (!(avalue instanceof Object)) {
        errLog("avalue instanceof Object");
        return false;
    }
    let cvalue: CValue = new CValue();
    if (!(cvalue instanceof Object)) {
        errLog("cvalue instanceof Object");
        return false;
    }
    return true;
}

function TestDyndeclInstanceofObject(): boolean {
    let adecl = new ADeclared();
    if (!(adecl instanceof Object)) {
        errLog("adecl instanceof Object");
        return false;
    }
    return true;
}



/*
 * ==============================================
 * === Tests <object type>InstanceofEtstype ===
 * ==============================================
 */

function TestObjectInstanceofEtstype(): boolean {
    let o = new Object();
    if (o instanceof A) {
        errLog("o instanceof A");
        return false;
    }
    return true;
}

function TestEtstypeInstanceofEtstype(): boolean {
    let a = new A();
    if (!(a instanceof A)) {
        errLog("a instanceof A");
        return false;
    }
    // TODO(v.cherkashin): Enable when inheritance from declared type will be implemented, #13552
    //let b = new B();
    //if (!(b instanceof A)) {
    //    errLog("b instanceof A");
    //    return false;
    //}
    let c = new C();
    if (!(c instanceof A)) {
        errLog("c instanceof A");
        return false;
    }
    return true;
}

function TestJsvalueInstanceofEtstype(): boolean {
    // Check js object as left argument
    if (jsAvalue instanceof A) {
        errLog("js_avalue instanceof A");
        return false;
    }
    let jsAvalueX = asJsvalue(jsAvalue);
    if (jsAvalueX instanceof A) {
        errLog("js_avalue_x instanceof A");
        return false;
    }

    // Check ets object as left argumet
    let a = new A();
    let aJsvalue: JSValue = asJsvalue(a);
    if (!(aJsvalue instanceof A)) {
        errLog("a_jsvalue instanceof A");
        return false;
    }
    // TODO(v.cherkashin): Enable when inheritance from declared type will be implemented, #13552
    //if (aJsvalue instanceof B) {
    //    errLog("a_jsvalue instanceof B");
    //    return false;
    //}
    let c = new C();
    let cJsvalue: JSValue = asJsvalue(c);
    if (!(cJsvalue instanceof A)) {
        errLog("c_jsvalue instanceof A");
        return false;
    }
    return true;
}

function TestDynvalueInstanceofEtstype(): boolean {
    let avalue: AValue = new AValue();
    if (avalue instanceof A) {
        errLog("avalue instanceof A");
        return false;
    }
    // TODO(v.cherkashin): Enable when inheritance from declared type will be implemented, #13552
    //let cvalue: CValue = new CValue();
    //if (!(cvalue instanceof B)) {
    //    errLog("cvalue instanceof B");
    //    return false;
    //}
    return true;
}

function TestDyndeclInstanceofEtstype(): boolean {
    let adecl = new ADeclared();
    if (adecl instanceof A) {
        errLog("adecl instanceof A");
        return false;
    }
    let ddecl = new DDeclared();
    if (!(ddecl instanceof A)) {
        errLog("ddecl instanceof A");
        return false;
    }

    // Check compat class
    let etsError = new Error("ets error message")
    let etsErrorJsval: JSValue = etsError;
    if (!(etsErrorJsval instanceof Error)) {
        errLog("etsErrorJsval instanceof Error");
        return false;
    }

    let jsError: Error = createError("error message");
    if (!(jsError instanceof Error)) {
        errLog("jsError instanceof Error");
        return false;
    }
    // TODO(v.cherkashin): Enable when processing the compat types will be implemented, #13577
    //let jsErrorJsval: JSValue = jsError;
    //if (!(js_error_jsval instanceof Error)) {
    //    errLog("js_error_jsval instanceof Error");
    //    return false;
    //}
    return true;
}


/*
 * ==============================================
 * === Tests <object type>InstanceofDyndecl ===
 * ==============================================
 */

function TestObjectInstanceofDyndecl(): boolean {
    // Check 'object instanceof dyndecl'
    let o = new Object();
    if (o instanceof BDeclared) {
        errLog("o instanceof BDeclared");
        return false;
    }
    return true;
}

function TestEtstypeInstanceofDyndecl(): boolean {
    // Check 'etstype instanceof dyndecl'
    let a = new A();
    if (a instanceof BDeclared) {
        errLog("a instanceof BDeclared");
        return false;
    }
    // TODO(v.cherkashin): Enable when inheritance with declared type will be implemented, #13552
    //let b = new B();
    //if (!(b instanceof BDeclared)) {
    //    errLog("b instanceof BDeclared");
    //    return false;
    //}
    return true;
}

function TestJsvalueInstanceofDyndecl(): boolean {
    // Check 'jsvalue instanceof dyndecl'
    if (jsAvalue instanceof BDeclared) {
        errLog("js_avalue instanceof BDeclared");
        return false;
    }
    if (!(jsCvalue instanceof BDeclared)) {
        errLog("jsCvalue instanceof BDeclared");
        return false;
    }
    return true;
}

function TestDynvalueInstanceofDyndecl(): boolean {
    // Check 'dynvalue instanceof dyndecl'
    let avalue: AValue = new AValue();
    if (avalue instanceof BDeclared) {
        errLog("avalue instanceof BDeclared");
        return false;
    }
    let cvalue: CValue = new CValue();
    if (!(cvalue instanceof BDeclared)) {
        errLog("cvalue instanceof BDeclared");
        return false;
    }
    return true;
}

function TestDyndeclInstanceofDyndecl(): boolean {
    let adecl: ADeclared = new ADeclared();
    if (adecl instanceof BDeclared) {
        errLog("adecl instanceof BDeclared");
        return false;
    }
    let bdecl: BDeclared = new BDeclared();
    if (!(bdecl instanceof BDeclared)) {
        errLog("bdecl instanceof BDeclared");
        return false;
    }
    let cdecl: CDeclared = new CDeclared();
    if (cdecl instanceof BDeclared) {
        errLog("cdecl instanceof BDeclared");
        return false;
    }
    if (!(cdecl instanceof ADeclared)) {
        errLog("cdecl instanceof ADeclared");
        return false;
    }

    // Check 'js_fn instanceof dyndecl'
    if (jsFn instanceof BDeclared) {
        errLog("js_fn instanceof BDeclared");
        return false;
    }

    return true;
}

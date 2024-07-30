/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
    js_avalue,
    js_cvalue,
    js_fn,
} from "/plugins/ets/tests/interop_js/tests/instanceof/js/instanceof.js"
import {
    ADeclared,
    BDeclared,
    CDeclared,
    DDeclared,
    as_jsvalue,
    create_error,
} from "dynamic_import"


class A {}
class B extends BDeclared {}
class C extends A {}


function err_log(op: String) {
    console.println("Bug operation: " + op);
}



/*
 * =====================================
 * Tests <object type>_instanceof_object
 * =====================================
 */

function Test_object_instanceof_object(): boolean {
    let o = new Object();
    if (!(o instanceof Object)) {
        err_log("o instanceof Object");
        return false;
    }
    return true;
}

function Test_etstype_instanceof_object(): boolean {
    let a = new A();
    if (!(a instanceof Object)) {
        err_log("a instanceof Object");
        return false;
    }
    return true;
}

function Test_jsvalue_instanceof_object(): boolean {
    // Check js object as left argument
    if (!(js_avalue instanceof Object)) {
        err_log("js_avalue instanceof Object");
        return false;
    }
    let js_avalue_x = as_jsvalue(js_avalue);
    if (!(js_avalue_x instanceof Object)) {
        err_log("js_avalue_x instanceof Object");
        return false;
    }

    // Check ets object as left argumet
    let a = new A();
    let a_jsvalue: JSValue = as_jsvalue(a);
    if (!(a_jsvalue instanceof Object)) {
        err_log("a_jsvalue instanceof Object");
        return false;
    }
    return true;
}

function Test_dynvalue_instanceof_object(): boolean {
    let avalue: AValue = new AValue();
    if (!(avalue instanceof Object)) {
        err_log("avalue instanceof Object");
        return false;
    }
    let cvalue: CValue = new CValue();
    if (!(cvalue instanceof Object)) {
        err_log("cvalue instanceof Object");
        return false;
    }
    return true;
}

function Test_dyndecl_instanceof_object(): boolean {
    let adecl = new ADeclared();
    if (!(adecl instanceof Object)) {
        err_log("adecl instanceof Object");
        return false;
    }
    return true;
}



/*
 * ==============================================
 * === Tests <object type>_instanceof_etstype ===
 * ==============================================
 */

function Test_object_instanceof_etstype(): boolean {
    let o = new Object();
    if (o instanceof A) {
        err_log("o instanceof A");
        return false;
    }
    return true;
}

function Test_etstype_instanceof_etstype(): boolean {
    let a = new A();
    if (!(a instanceof A)) {
        err_log("a instanceof A");
        return false;
    }
    // TODO(v.cherkashin): Enable when inheritance from declared type will be implemented, #13552
    //let b = new B();
    //if (!(b instanceof A)) {
    //    err_log("b instanceof A");
    //    return false;
    //}
    let c = new C();
    if (!(c instanceof A)) {
        err_log("c instanceof A");
        return false;
    }
    return true;
}

function Test_jsvalue_instanceof_etstype(): boolean {
    // Check js object as left argument
    if (js_avalue instanceof A) {
        err_log("js_avalue instanceof A");
        return false;
    }
    let js_avalue_x = as_jsvalue(js_avalue);
    if (js_avalue_x instanceof A) {
        err_log("js_avalue_x instanceof A");
        return false;
    }

    // Check ets object as left argumet
    let a = new A();
    let a_jsvalue: JSValue = as_jsvalue(a);
    if (!(a_jsvalue instanceof A)) {
        err_log("a_jsvalue instanceof A");
        return false;
    }
    // TODO(v.cherkashin): Enable when inheritance from declared type will be implemented, #13552
    //if (a_jsvalue instanceof B) {
    //    err_log("a_jsvalue instanceof B");
    //    return false;
    //}
    let c = new C();
    let c_jsvalue: JSValue = as_jsvalue(c);
    if (!(c_jsvalue instanceof A)) {
        err_log("c_jsvalue instanceof A");
        return false;
    }
    return true;
}

function Test_dynvalue_instanceof_etstype(): boolean {
    let avalue: AValue = new AValue();
    if (avalue instanceof A) {
        err_log("avalue instanceof A");
        return false;
    }
    // TODO(v.cherkashin): Enable when inheritance from declared type will be implemented, #13552
    //let cvalue: CValue = new CValue();
    //if (!(cvalue instanceof B)) {
    //    err_log("cvalue instanceof B");
    //    return false;
    //}
    return true;
}

function Test_dyndecl_instanceof_etstype(): boolean {
    let adecl = new ADeclared();
    if (adecl instanceof A) {
        err_log("adecl instanceof A");
        return false;
    }
    let ddecl = new DDeclared();
    if (!(ddecl instanceof A)) {
        err_log("ddecl instanceof A");
        return false;
    }

    // Check compat class
    let ets_error = new Error("ets error message")
    let ets_error_jsval: JSValue = ets_error;
    if (!(ets_error_jsval instanceof Error)) {
        err_log("ets_error_jsval instanceof Error");
        return false;
    }

    let js_error: Error = create_error("error message");
    if (!(js_error instanceof Error)) {
        err_log("js_error instanceof Error");
        return false;
    }
    // TODO(v.cherkashin): Enable when processing the compat types will be implemented, #13577
    //let js_error_jsval: JSValue = js_error;
    //if (!(js_error_jsval instanceof Error)) {
    //    err_log("js_error_jsval instanceof Error");
    //    return false;
    //}
    return true;
}


/*
 * ==============================================
 * === Tests <object type>_instanceof_dyndecl ===
 * ==============================================
 */

function Test_object_instanceof_dyndecl(): boolean {
    // Check 'object instanceof dyndecl'
    let o = new Object();
    if (o instanceof BDeclared) {
        err_log("o instanceof BDeclared");
        return false;
    }
    return true;
}

function Test_etstype_instanceof_dyndecl(): boolean {
    // Check 'etstype instanceof dyndecl'
    let a = new A();
    if (a instanceof BDeclared) {
        err_log("a instanceof BDeclared");
        return false;
    }
    // TODO(v.cherkashin): Enable when inheritance with declared type will be implemented, #13552
    //let b = new B();
    //if (!(b instanceof BDeclared)) {
    //    err_log("b instanceof BDeclared");
    //    return false;
    //}
    return true;
}

function Test_jsvalue_instanceof_dyndecl(): boolean {
    // Check 'jsvalue instanceof dyndecl'
    if (js_avalue instanceof BDeclared) {
        err_log("js_avalue instanceof BDeclared");
        return false;
    }
    if (!(js_cvalue instanceof BDeclared)) {
        err_log("js_cvalue instanceof BDeclared");
        return false;
    }
    return true;
}

function Test_dynvalue_instanceof_dyndecl(): boolean {
    // Check 'dynvalue instanceof dyndecl'
    let avalue: AValue = new AValue();
    if (avalue instanceof BDeclared) {
        err_log("avalue instanceof BDeclared");
        return false;
    }
    let cvalue: CValue = new CValue();
    if (!(cvalue instanceof BDeclared)) {
        err_log("cvalue instanceof BDeclared");
        return false;
    }
    return true;
}

function Test_dyndecl_instanceof_dyndecl(): boolean {
    let adecl: ADeclared = new ADeclared();
    if (adecl instanceof BDeclared) {
        err_log("adecl instanceof BDeclared");
        return false;
    }
    let bdecl: BDeclared = new BDeclared();
    if (!(bdecl instanceof BDeclared)) {
        err_log("bdecl instanceof BDeclared");
        return false;
    }
    let cdecl: CDeclared = new CDeclared();
    if (cdecl instanceof BDeclared) {
        err_log("cdecl instanceof BDeclared");
        return false;
    }
    if (!(cdecl instanceof ADeclared)) {
        err_log("cdecl instanceof ADeclared");
        return false;
    }

    // Check 'js_fn instanceof dyndecl'
    if (js_fn instanceof BDeclared) {
        err_log("js_fn instanceof BDeclared");
        return false;
    }

    return true;
}

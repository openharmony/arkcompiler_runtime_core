/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

const { etsVm, getTestClass, getTestFunction } = require("ets_proxy.test.js");

const FooBase = getTestClass("FooBase");
const FooDerived = getTestClass("FooDerived");
const IdentityError = getTestFunction("IdentityError");

{   // Verify prototype chain and properties
    let base = new FooBase();
    let derived = new FooDerived();

    ASSERT_TRUE(base instanceof FooBase);
    ASSERT_TRUE(derived instanceof FooDerived);
    ASSERT_TRUE(derived instanceof FooBase);

    ASSERT_EQ(Object.getPrototypeOf(FooDerived), FooBase);
    ASSERT_EQ(Object.getPrototypeOf(FooDerived.prototype), FooBase.prototype);
    ASSERT_TRUE(FooDerived.prototype.hasOwnProperty("f_derived"));
    // ASSERT_TRUE(!FooDerived.prototype.hasOwnProperty("f_base"));
    ASSERT_TRUE(FooDerived.hasOwnProperty("sf_derived"));
    // ASSERT_TRUE(!FooDerived.hasOwnProperty("sf_base"));

    ASSERT_TRUE(FooBase.prototype.hasOwnProperty("f_base"));
    ASSERT_EQ(base.f_base, "f_base");
    ASSERT_EQ(FooBase.sf_base, "sf_base");
    ASSERT_EQ(base.fn_base(), "fn_base");

    ASSERT_EQ(derived.f_derived, "f_derived");
    ASSERT_EQ(FooDerived.sf_derived, "sf_derived");
    ASSERT_EQ(derived.f_base, "f_base");
    ASSERT_EQ(FooDerived.sf_base, "sf_base");
    ASSERT_EQ(derived.fn_base(), "fn_base");
}

{   // Passing downcasted object preserves proxy prototype
    let derived = new FooDerived();

    ASSERT_EQ(derived.getAsFooBase(), derived);
    ASSERT_TRUE(FooDerived.createAsFooBase() instanceof FooDerived);
    ASSERT_EQ(FooDerived.createAsFooBase().f_derived, "f_derived");
}

{   // Passing object as core/Object preserves proxy prototype
    let base = new FooBase();

    ASSERT_EQ(base.getAsObject(), base);
    ASSERT_TRUE(FooBase.createAsObject() instanceof FooBase);
    ASSERT_EQ(FooBase.createAsObject().f_base, "f_base");

    ASSERT_EQ("" + base, "FooBase toString");
    ASSERT_EQ("" + FooDerived.createAsFooBase(), "FooDerived toString");
}

{   // Simple compat proxy
    let ets_err = new ets.Error("fooo", undefined);
    ASSERT_TRUE(ets_err instanceof Error);
    ASSERT_EQ(ets_err.message, "fooo");
    ASSERT_EQ(IdentityError(ets_err), ets_err);
}

{   // Classes derived from core/ appear like js built-in objects
    const FooError1 = getTestClass("FooError1");
    const FooError2 = getTestClass("FooError2");

    let err1 = new FooError1();
    ASSERT_TRUE(err1 instanceof FooError1);
    ASSERT_TRUE(err1 instanceof Error);
    ASSERT_EQ(err1.f_error1, "f_error1");
    ASSERT_EQ(err1.message, "");
    err1.toString();

    let err2 = new FooError2();
    ASSERT_TRUE(err2 instanceof FooError2);
    ASSERT_TRUE(err2 instanceof FooError1);
    ASSERT_TRUE(err2 instanceof Error);
    err2.toString();
    ASSERT_EQ(IdentityError(err2), err2);
}

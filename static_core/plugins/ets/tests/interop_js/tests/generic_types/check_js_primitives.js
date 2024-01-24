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

const { check_generic_value } = require("generic_types.test.js");

// Null
check_generic_value(null);

// Undefined
check_generic_value(undefined);

// Boolean
check_generic_value(false);
check_generic_value(true);
check_generic_value(new Boolean(false));
check_generic_value(new Boolean(true));

// Number
check_generic_value(234);
check_generic_value(4.234);
check_generic_value(new Number(34));
check_generic_value(new Number(-643.23566));

// BigInt
// TODO(v.cherkashin): Enable when implemented
// check_generic_value(9007199254740991n);
// check_generic_value(new BigInt("0b11010101"));

// String
check_generic_value("abcdefg");
check_generic_value(new String("ABCDEFG"));

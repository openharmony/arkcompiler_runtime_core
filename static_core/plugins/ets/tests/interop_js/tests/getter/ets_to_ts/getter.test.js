/**
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

const etsVm = require('lib/module/ets_interop_js_napi');

const string = 'string';
const number = 1;
const bool = true;
const arr = [number];
const obj = { test: number };
const tuple = [number, string];

const checkArray = (arg) => arg instanceof Array;
const checkObj = (arg) => arg !== null && typeof arg === 'object' && !Array.isArray(arg);

const PublicGetterClass = etsVm.getClass('Lgetter/test/PublicGetterClass;');
const createPublicGetterClassFromEts = etsVm.getFunction('Lgetter/test/ETSGLOBAL;', 'create_public_getter_class_from_ets');

const ProtectedGetterOrigenClass = etsVm.getClass('Lgetter/test/ProtectedGetterOrigenClass;');
const createProtectedGetterOrigenClassFromEts = etsVm.getFunction('Lgetter/test/ETSGLOBAL;', 'create_protected_getter_origen_class_from_ets');

const ProtectedGetterInheritanceClass = etsVm.getClass('Lgetter/test/ProtectedGetterInheritanceClass;');
const createProtectedGetterInheritanceClassFromEts = etsVm.getFunction('Lgetter/test/ETSGLOBAL;', 'create_protected_getter_inheritance_class_from_ets');

const PrivateGetterClass = etsVm.getClass('Lgetter/test/PrivateGetterClass;');
const createPrivateGetterClassFromEts = etsVm.getFunction('Lgetter/test/ETSGLOBAL;', 'create_private_getter_class_from_ets');

const PrivateGetterInheritanceClass = etsVm.getClass('Lgetter/test/PrivateGetterInheritanceClass;');
const createPrivateGetterInheritanceClassFromEts = etsVm.getFunction('Lgetter/test/ETSGLOBAL;', 'create_private_getter_inheritance_class_from_ets');

const UnionTypeClass = etsVm.getClass('Lgetter/test/UnionTypeClass;');
const createUnionTypeGetterClassFromEts = etsVm.getFunction('Lgetter/test/ETSGLOBAL;', 'create_union_type_getter_class_from_ets');

const AnyTypeClass = etsVm.getClass('Lgetter/test/AnyTypeClass;');
const createAnyTypeGetterClassFromEtsString = etsVm.getFunction('Lgetter/test/ETSGLOBAL;', 'create_any_type_getter_class_from_ets_string');
const createAnyTypeGetterClassFromEtsInt = etsVm.getFunction('Lgetter/test/ETSGLOBAL;', 'create_any_type_getter_class_from_ets_int');
const createAnyTypeGetterClassFromEtsBool = etsVm.getFunction('Lgetter/test/ETSGLOBAL;', 'create_any_type_getter_class_from_ets_bool');
const createAnyTypeGetterClassFromEtsArr = etsVm.getFunction('Lgetter/test/ETSGLOBAL;', 'create_any_type_getter_class_from_ets_arr');
const createAnyTypeGetterClassFromEtsObj = etsVm.getFunction('Lgetter/test/ETSGLOBAL;', 'create_any_type_getter_class_from_ets_obj');
const createAnyTypeGetterClassFromEtsTuple = etsVm.getFunction('Lgetter/test/ETSGLOBAL;', 'create_any_type_getter_class_from_ets_tuple');
const createAnyTypeGetterClassFromEtsUnion = etsVm.getFunction('Lgetter/test/ETSGLOBAL;', 'create_any_type_getter_class_from_ets_union');

const SubsetByRef = etsVm.getClass('Lgetter/test/SubsetByRef;');
const createSubsetByRefGetterClassFromEts = etsVm.getFunction('Lgetter/test/ETSGLOBAL;', 'create_subset_by_ref_getter_class_from_ets');

const SubsetByValueClass = etsVm.getClass('Lgetter/test/SubsetByValueClass;');
const createSubsetByValueGetterClassFromEts = etsVm.getFunction('Lgetter/test/ETSGLOBAL;', 'create_subset_by_value_getter_class_from_ets');

module.exports = {
	checkArray,
	checkObj,
	string,
	number,
	bool,
	arr,
	obj,
	tuple,
	PublicGetterClass,
	createPublicGetterClassFromEts,
	ProtectedGetterOrigenClass,
	createProtectedGetterOrigenClassFromEts,
	ProtectedGetterInheritanceClass,
	createProtectedGetterInheritanceClassFromEts,
	PrivateGetterClass,
	createPrivateGetterClassFromEts,
	PrivateGetterInheritanceClass,
	createPrivateGetterInheritanceClassFromEts,
	UnionTypeClass,
	createUnionTypeGetterClassFromEts,
	AnyTypeClass,
	createAnyTypeGetterClassFromEtsString,
	createAnyTypeGetterClassFromEtsInt,
	createAnyTypeGetterClassFromEtsBool,
	createAnyTypeGetterClassFromEtsArr,
	createAnyTypeGetterClassFromEtsObj,
	createAnyTypeGetterClassFromEtsTuple,
	createAnyTypeGetterClassFromEtsUnion,
	SubsetByRef,
	createSubsetByRefGetterClassFromEts,
	SubsetByValueClass,
	createSubsetByValueGetterClassFromEts,
};

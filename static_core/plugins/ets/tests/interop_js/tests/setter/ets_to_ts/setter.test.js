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

const num = 1;
const string = 'string';

const UnionSetter = etsVm.getClass('Lsetter/test/UnionSetter;');
const InterfaceSetter = etsVm.getClass('Lsetter/test/InterfaceSetter;');
const SubsetRefSet = etsVm.getClass('Lsetter/test/SubsetRefSet;');
const SubsetValueSet = etsVm.getClass('Lsetter/test/SubsetValueSet;');
const BaseClass = etsVm.getClass('Lsetter/test/BaseClass;');
const SetterAnyType = etsVm.getClass('Lsetter/test/SetterAnyType;');
const TupleSet = etsVm.getClass('Lsetter/test/TupleSet;');
const AbstractClass = etsVm.getClass('Lsetter/test/AbstractClass;');

const createAbstractClass = etsVm.getFunction('Lsetter/test/ETSGLOBAL;', 'create_abstratc_class');
const createBaseClass = etsVm.getFunction('Lsetter/test/ETSGLOBAL;', 'create_base_class');
const createUnionClass = etsVm.getFunction('Lsetter/test/ETSGLOBAL;', 'create_union_class');
const createInterfaceClass = etsVm.getFunction('Lsetter/test/ETSGLOBAL;', 'create_interface_class');
const createSubsetRefSetClass = etsVm.getFunction('Lsetter/test/ETSGLOBAL;', 'create_subset_ref_set_class');
const createSubsetValueSetClass = etsVm.getFunction('Lsetter/test/ETSGLOBAL;', 'create_subset_value_set_class');
const createSetterAnyTypeClass = etsVm.getFunction('Lsetter/test/ETSGLOBAL;', 'create_setter_any_type_class');
const createTupleSetClass = etsVm.getFunction('Lsetter/test/ETSGLOBAL;', 'create_tuple_set_class');

module.exports = {
	UnionSetter,
	InterfaceSetter,
	SubsetRefSet,
	SubsetValueSet,
	BaseClass,
	SetterAnyType,
	TupleSet,
	AbstractClass,
	createAbstractClass,
	createBaseClass,
	createUnionClass,
	createInterfaceClass,
	createSubsetRefSetClass,
	createSubsetValueSetClass,
	createSetterAnyTypeClass,
	createTupleSetClass,
	num,
	string,
};

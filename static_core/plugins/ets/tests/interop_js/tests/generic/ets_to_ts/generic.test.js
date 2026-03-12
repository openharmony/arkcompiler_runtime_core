/**
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

const etsVm = globalThis.gtest.etsVm;

const UnionClass = etsVm.getClass('Lgeneric/UnionClass;');
const createUnionObjectFromEts = etsVm.getFunction('Lgeneric/ETSGLOBAL;', 'create_union_object_from_ets');

const AbstractClass = etsVm.getClass('Lgeneric/AbstractClass;');
const createAbstractObjectFromEts = etsVm.getFunction('Lgeneric/ETSGLOBAL;', 'create_abstract_object_from_ets');

const GIClass = etsVm.getClass('Lgeneric/GIClass;');
const createInterfaceObjectFromEts = etsVm.getFunction('Lgeneric/ETSGLOBAL;', 'create_interface_object_from_ets');

const GClass = etsVm.getClass('Lgeneric/GClass;');
const createGenericObjectFromEts = etsVm.getFunction('Lgeneric/ETSGLOBAL;', 'create_generic_object_from_ets');

const genericFunction = etsVm.getFunction('Lgeneric/ETSGLOBAL;', 'generic_function');

const tupleDeclaredType = etsVm.getFunction('Lgeneric/ETSGLOBAL;', 'tuple_declared_type');

const genericSubsetRef = etsVm.getFunction('Lgeneric/ETSGLOBAL;', 'generic_subset_ref');

const num = 1;
const bool = true;
const data = { data: 'string' };
const str = 'string';

const getIssueExtras = () => ({
	explicitlyDeclaredType: etsVm.getFunction('Lgeneric/ETSGLOBAL;', 'explicitly_declared_type'),
});

const exported = {
	UnionClass,
	createUnionObjectFromEts,
	GIClass,
	createAbstractObjectFromEts,
	GClass,
	createGenericObjectFromEts,
	genericFunction,
	tupleDeclaredType,
	genericSubsetRef,
	AbstractClass,
	createInterfaceObjectFromEts,
	num,
	bool,
	data,
	str,
};

module.exports = Object.assign(exported, false ? getIssueExtras() : {});

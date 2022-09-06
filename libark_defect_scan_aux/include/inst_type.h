/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef LIBARK_DEFECT_SCAN_AUX_INCLUDE_INST_TYPE_H
#define LIBARK_DEFECT_SCAN_AUX_INCLUDE_INST_TYPE_H

#include "compiler/optimizer/ir/inst.h"

namespace panda::defect_scan_aux {
#define OPCODE_INSTTYPE_MAP(V)                                \
    V(Neg, Neg)                                               \
    V(Abs, Abs)                                               \
    V(Sqrt, Sqrt)                                             \
    V(Not, Not)                                               \
    V(Add, Add)                                               \
    V(Sub, Sub)                                               \
    V(Mul, Mul)                                               \
    V(Div, Div)                                               \
    V(Mod, Mod)                                               \
    V(Min, Min)                                               \
    V(Max, Max)                                               \
    V(Shl, Shl)                                               \
    V(Shr, Shr)                                               \
    V(AShr, AShr)                                             \
    V(And, And)                                               \
    V(Or, Or)                                                 \
    V(Xor, Xor)                                               \
    V(Compare, Compare)                                       \
    V(Cmp, Cmp)                                               \
    V(CompareAnyType, CompareAnyType)                         \
    V(CastAnyTypeValue, CastAnyTypeValue)                     \
    V(CastValueToAnyType, CastValueToAnyType)                 \
    V(Cast, Cast)                                             \
    V(Constant, Constant)                                     \
    V(Parameter, Parameter)                                   \
    V(NullPtr, NullPtr)                                       \
    V(NewArray, NewArray)                                     \
    V(NewObject, NewObject)                                   \
    V(InitObject, InitObject)                                 \
    V(LoadArray, LoadArray)                                   \
    V(LoadCompressedStringChar, LoadCompressedStringChar)     \
    V(StoreArray, StoreArray)                                 \
    V(LoadObject, LoadObject)                                 \
    V(Load, Load)                                             \
    V(LoadI, LoadI)                                           \
    V(Store, Store)                                           \
    V(StoreI, StoreI)                                         \
    V(UnresolvedLoadObject, UnresolvedLoadObject)             \
    V(StoreObject, StoreObject)                               \
    V(UnresolvedStoreObject, UnresolvedStoreObject)           \
    V(LoadStatic, LoadStatic)                                 \
    V(UnresolvedLoadStatic, UnresolvedLoadStatic)             \
    V(StoreStatic, StoreStatic)                               \
    V(UnresolvedStoreStatic, UnresolvedStoreStatic)           \
    V(LenArray, LenArray)                                     \
    V(LoadString, LoadString)                                 \
    V(LoadConstArray, LoadConstArray)                         \
    V(FillConstArray, FillConstArray)                         \
    V(LoadType, LoadType)                                     \
    V(UnresolvedLoadType, UnresolvedLoadType)                 \
    V(CheckCast, CheckCast)                                   \
    V(IsInstance, IsInstance)                                 \
    V(InitClass, InitClass)                                   \
    V(LoadClass, LoadClass)                                   \
    V(LoadAndInitClass, LoadAndInitClass)                     \
    V(UnresolvedLoadAndInitClass, UnresolvedLoadAndInitClass) \
    V(GetInstanceClass, GetInstanceClass)                     \
    V(ClassImmediate, ClassImmediate)                         \
    V(NullCheck, NullCheck)                                   \
    V(BoundsCheck, BoundsCheck)                               \
    V(RefTypeCheck, RefTypeCheck)                             \
    V(ZeroCheck, ZeroCheck)                                   \
    V(NegativeCheck, NegativeCheck)                           \
    V(AnyTypeCheck, AnyTypeCheck)                             \
    V(Deoptimize, Deoptimize)                                 \
    V(DeoptimizeIf, DeoptimizeIf)                             \
    V(DeoptimizeCompare, DeoptimizeCompare)                   \
    V(DeoptimizeCompareImm, DeoptimizeCompareImm)             \
    V(IsMustDeoptimize, IsMustDeoptimize)                     \
    V(ReturnVoid, ReturnVoid)                                 \
    V(Return, Return)                                         \
    V(ReturnInlined, ReturnInlined)                           \
    V(Throw, Throw)                                           \
    V(IndirectJump, IndirectJump)                             \
    V(CallStatic, CallStatic)                                 \
    V(UnresolvedCallStatic, UnresolvedCallStatic)             \
    V(CallVirtual, CallVirtual)                               \
    V(UnresolvedCallVirtual, UnresolvedCallVirtual)           \
    V(CallDynamic, CallDynamic)                               \
    V(CallIndirect, CallIndirect)                             \
    V(Call, Call)                                             \
    V(MultiArray, MultiArray)                                 \
    V(Monitor, Monitor)                                       \
    V(Intrinsic, Intrinsic)                                   \
    V(Builtin, Builtin)                                       \
    V(AddI, AddI)                                             \
    V(SubI, SubI)                                             \
    V(MulI, MulI)                                             \
    V(DivI, DivI)                                             \
    V(ModI, ModI)                                             \
    V(ShlI, ShlI)                                             \
    V(ShrI, ShrI)                                             \
    V(AShrI, AShrI)                                           \
    V(AndI, AndI)                                             \
    V(OrI, OrI)                                               \
    V(XorI, XorI)                                             \
    V(MAdd, MAdd)                                             \
    V(MSub, MSub)                                             \
    V(MNeg, MNeg)                                             \
    V(OrNot, OrNot)                                           \
    V(AndNot, AndNot)                                         \
    V(XorNot, XorNot)                                         \
    V(AndSR, AndSR)                                           \
    V(OrSR, OrSR)                                             \
    V(XorSR, XorSR)                                           \
    V(AndNotSR, AndNotSR)                                     \
    V(OrNotSR, OrNotSR)                                       \
    V(XorNotSR, XorNotSR)                                     \
    V(AddSR, AddSR)                                           \
    V(SubSR, SubSR)                                           \
    V(NegSR, NegSR)                                           \
    V(BoundsCheckI, BoundsCheckI)                             \
    V(LoadArrayI, LoadArrayI)                                 \
    V(LoadCompressedStringCharI, LoadCompressedStringCharI)   \
    V(StoreArrayI, StoreArrayI)                               \
    V(LoadArrayPair, LoadArrayPair)                           \
    V(LoadArrayPairI, LoadArrayPairI)                         \
    V(LoadPairPart, LoadPairPart)                             \
    V(StoreArrayPair, StoreArrayPair)                         \
    V(StoreArrayPairI, StoreArrayPairI)                       \
    V(ReturnI, ReturnI)                                       \
    V(Phi, Phi)                                               \
    V(SpillFill, SpillFill)                                   \
    V(SaveState, SaveState)                                   \
    V(SafePoint, SafePoint)                                   \
    V(SaveStateDeoptimize, SaveStateDeoptimize)               \
    V(SaveStateOsr, SaveStateOsr)                             \
    V(Select, Select)                                         \
    V(SelectImm, SelectImm)                                   \
    V(AddOverflow, AddOverflow)                               \
    V(SubOverflow, SubOverflow)                               \
    V(AddOverflowCheck, AddOverflowCheck)                     \
    V(SubOverflowCheck, SubOverflowCheck)                     \
    V(If, If)                                                 \
    V(IfImm, IfImm)                                           \
    V(NOP, NOP)                                               \
    V(Try, Try)                                               \
    V(CatchPhi, CatchPhi)                                     \
    V(LiveIn, LiveIn)                                         \
    V(LiveOut, LiveOut)

#define INTRINSIC_INSTTYPE_MAP(V)                                                                 \
    V(ECMA_LDNAN_PREF_NONE, Intrinsic_LdNan)                                                      \
    V(ECMA_LDINFINITY_PREF_NONE, Intrinsic_LdInfinity)                                            \
    V(ECMA_LDGLOBALTHIS_PREF_NONE, Intrinsic_LdGlobalThis)                                        \
    V(ECMA_LDUNDEFINED_PREF_NONE, Intrinsic_LdUndefined)                                          \
    V(ECMA_LDNULL_PREF_NONE, Intrinsic_LdNull)                                                    \
    V(ECMA_LDSYMBOL_PREF_NONE, Intrinsic_LdSymbol)                                                \
    V(ECMA_LDGLOBAL_PREF_NONE, Intrinsic_LdGlobal)                                                \
    V(ECMA_LDTRUE_PREF_NONE, Intrinsic_LdTrue)                                                    \
    V(ECMA_LDFALSE_PREF_NONE, Intrinsic_LdFalse)                                                  \
    V(ECMA_THROWDYN_PREF_NONE, Intrinsic_ThrowDyn)                                                \
    V(ECMA_RETHROWDYN_PREF_NONE, Intrinsic_RethrowDyn)                                            \
    V(ECMA_TYPEOFDYN_PREF_NONE, Intrinsic_TypeofDyn)                                              \
    V(ECMA_LDLEXENVDYN_PREF_NONE, Intrinsic_LdLexEnvDyn)                                          \
    V(ECMA_POPLEXENVDYN_PREF_NONE, Intrinsic_PopLexEnvDyn)                                        \
    V(ECMA_GETUNMAPPEDARGS_PREF_NONE, Intrinsic_GetUnmappedArgs)                                  \
    V(ECMA_GETPROPITERATOR_PREF_NONE, Intrinsic_GetPropIterator)                                  \
    V(ECMA_ASYNCFUNCTIONENTER_PREF_NONE, Intrinsic_AsyncFunctionEnter)                            \
    V(ECMA_LDHOLE_PREF_NONE, Intrinsic_LdHole)                                                    \
    V(ECMA_RETURNUNDEFINED_PREF_NONE, Intrinsic_ReturnUndefined)                                  \
    V(ECMA_CREATEEMPTYOBJECT_PREF_NONE, Intrinsic_CreateEmptyObject)                              \
    V(ECMA_CREATEEMPTYARRAY_PREF_NONE, Intrinsic_CreateEmptyArray)                                \
    V(ECMA_GETITERATOR_PREF_V8_V8, Intrinsic_GetIterator_V8)                                      \
    V(ECMA_GETITERATOR_PREF_NONE, Intrinsic_GetIterator_None)                                     \
    V(ECMA_GETASYNCITERATOR_PREF_NONE, Intrinsic_GetAsyncIterator)                                \
    V(ECMA_THROWTHROWNOTEXISTS_PREF_NONE, Intrinsic_ThrowThrowNotExists)                          \
    V(ECMA_THROWPATTERNNONCOERCIBLE_PREF_NONE, Intrinsic_ThrowPatternNonCoercible)                \
    V(ECMA_LDHOMEOBJECT_PREF_NONE, Intrinsic_LdHomeObject)                                        \
    V(ECMA_THROWDELETESUPERPROPERTY_PREF_NONE, Intrinsic_ThrowDeleteSuperProperty)                \
    V(ECMA_DEBUGGER_PREF_NONE, Intrinsic_Debugger)                                                \
    V(ECMA_ADD2DYN_PREF_V8, Intrinsic_Add2Dyn)                                                    \
    V(ECMA_SUB2DYN_PREF_V8, Intrinsic_Sub2Dyn)                                                    \
    V(ECMA_MUL2DYN_PREF_V8, Intrinsic_Mul2Dyn)                                                    \
    V(ECMA_DIV2DYN_PREF_V8, Intrinsic_Div2Dyn)                                                    \
    V(ECMA_MOD2DYN_PREF_V8, Intrinsic_Mod2Dyn)                                                    \
    V(ECMA_EQDYN_PREF_V8, Intrinsic_EqDyn)                                                        \
    V(ECMA_NOTEQDYN_PREF_V8, Intrinsic_NotEqDyn)                                                  \
    V(ECMA_LESSDYN_PREF_V8, Intrinsic_LessDyn)                                                    \
    V(ECMA_LESSEQDYN_PREF_V8, Intrinsic_LessEqDyn)                                                \
    V(ECMA_GREATERDYN_PREF_V8, Intrinsic_GreaterDyn)                                              \
    V(ECMA_GREATEREQDYN_PREF_V8, Intrinsic_GreaterEqDyn)                                          \
    V(ECMA_SHL2DYN_PREF_V8, Intrinsic_Shl2Dyn)                                                    \
    V(ECMA_SHR2DYN_PREF_V8, Intrinsic_Shr2Dyn)                                                    \
    V(ECMA_ASHR2DYN_PREF_V8, Intrinsic_AShr2Dyn)                                                  \
    V(ECMA_AND2DYN_PREF_V8, Intrinsic_And2Dyn)                                                    \
    V(ECMA_OR2DYN_PREF_V8, Intrinsic_Or2Dyn)                                                      \
    V(ECMA_XOR2DYN_PREF_V8, Intrinsic_Xor2Dyn)                                                    \
    V(ECMA_TONUMBER_PREF_V8, Intrinsic_ToNumber)                                                  \
    V(ECMA_TONUMERIC_PREF_V8, Intrinsic_ToNumeric)                                                \
    V(ECMA_NEGDYN_PREF_V8, Intrinsic_NegDyn)                                                      \
    V(ECMA_NOTDYN_PREF_V8, Intrinsic_NotDyn)                                                      \
    V(ECMA_INCDYN_PREF_V8, Intrinsic_IncDyn)                                                      \
    V(ECMA_DECDYN_PREF_V8, Intrinsic_DecDyn)                                                      \
    V(ECMA_EXPDYN_PREF_V8, Intrinsic_ExpDyn)                                                      \
    V(ECMA_ISINDYN_PREF_V8, Intrinsic_IsInDyn)                                                    \
    V(ECMA_INSTANCEOFDYN_PREF_V8, Intrinsic_InstanceofDyn)                                        \
    V(ECMA_STRICTNOTEQDYN_PREF_V8, Intrinsic_StrictNotEqDyn)                                      \
    V(ECMA_STRICTEQDYN_PREF_V8, Intrinsic_StrictEqDyn)                                            \
    V(ECMA_RESUMEGENERATOR_PREF_V8, Intrinsic_ResumeGenerator)                                    \
    V(ECMA_GETRESUMEMODE_PREF_V8, Intrinsic_GetResumeMode)                                        \
    V(ECMA_CREATEGENERATOROBJ_PREF_V8, Intrinsic_CreateGeneratorObj)                              \
    V(ECMA_SETGENERATORSTATE_PREF_V8_IMM8, Intrinsic_SetGeneratorState)                           \
    V(ECMA_CREATEASYNCGENERATOROBJ_PREF_V8, Intrinsic_CreateAsyncGeneratorObj)                    \
    V(ECMA_THROWCONSTASSIGNMENT_PREF_V8, Intrinsic_ThrowConstAssignment)                          \
    V(ECMA_GETMETHOD_PREF_ID32_V8, Intrinsic_GetMethod)                                           \
    V(ECMA_GETTEMPLATEOBJECT_PREF_V8, Intrinsic_GetTemplateObject)                                \
    V(ECMA_GETNEXTPROPNAME_PREF_V8, Intrinsic_GetNextPropName)                                    \
    V(ECMA_CALLARG0DYN_PREF_V8, Intrinsic_CallArg0Dyn)                                            \
    V(ECMA_THROWIFNOTOBJECT_PREF_V8, Intrinsic_ThrowIfNotObject)                                  \
    V(ECMA_CLOSEITERATOR_PREF_V8, Intrinsic_CloseIterator)                                        \
    V(ECMA_COPYMODULE_PREF_V8, Intrinsic_CopyModule)                                              \
    V(ECMA_SUPERCALLSPREAD_PREF_V8, Intrinsic_SuperCallSpread)                                    \
    V(ECMA_DELOBJPROP_PREF_V8_V8, Intrinsic_DelObjProp)                                           \
    V(ECMA_NEWOBJSPREADDYN_PREF_V8_V8, Intrinsic_NewObjSpreadDyn)                                 \
    V(ECMA_CREATEITERRESULTOBJ_PREF_V8_V8, Intrinsic_CreateIterResultObj)                         \
    V(ECMA_SUSPENDGENERATOR_PREF_V8_V8, Intrinsic_SuspendGenerator)                               \
    V(ECMA_SUSPENDASYNCGENERATOR_PREF_V8, Intrinsic_SuspendAsyncGenerator)                        \
    V(ECMA_ASYNCFUNCTIONAWAITUNCAUGHT_PREF_V8_V8, Intrinsic_AsyncFunctionAwaitUncaught)           \
    V(ECMA_THROWUNDEFINEDIFHOLE_PREF_V8_V8, Intrinsic_ThrowUndefinedIfHole)                       \
    V(ECMA_CALLARG1DYN_PREF_V8_V8, Intrinsic_CallArg1Dyn)                                         \
    V(ECMA_COPYDATAPROPERTIES_PREF_V8_V8, Intrinsic_CopyDataProperties)                           \
    V(ECMA_STARRAYSPREAD_PREF_V8_V8, Intrinsic_StArraySpread)                                     \
    V(ECMA_SETOBJECTWITHPROTO_PREF_V8_V8, Intrinsic_SetObjectWithProto)                           \
    V(ECMA_LDOBJBYVALUE_PREF_V8_V8, Intrinsic_LdObjByValue)                                       \
    V(ECMA_STOBJBYVALUE_PREF_V8_V8, Intrinsic_StObjByValue)                                       \
    V(ECMA_STOWNBYVALUE_PREF_V8_V8, Intrinsic_StOwnByValue)                                       \
    V(ECMA_LDSUPERBYVALUE_PREF_V8_V8, Intrinsic_LdSuperByValue)                                   \
    V(ECMA_STSUPERBYVALUE_PREF_V8_V8, Intrinsic_StSuperByValue)                                   \
    V(ECMA_LDOBJBYINDEX_PREF_IMM8_V8, Intrinsic_LdObjByIndex_Imm8)                                \
    V(ECMA_LDOBJBYINDEX_PREF_IMM16_V8, Intrinsic_LdObjByIndex_Imm16)                              \
    V(ECMA_LDOBJBYINDEX_PREF_V8_IMM32, Intrinsic_LdObjByIndex_Imm32)                              \
    V(ECMA_STOBJBYINDEX_PREF_IMM8_V8, Intrinsic_StObjByIndex_Imm8)                                \
    V(ECMA_STOBJBYINDEX_PREF_IMM16_V8, Intrinsic_StObjByIndex_Imm16)                              \
    V(ECMA_STOBJBYINDEX_PREF_V8_IMM32, Intrinsic_StObjByIndex_Imm32)                              \
    V(ECMA_STOWNBYINDEX_PREF_IMM8_V8, Intrinsic_StOwnByIndex_Imm8)                                \
    V(ECMA_STOWNBYINDEX_PREF_IMM16_V8, Intrinsic_StOwnByIndex_Imm16)                              \
    V(ECMA_STOWNBYINDEX_PREF_V8_IMM32, Intrinsic_StOwnByIndex_Imm32)                              \
    V(ECMA_CALLSPREADDYN_PREF_V8_V8_V8, Intrinsic_CallSpreadDyn)                                  \
    V(ECMA_ASYNCFUNCTIONRESOLVE_PREF_V8_V8_V8, Intrinsic_AsyncFunctionResolve)                    \
    V(ECMA_ASYNCFUNCTIONREJECT_PREF_V8_V8_V8, Intrinsic_AsyncFunctionReject)                      \
    V(ECMA_ASYNCGENERATORRESOLVE_PREF_V8, Intrinsic_AsyncGeneratorResolve)                        \
    V(ECMA_ASYNCGENERATORREJECT_PREF_V8, Intrinsic_AsyncGeneratorReject)                          \
    V(ECMA_CALLARGS2DYN_PREF_V8_V8_V8, Intrinsic_CallArgs2Dyn)                                    \
    V(ECMA_CALLARGS3DYN_PREF_V8_V8_V8_V8, Intrinsic_CallArgs3Dyn)                                 \
    V(ECMA_DEFINEGETTERSETTERBYVALUE_PREF_V8_V8_V8_V8, Intrinsic_DefineGetterSetterByValue)       \
    V(ECMA_NEWOBJDYNRANGE_PREF_IMM16_V8, Intrinsic_NewObjDynRange)                                \
    V(ECMA_CALLIRANGEDYN_PREF_IMM16_V8, Intrinsic_CallIRangeDyn)                                  \
    V(ECMA_CALLITHISRANGEDYN_PREF_IMM16_V8, Intrinsic_CallIThisRangeDyn)                          \
    V(ECMA_SUPERCALL_PREF_IMM16_V8, Intrinsic_SuperCall)                                          \
    V(ECMA_CREATEOBJECTWITHEXCLUDEDKEYS_PREF_IMM16_V8_V8, Intrinsic_CreateObjectWithExcludedKeys) \
    V(ECMA_DEFINEFUNCDYN_PREF_ID16_IMM16_V8, Intrinsic_DefineFuncDyn)                             \
    V(ECMA_DEFINENCFUNCDYN_PREF_ID16_IMM16_V8, Intrinsic_DefineNcFuncDyn)                         \
    V(ECMA_DEFINEGENERATORFUNC_PREF_ID16_IMM16_V8, Intrinsic_DefineGeneratorFunc)                 \
    V(ECMA_DEFINEASYNCFUNC_PREF_ID16_IMM16_V8, Intrinsic_DefineAsyncFunc)                         \
    V(ECMA_DEFINEASYNCGENERATORFUNC_PREF_ID16_V8, Intrinsic_DefineAsyncGeneratorFunc)             \
    V(ECMA_DEFINEMETHOD_PREF_ID16_IMM16_V8, Intrinsic_DefineMethod)                               \
    V(ECMA_NEWLEXENVDYN_PREF_IMM16, Intrinsic_NewLexEnvDyn)                                       \
    V(ECMA_COPYLEXENVDYN_PREF_NONE, Intrinsic_CopyLexEnvDyn)                                      \
    V(ECMA_COPYRESTARGS_PREF_IMM16, Intrinsic_CopyRestArgs)                                       \
    V(ECMA_CREATEARRAYWITHBUFFER_PREF_IMM16, Intrinsic_CreateArrayWithBuffer)                     \
    V(ECMA_CREATEOBJECTHAVINGMETHOD_PREF_IMM16, Intrinsic_CreateObjectHavingMethod)               \
    V(ECMA_THROWIFSUPERNOTCORRECTCALL_PREF_IMM16, Intrinsic_ThrowIfSuperNotCorrectCall)           \
    V(ECMA_CREATEOBJECTWITHBUFFER_PREF_IMM16, Intrinsic_CreateObjectWithBuffer)                   \
    V(ECMA_LDLEXVARDYN_PREF_IMM4_IMM4, Intrinsic_LdLexVarDyn_Imm4)                                \
    V(ECMA_LDLEXVARDYN_PREF_IMM8_IMM8, Intrinsic_LdLexVarDyn_Imm8)                                \
    V(ECMA_LDLEXVARDYN_PREF_IMM16_IMM16, Intrinsic_LdLexVarDyn_Imm16)                             \
    V(ECMA_STLEXVARDYN_PREF_IMM4_IMM4_V8, Intrinsic_StLexVarDyn_Imm4)                             \
    V(ECMA_STLEXVARDYN_PREF_IMM8_IMM8_V8, Intrinsic_StLexVarDyn_Imm8)                             \
    V(ECMA_STLEXVARDYN_PREF_IMM16_IMM16_V8, Intrinsic_StLexVarDyn_Imm16)                          \
    V(ECMA_DEFINECLASSWITHBUFFER_PREF_ID16_IMM16_IMM16_V8_V8, Intrinsic_DefineClassWithBuffer)    \
    V(ECMA_IMPORTMODULE_PREF_ID32, Intrinsic_ImportModule)                                        \
    V(ECMA_STMODULEVAR_PREF_ID32, Intrinsic_StModuleVar)                                          \
    V(ECMA_TRYLDGLOBALBYNAME_PREF_ID32, Intrinsic_TryLdGlobalByName)                              \
    V(ECMA_TRYSTGLOBALBYNAME_PREF_ID32, Intrinsic_TryStGlobalByName)                              \
    V(ECMA_LDGLOBALVAR_PREF_ID32, Intrinsic_LdGlobalVar)                                          \
    V(ECMA_STGLOBALVAR_PREF_ID32, Intrinsic_StGlobalVar)                                          \
    V(ECMA_STGLOBALLET_PREF_ID32, Intrinsic_StGlobalLet)                                          \
    V(ECMA_LDOBJBYNAME_PREF_ID32_V8, Intrinsic_LdObjByName)                                       \
    V(ECMA_STOBJBYNAME_PREF_ID32_V8, Intrinsic_StObjByName)                                       \
    V(ECMA_STOWNBYNAME_PREF_ID32_V8, Intrinsic_StOwnByName)                                       \
    V(ECMA_LDSUPERBYNAME_PREF_ID32_V8, Intrinsic_LdSuperByName)                                   \
    V(ECMA_STSUPERBYNAME_PREF_ID32_V8, Intrinsic_StSuperByName)                                   \
    V(ECMA_LDMODVARBYNAME_PREF_ID32_V8, Intrinsic_LdModVarByName)                                 \
    V(ECMA_TOBOOLEAN_PREF_NONE, Intrinsic_ToBoolean)                                              \
    V(ECMA_NEGATE_PREF_NONE, Intrinsic_Negate)                                                    \
    V(ECMA_ISTRUE_PREF_NONE, Intrinsic_IsTrue)                                                    \
    V(ECMA_ISFALSE_PREF_NONE, Intrinsic_IsFalse)                                                  \
    V(ECMA_ISUNDEFINED_PREF_NONE, Intrinsic_IsUndefined)                                          \
    V(ECMA_ISCOERCIBLE_PREF_NONE, Intrinsic_IsCoercible)                                          \
    V(ECMA_JFALSE_PREF_IMM8, Intrinsic_JFalse_Imm8)                                               \
    V(ECMA_JFALSE_PREF_IMM16, Intrinsic_JFalse_Imm16)                                             \
    V(ECMA_JFALSE_PREF_IMM32, Intrinsic_JFalse_Imm32)                                             \
    V(ECMA_JTRUE_PREF_IMM8, Intrinsic_JTrue_Imm8)                                                 \
    V(ECMA_JTRUE_PREF_IMM16, Intrinsic_JTrue_Imm16)                                               \
    V(ECMA_JTRUE_PREF_IMM32, Intrinsic_JTrue_Imm32)                                               \
    V(ECMA_RETURN_DYN_PREF_NONE, Intrinsic_ReturnDyn)                                             \
    V(ECMA_ITERNEXT_PREF_V8, Intrinsic_IterNext)                                                  \
    V(ECMA_GETMODULENAMESPACE_PREF_ID32, Intrinsic_GetModuleNamespace)                            \
    V(ECMA_LDMODULEVAR_PREF_ID32_IMM8, Intrinsic_LdModuleVar)                                     \
    V(ECMA_STCONSTTOGLOBALRECORD_PREF_ID32, Intrinsic_StConstToGlobalRecord)                      \
    V(ECMA_STLETTOGLOBALRECORD_PREF_ID32, Intrinsic_StLetToGlobalRecord)                          \
    V(ECMA_GETITERATORNEXT_PREF_V8_V8, Intrinsic_GetIteratorNext)                                 \
    V(ECMA_CREATEREGEXPWITHLITERAL_PREF_ID32_IMM8, Intrinsic_CreateRegexpWithLiteral)             \
    V(ECMA_STCLASSTOGLOBALRECORD_PREF_ID32, Intrinsic_StClassToGlobalRecord)                      \
    V(ECMA_STOWNBYVALUEWITHNAMESET_PREF_V8_V8, Intrinsic_StOwnByValueWithNameSet)                 \
    V(ECMA_LDFUNCTION_PREF_NONE, Intrinsic_LdFunction)                                            \
    V(ECMA_NEWLEXENVWITHNAMEDYN_PREF_IMM16_IMM16, Intrinsic_NewLexEnvWithNameDyn)                 \
    V(ECMA_LDBIGINT_PREF_ID32, Intrinsic_LdBigInt)                                                \
    V(ECMA_STOWNBYNAMEWITHNAMESET_PREF_ID32_V8, Intrinsic_StOwnByNameWithNameSet)

#define INSTTYPE_ENUM(x, y) y,
#define BUILD_OPCODE_MAP(x, y) {Opcode::x, InstType::y},
#define BUILD_INTRINSIC_MAP(x, y) {IntrinsicId::x, InstType::y},

using Opcode = compiler::Opcode;
using IntrinsicId = compiler::RuntimeInterface::IntrinsicId;
enum class InstType {
    OPCODE_INSTTYPE_MAP(INSTTYPE_ENUM) INTRINSIC_INSTTYPE_MAP(INSTTYPE_ENUM) COUNT,
    INVALID_TYPE,
};
}  // namespace panda::defect_scan_aux
#endif  // LIBARK_DEFECT_SCAN_AUX_INCLUDE_INST_TYPE_H
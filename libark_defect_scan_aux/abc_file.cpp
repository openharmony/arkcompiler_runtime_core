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

#include "abc_file.h"
#include "libpandabase/mem/pool_manager.h"
#include "libpandafile/class_data_accessor-inl.h"
#include "libpandafile/code_data_accessor-inl.h"
#include "libpandafile/method_data_accessor-inl.h"
#include "libpandafile/literal_data_accessor-inl.h"
#include "libpandafile/field_data_accessor-inl.h"
#include "libpandafile/module_data_accessor-inl.h"
#include "compiler/optimizer/ir_builder/ir_builder.h"
#include "bytecode_optimizer/runtime_adapter.h"
#include "callee_info.h"
#include "class.h"
#include "function.h"
#include "module_record.h"

namespace panda::defect_scan_aux {
using EntityId = panda_file::File::EntityId;
using StringData = panda_file::StringData;
using LiteralTag = panda_file::LiteralTag;
using ModuleDataAccessor = panda_file::ModuleDataAccessor;
using ModuleTag = panda_file::ModuleTag;

AbcFile::AbcFile(std::string_view filename, std::unique_ptr<const panda_file::File> &&panda_file)
    : filename_(filename), panda_file_(std::forward<std::unique_ptr<const panda_file::File>>(panda_file))
{
    PoolManager::Initialize(PoolType::MALLOC);
    allocator_ = std::make_unique<ArenaAllocator>(SpaceType::SPACE_TYPE_COMPILER);
    local_allocator_ = std::make_unique<ArenaAllocator>(SpaceType::SPACE_TYPE_COMPILER, nullptr, true);
}

AbcFile::~AbcFile()
{
    PoolManager::Finalize();
}

std::unique_ptr<const AbcFile> AbcFile::Open(std::string_view abc_filename)
{
    auto panda_file = panda_file::OpenPandaFile(abc_filename);
    if (panda_file == nullptr) {
        LOG(ERROR, DEFECT_SCAN_AUX) << "Can not open binary file '" << abc_filename << "'";
        return nullptr;
    }

    std::unique_ptr<AbcFile> abc_file(new (std::nothrow) AbcFile(abc_filename, std::move(panda_file)));
    if (abc_file == nullptr) {
        LOG(ERROR, DEFECT_SCAN_AUX) << "Can not create AbcFile instance for '" << abc_filename << "'";
        return nullptr;
    }

    abc_file->ExtractDebugInfo();
    abc_file->ExtractModuleInfo();
    abc_file->InitializeAllDefinedFunction();
    abc_file->ExtractDefinedClassAndFunctionInfo();
    abc_file->ExtractClassAndFunctionExportList();
    return abc_file;
}

bool AbcFile::IsModule() const
{
    return module_record_ != nullptr;
}

const std::string &AbcFile::GetAbcFileName() const
{
    return filename_;
}

size_t AbcFile::GetDefinedFunctionCount() const
{
    return def_func_list_.size();
}

size_t AbcFile::GetDefinedClassCount() const
{
    return def_class_list_.size();
}

const Function *AbcFile::GetDefinedFunctionByIndex(size_t index) const
{
    ASSERT(index < def_func_list_.size());
    return def_func_list_[index].get();
}

const Function *AbcFile::GetFunctionByName(std::string_view func_name) const
{
    return GetFunctionByNameImpl(func_name);
}

const Function *AbcFile::GetExportFunctionByExportName(std::string_view export_func_name) const
{
    if (!IsModule()) {
        return nullptr;
    }

    std::string inter_func_name = GetInternalNameByExportName(export_func_name);
    for (auto export_func : export_func_list_) {
        const std::string &ex_func_name = export_func->GetFunctionName();
        std::string_view no_hashtag_name = GetNameWithoutHashtag(ex_func_name);
        if (no_hashtag_name == inter_func_name) {
            return export_func;
        }
    }
    return nullptr;
}

const Class *AbcFile::GetDefinedClassByIndex(size_t index) const
{
    ASSERT(index < def_class_list_.size());
    return def_class_list_[index].get();
}

const Class *AbcFile::GetClassByName(std::string_view class_name) const
{
    return GetClassByNameImpl(class_name);
}

const Class *AbcFile::GetExportClassByExportName(std::string_view export_class_name) const
{
    if (!IsModule()) {
        return nullptr;
    }

    std::string inter_class_name = GetInternalNameByExportName(export_class_name);
    for (auto export_class : export_class_list_) {
        const std::string &ex_class_name = export_class->GetClassName();
        std::string_view no_hashtag_name = GetNameWithoutHashtag(ex_class_name);
        if (no_hashtag_name == inter_class_name) {
            return export_class;
        }
    }
    return nullptr;
}

ssize_t AbcFile::GetLineNumberByInst(const Function *func, const Inst &inst) const
{
    auto &line_number_table = debug_info_->GetLineNumberTable(func->GetMethodId());
    if (!line_number_table.empty()) {
        uint32_t inst_pc = inst.GetPc();
        // line_number_table is in ascending order, find the element that satisfies e1.pc <= inst_pc < e2.pc
        auto comp = [](size_t value, const panda_file::LineTableEntry &entry) { return value >= entry.offset; };
        auto iter = std::upper_bound(line_number_table.rbegin(), line_number_table.rend(), inst_pc, comp);
        if (iter != line_number_table.rend()) {
            // line number written in a .abc file starts from 0
            return iter->line + 1;
        }
    }
    return -1;
}

std::string AbcFile::GetInternalNameByExportName(std::string_view export_name) const
{
    if (!IsModule()) {
        return EMPTY_STR;
    }
    return module_record_->GetInternalNameByExportName(export_name);
}

std::string AbcFile::GetImportNameByExportName(std::string_view export_name) const
{
    if (!IsModule()) {
        return EMPTY_STR;
    }
    return module_record_->GetImportNameByExportName(export_name);
}

std::string AbcFile::GetModuleNameByExportName(std::string_view export_name) const
{
    if (!IsModule()) {
        return EMPTY_STR;
    }
    return module_record_->GetModuleNameByExportName(export_name);
}

std::string AbcFile::GetModuleNameByInternalName(std::string_view local_name) const
{
    if (!IsModule()) {
        return EMPTY_STR;
    }
    return module_record_->GetModuleNameByInternalName(local_name);
}

std::string AbcFile::GetImportNameByInternalName(std::string_view local_name) const
{
    if (!IsModule()) {
        return EMPTY_STR;
    }
    return module_record_->GetImportNameByInternalName(local_name);
}

std::string_view AbcFile::GetNameWithoutHashtag(std::string_view name) const
{
    if (name[0] == '#') {
        size_t sec_hashtag_idx = name.find_first_of('#', 1);
        if (sec_hashtag_idx != std::string::npos && (sec_hashtag_idx + 1) <= name.size()) {
            return name.substr(sec_hashtag_idx + 1);
        }
    }
    return name;
}

std::string AbcFile::GetStringByInst(const Inst &inst) const
{
    auto type = inst.GetType();
    switch (type) {
        case InstType::DEFINEFUNC_IMM8_ID16_IMM8:
        case InstType::DEFINEFUNC_IMM16_ID16_IMM8:
        case InstType::DEFINEMETHOD_IMM8_ID16_IMM8:
        case InstType::DEFINEMETHOD_IMM16_ID16_IMM8:
        case InstType::DEFINECLASSWITHBUFFER_IMM8_ID16_ID16_IMM16_V8:
        case InstType::DEFINECLASSWITHBUFFER_IMM16_ID16_ID16_IMM16_V8: {
            uint32_t method_id = inst.GetImms()[1];
            return GetStringByMethodId(EntityId(method_id));
        }
        case InstType::TRYLDGLOBALBYNAME_IMM8_ID16:
        case InstType::TRYSTGLOBALBYNAME_IMM8_ID16:
        case InstType::TRYLDGLOBALBYNAME_IMM16_ID16:
        case InstType::TRYSTGLOBALBYNAME_IMM16_ID16:
        case InstType::STCONSTTOGLOBALRECORD_IMM16_ID16:
        case InstType::STTOGLOBALRECORD_IMM16_ID16:
        case InstType::LDGLOBALVAR_IMM16_ID16:
        case InstType::STGLOBALVAR_IMM16_ID16:
        case InstType::LDOBJBYNAME_IMM8_ID16:
        case InstType::LDOBJBYNAME_IMM16_ID16:
        case InstType::STOBJBYNAME_IMM8_ID16_V8:
        case InstType::STOBJBYNAME_IMM16_ID16_V8:
        case InstType::LDSUPERBYNAME_IMM8_ID16:
        case InstType::LDSUPERBYNAME_IMM16_ID16:
        case InstType::STSUPERBYNAME_IMM8_ID16_V8:
        case InstType::STSUPERBYNAME_IMM16_ID16_V8:
        case InstType::LDTHISBYNAME_IMM8_ID16:
        case InstType::LDTHISBYNAME_IMM16_ID16:
        case InstType::STTHISBYNAME_IMM8_ID16:
        case InstType::STTHISBYNAME_IMM16_ID16:
        case InstType::STOWNBYNAME_IMM8_ID16_V8:
        case InstType::STOWNBYNAME_IMM16_ID16_V8:
        case InstType::STOWNBYNAMEWITHNAMESET_IMM8_ID16_V8:
        case InstType::STOWNBYNAMEWITHNAMESET_IMM16_ID16_V8: {
            uint32_t string_id = inst.GetImms()[1];
            return GetStringByStringId(EntityId(string_id));
        }
        default:
            return EMPTY_STR;
    }
}

// TODO(wangyantian): may match multiple stlex inst when considering control flow
std::optional<FuncInstPair> AbcFile::GetStLexInstByLdLexInst(FuncInstPair func_inst_pair) const
{
    const Function *func = func_inst_pair.first;
    const Inst &ld_lex_inst = func_inst_pair.second;
    if (func == nullptr || !ld_lex_inst.IsInstLdLexVar()) {
        return std::nullopt;
    }

    auto ld_imms = ld_lex_inst.GetImms();
    uint32_t ld_level = ld_imms[0];
    uint32_t ld_slot_id = ld_imms[1];
    const Function *cur_func = func;
    uint32_t i = 0;
    while (true) {
        bool has_new_lexenv = false;
        const auto &graph = cur_func->GetGraph();
        graph.VisitAllInstructions([&has_new_lexenv](const Inst &inst) {
            if (inst.GetType() == InstType::NEWLEXENV_IMM8 || inst.GetType() == InstType::WIDE_NEWLEXENV_PREF_IMM16) {
                has_new_lexenv = true;
            }
        });
        if (has_new_lexenv) {
            i++;
        }
        if (i == ld_level + 1) {
            break;
        }
        cur_func = cur_func->GetParentFunction();
        if (cur_func == nullptr) {
            return std::nullopt;
        }
    }
    auto &graph = cur_func->GetGraph();
    Inst st_lex_inst = ld_lex_inst;
    graph.VisitAllInstructions([ld_slot_id, &st_lex_inst](const Inst &inst) {
        if (inst.IsInstStLexVar()) {
            auto st_imms = inst.GetImms();
            uint32_t st_level = st_imms[0];
            uint32_t st_slot_id = st_imms[1];
            if (st_level == 0 && st_slot_id == ld_slot_id) {
                st_lex_inst = inst;
            }
        }
    });
    if (st_lex_inst != ld_lex_inst) {
        return FuncInstPair(cur_func, st_lex_inst);
    }

    return std::nullopt;
}

std::optional<FuncInstPair> AbcFile::GetStGlobalInstByLdGlobalInst(FuncInstPair func_inst_pair) const
{
    const Function *func = func_inst_pair.first;
    const Inst &ld_global_inst = func_inst_pair.second;
    if (func == nullptr || !ld_global_inst.IsInstLdGlobal()) {
        return std::nullopt;
    }

    uint32_t ld_str_id = ld_global_inst.GetImms()[0];
    // TODO(wangyantian): only consider that func_main_0 has StGlobal inst for now, what about other cases?
    const Function *func_main = def_func_list_[0].get();
    auto &graph = func_main->GetGraph();
    Inst st_global_inst = ld_global_inst;
    graph.VisitAllInstructions([ld_str_id, &st_global_inst](const Inst &inst) {
        if (inst.IsInstStGlobal()) {
            uint32_t st_str_id = inst.GetImms()[0];
            if (st_str_id == ld_str_id) {
                st_global_inst = inst;
            }
        }
    });
    if (st_global_inst != ld_global_inst) {
        return FuncInstPair(func_main, st_global_inst);
    }

    return std::nullopt;
}

void AbcFile::ExtractDebugInfo()
{
    debug_info_ = std::make_unique<const panda_file::DebugInfoExtractor>(panda_file_.get());
    if (debug_info_ == nullptr) {
        LOG(FATAL, DEFECT_SCAN_AUX) << "Failed to extract debug info";
    }
}

void AbcFile::ExtractModuleInfo()
{
    int module_offset = -1;
    for (uint32_t id : panda_file_->GetClasses()) {
        EntityId class_id(id);
        if (panda_file_->IsExternal(class_id)) {
            continue;
        }
        panda_file::ClassDataAccessor cda(*panda_file_, class_id);
        const char *desc = utf::Mutf8AsCString(cda.GetDescriptor());
        if (std::strcmp(MODULE_CLASS, desc) == 0) {
            cda.EnumerateFields([&](panda_file::FieldDataAccessor &field_accessor) -> void {
                EntityId field_name_id = field_accessor.GetNameId();
                StringData sd = panda_file_->GetStringData(field_name_id);
                if (std::strcmp(utf::Mutf8AsCString(sd.data), filename_.data())) {
                    module_offset = field_accessor.GetValue<int32_t>().value();
                    return;
                }
            });
            break;
        }
    }
    if (module_offset == -1) {
        return;
    }

    std::unique_ptr<ModuleRecord> module_record = std::make_unique<ModuleRecord>(filename_);
    if (module_record == nullptr) {
        LOG(FATAL, DEFECT_SCAN_AUX) << "Can not create ModuleRecord instance for '" << filename_ << "'";
    }
    ExtractModuleRecord(EntityId(module_offset), module_record);
    module_record_ = std::move(module_record);
}

void AbcFile::ExtractModuleRecord(EntityId module_id, std::unique_ptr<ModuleRecord> &module_record)
{
    ModuleDataAccessor mda(*panda_file_, module_id);
    const std::vector<uint32_t> &request_modules_idx = mda.getRequestModules();
    std::vector<std::string> request_modules;
    for (size_t idx = 0; idx < request_modules_idx.size(); ++idx) {
        request_modules.push_back(GetStringByStringId(EntityId(request_modules_idx[idx])));
    }
    module_record->SetRequestModules(request_modules);

    size_t regular_import_num = 0;
    size_t local_export_num = 0;
    mda.EnumerateModuleRecord([&](const ModuleTag &tag, uint32_t export_name_offset, uint32_t module_request_idx,
                                  uint32_t import_name_offset, uint32_t local_name_offset) {
        size_t request_num = request_modules.size();
        ASSERT(request_num == 0 || module_request_idx < request_num);
        std::string module_request = EMPTY_STR;
        if (request_num != 0) {
            module_request = request_modules[module_request_idx];
        }
        switch (tag) {
            case ModuleTag::REGULAR_IMPORT: {
                ++regular_import_num;
                std::string local_name = GetStringByStringId(EntityId(local_name_offset));
                std::string import_name = GetStringByStringId(EntityId(import_name_offset));
                module_record->AddImportEntry({module_request, import_name, local_name});
                LOG(DEBUG, DEFECT_SCAN_AUX) << "ModuleRecord adds a regular import: [" << module_request << ", "
                                            << import_name << ", " << local_name << "]";
                break;
            }
            case ModuleTag::NAMESPACE_IMPORT: {
                std::string local_name = GetStringByStringId(EntityId(local_name_offset));
                module_record->AddImportEntry({module_request, "*", local_name});
                LOG(DEBUG, DEFECT_SCAN_AUX)
                    << "ModuleRecord adds a namespace import: [" << module_request << ", *, " << local_name << "]";
                break;
            }
            case ModuleTag::LOCAL_EXPORT: {
                ++local_export_num;
                std::string local_name = GetStringByStringId(EntityId(local_name_offset));
                std::string export_name = GetStringByStringId(EntityId(export_name_offset));
                module_record->AddExportEntry({export_name, EMPTY_STR, EMPTY_STR, local_name});
                LOG(DEBUG, DEFECT_SCAN_AUX)
                    << "ModuleRecord adds a local export: [" << export_name << ", null, null, " << local_name << "]";
                break;
            }
            case ModuleTag::INDIRECT_EXPORT: {
                std::string export_name = GetStringByStringId(EntityId(export_name_offset));
                std::string import_name = GetStringByStringId(EntityId(import_name_offset));
                module_record->AddExportEntry({export_name, module_request, import_name, EMPTY_STR});
                LOG(DEBUG, DEFECT_SCAN_AUX) << "ModuleRecord adds an indirect export: [" << export_name << ", "
                                            << module_request << ", " << import_name << ", null]";
                break;
            }
            case ModuleTag::STAR_EXPORT: {
                module_record->AddExportEntry({EMPTY_STR, module_request, "*", EMPTY_STR});
                LOG(DEBUG, DEFECT_SCAN_AUX) << "ModuleRecord adds a start export: ["
                                            << "null, " << module_request << "*, null]";
                break;
            }
            default: {
                UNREACHABLE();
                break;
            }
        }
    });
    module_record->SetRegularImportNum(regular_import_num);
    module_record->SetLocalExportNum(local_export_num);
}

void AbcFile::InitializeAllDefinedFunction()
{
    for (uint32_t id : panda_file_->GetClasses()) {
        EntityId class_id {id};
        if (panda_file_->IsExternal(class_id)) {
            continue;
        }

        panda_file::ClassDataAccessor cda {*panda_file_, class_id};
        cda.EnumerateMethods([&](panda_file::MethodDataAccessor &mda) {
            if (!mda.IsExternal()) {
                std::string func_name = GetStringByStringId(mda.GetNameId());
                EntityId m_id = mda.GetMethodId();
                panda_file::CodeDataAccessor cda {*panda_file_, mda.GetCodeId().value()};
                uint32_t arg_count = cda.GetNumArgs();
                compiler::Graph *graph = GenerateFunctionGraph(mda, func_name);
                if (graph == nullptr) {
                    return;
                }
                std::unique_ptr<Function> func =
                    std::make_unique<Function>(func_name, m_id, arg_count, Graph(graph), this);
                if (func == nullptr) {
                    LOG(FATAL, DEFECT_SCAN_AUX) << "Can not allocate memory when processing '" << filename_ << "'";
                }
                LOG(DEBUG, DEFECT_SCAN_AUX) << "Create a new function: " << func_name;
                AddDefinedFunction(std::move(func));
            }
        });
    }
}

void AbcFile::ExtractDefinedClassAndFunctionInfo()
{
    for (auto &func : def_func_list_) {
        ExtractClassAndFunctionInfo(func.get());
    }

    std::unordered_set<const Function *> processed_func;
    for (auto &def_class : def_class_list_) {
        const Function *def_func = def_class->GetDefineFunction();
        if (def_func != nullptr && processed_func.count(def_func) == 0) {
            ExtractClassInheritInfo(def_func);
            processed_func.insert(def_func);
        }
    }

    for (auto &func : def_func_list_) {
        ExtractFunctionCalleeInfo(func.get());
    }
}

void AbcFile::ExtractClassAndFunctionInfo(Function *func)
{
    auto &graph = func->GetGraph();
    graph.VisitAllInstructions([&](const Inst &inst) {
        auto type = inst.GetType();
        switch (type) {
            case InstType::DEFINECLASSWITHBUFFER_IMM8_ID16_ID16_IMM16_V8:
            case InstType::DEFINECLASSWITHBUFFER_IMM16_ID16_ID16_IMM16_V8: {
                auto def_class = ResolveDefineClassWithBufferInst(func, inst);
                AddDefinedClass(std::move(def_class));
                break;
            }
            case InstType::DEFINEFUNC_IMM8_ID16_IMM8:
            case InstType::DEFINEFUNC_IMM16_ID16_IMM8: {
                Function *def_func = ResolveDefineFuncInstCommon(func, inst);
                BuildFunctionDefineChain(func, def_func);
                break;
            }
            case InstType::DEFINEMETHOD_IMM8_ID16_IMM8:
            case InstType::DEFINEMETHOD_IMM16_ID16_IMM8: {
                auto member_func = ResolveDefineFuncInstCommon(func, inst);
                BuildFunctionDefineChain(func, member_func);
                // resolve the class where it's defined
                Inst def_method_input0 = inst.GetInputInsts()[0];
                Inst ld_obj_input0 = def_method_input0.GetInputInsts()[0];
                if ((def_method_input0.GetType() == InstType::LDOBJBYNAME_IMM8_ID16 ||
                     def_method_input0.GetType() == InstType::LDOBJBYNAME_IMM16_ID16) &&
                    GetStringByInst(def_method_input0) == PROTOTYPE &&
                    (ld_obj_input0.GetType() == InstType::DEFINECLASSWITHBUFFER_IMM8_ID16_ID16_IMM16_V8 ||
                     ld_obj_input0.GetType() == InstType::DEFINECLASSWITHBUFFER_IMM16_ID16_ID16_IMM16_V8)) {
                    auto clazz = GetClassByNameImpl(GetStringByInst(ld_obj_input0));
                    if (clazz != nullptr) {
                        BuildClassAndMemberFuncRelation(clazz, member_func);
                    }
                }
                break;
            }
            default:
                break;
        }
    });
}

void AbcFile::ExtractClassInheritInfo(const Function *func) const
{
    auto &graph = func->GetGraph();
    graph.VisitAllInstructions([&](const Inst &inst) {
        if (inst.GetType() != InstType::DEFINECLASSWITHBUFFER_IMM8_ID16_ID16_IMM16_V8 &&
            inst.GetType() != InstType::DEFINECLASSWITHBUFFER_IMM16_ID16_ID16_IMM16_V8) {
            return;
        }

        Class *cur_class = GetClassByNameImpl(GetStringByInst(inst));
        ASSERT(cur_class != nullptr);
        Inst def_class_input1 = inst.GetInputInsts()[0];
        auto [ret_ptr, ret_sym, ret_type] = ResolveInstCommon(func, def_class_input1);
        if (ret_ptr != nullptr && ret_type == ResolveType::CLASS_OBJECT) {
            auto par_class = reinterpret_cast<const Class *>(ret_ptr);
            cur_class->SetParentClass(par_class);
            return;
        }
        size_t first_delim_idx = ret_sym.find_first_of(DELIM);
        size_t last_delim_idx = ret_sym.find_last_of(DELIM);
        std::string par_class_name = ret_sym;
        std::string var_name = EMPTY_STR;
        if (last_delim_idx != std::string::npos) {
            par_class_name = ret_sym.substr(last_delim_idx + 1);
            var_name = ret_sym.substr(0, first_delim_idx);
            cur_class->SetParentClassName(par_class_name);
        }
        if (ret_type == ResolveType::UNRESOLVED_MODULE) {
            std::string imp_par_class_name = GetImportNameByInternalName(par_class_name);
            if (!imp_par_class_name.empty()) {
                cur_class->SetParentClassName(imp_par_class_name);
            }
            std::string inter_name = var_name.empty() ? par_class_name : var_name;
            std::string module_name = GetModuleNameByInternalName(inter_name);
            if (!module_name.empty()) {
                cur_class->SetParClassExternalModuleName(module_name);
            }
        }
        if (ret_type == ResolveType::UNRESOLVED_GLOBAL_VAR) {
            cur_class->SetParentClassName(par_class_name);
            var_name = var_name.empty() ? var_name : ret_sym.substr(0, last_delim_idx);
            cur_class->SetParClassGlobalVarName(var_name);
        }
    });
}

void AbcFile::ExtractFunctionCalleeInfo(Function *func)
{
    auto &graph = func->GetGraph();
    graph.VisitAllInstructions([&](const Inst &inst) {
        std::unique_ptr<CalleeInfo> callee_info {nullptr};
        switch (inst.GetType()) {
            case InstType::CALLARG0_IMM8: {
                callee_info = ResolveCallInstCommon(func, inst);
                callee_info->SetCalleeArgCount(0);
                break;
            }
            case InstType::CALLARG1_IMM8_V8: {
                callee_info = ResolveCallInstCommon(func, inst, 1);
                callee_info->SetCalleeArgCount(1);
                break;
            }
            case InstType::CALLARGS2_IMM8_V8_V8: {
                constexpr int ARG_COUNT = 2;
                callee_info = ResolveCallInstCommon(func, inst, ARG_COUNT);
                callee_info->SetCalleeArgCount(ARG_COUNT);
                break;
            }
            case InstType::CALLARGS3_IMM8_V8_V8_V8: {
                constexpr int ARG_COUNT = 3;
                callee_info = ResolveCallInstCommon(func, inst, ARG_COUNT);
                callee_info->SetCalleeArgCount(ARG_COUNT);
                break;
            }
            case InstType::CALLRANGE_IMM8_IMM8_V8: {
                uint32_t arg_count = inst.GetImms()[1];
                callee_info = ResolveCallInstCommon(func, inst, arg_count);
                callee_info->SetCalleeArgCount(arg_count);
                break;
            }
            case InstType::WIDE_CALLRANGE_PREF_IMM16_V8: {
                uint32_t arg_count = inst.GetImms()[0];
                callee_info = ResolveCallInstCommon(func, inst, arg_count);
                callee_info->SetCalleeArgCount(arg_count);
                break;
            }
            case InstType::SUPERCALLSPREAD_IMM8_V8: {
                callee_info = ResolveCallInstCommon(func, inst, 1);
                break;
            }
            case InstType::APPLY_IMM8_V8_V8: {
                constexpr uint32_t FUNC_OBJ_INDEX = 2;
                callee_info = ResolveCallInstCommon(func, inst, FUNC_OBJ_INDEX);
                break;
            }
            case InstType::CALLTHIS0_IMM8_V8: {
                callee_info = ResolveCallInstCommon(func, inst, 1);
                callee_info->SetCalleeArgCount(0);
                break;
            }
            case InstType::CALLTHIS1_IMM8_V8_V8: {
                constexpr int ARG_COUNT = 1;
                // 1 represents the this pointer
                callee_info = ResolveCallInstCommon(func, inst, ARG_COUNT + 1);
                callee_info->SetCalleeArgCount(ARG_COUNT);
                break;
            }
            case InstType::CALLTHIS2_IMM8_V8_V8_V8: {
                constexpr int ARG_COUNT = 2;
                callee_info = ResolveCallInstCommon(func, inst, ARG_COUNT + 1);
                callee_info->SetCalleeArgCount(ARG_COUNT);
                break;
            }
            case InstType::CALLTHIS3_IMM8_V8_V8_V8_V8: {
                constexpr int ARG_COUNT = 3;
                callee_info = ResolveCallInstCommon(func, inst, ARG_COUNT + 1);
                callee_info->SetCalleeArgCount(ARG_COUNT);
                break;
            }
            case InstType::CALLTHISRANGE_IMM8_IMM8_V8: {
                uint32_t arg_count = inst.GetImms()[1];
                callee_info = ResolveCallInstCommon(func, inst, arg_count + 1);
                callee_info->SetCalleeArgCount(arg_count);
                break;
            }
            case InstType::WIDE_CALLTHISRANGE_PREF_IMM16_V8: {
                uint32_t arg_count = inst.GetImms()[0];
                callee_info = ResolveCallInstCommon(func, inst, arg_count + 1);
                callee_info->SetCalleeArgCount(arg_count);
                break;
            }
            case InstType::SUPERCALLTHISRANGE_IMM8_IMM8_V8:
            case InstType::SUPERCALLARROWRANGE_IMM8_IMM8_V8: {
                uint32_t arg_count = inst.GetImms()[1];
                callee_info = ResolveSuperCallInst(func, inst);
                callee_info->SetCalleeArgCount(arg_count);
                break;
            }
            case InstType::WIDE_SUPERCALLTHISRANGE_PREF_IMM16_V8:
            case InstType::WIDE_SUPERCALLARROWRANGE_PREF_IMM16_V8: {
                uint32_t arg_count = inst.GetImms()[0];
                callee_info = ResolveSuperCallInst(func, inst);
                callee_info->SetCalleeArgCount(arg_count);
                break;
            }
            default:
                break;
        }
        if (callee_info != nullptr) {
            AddCalleeInfo(std::move(callee_info));
        }
    });
}

void AbcFile::BuildFunctionDefineChain(Function *parent_func, Function *child_func) const
{
    if (parent_func == nullptr || child_func == nullptr || child_func->GetParentFunction() == parent_func) {
        return;
    }
    child_func->SetParentFunction(parent_func);
    parent_func->AddDefinedFunction(child_func);
}

void AbcFile::BuildClassAndMemberFuncRelation(Class *clazz, Function *member_func) const
{
    if (clazz == nullptr || member_func == nullptr || member_func->GetClass() == clazz) {
        return;
    }
    clazz->AddMemberFunction(member_func);
    member_func->SetClass(clazz);
}

void AbcFile::ExtractClassAndFunctionExportList()
{
    if (!IsModule() || def_func_list_.empty()) {
        return;
    }
    const Function *func_main = def_func_list_[0].get();
    ASSERT(func_main->GetFunctionName() == ENTRY_FUNCTION_NAME);
    auto &graph = func_main->GetGraph();
    graph.VisitAllInstructions([&](const Inst &inst) {
        auto type = inst.GetType();
        if (type == InstType::STMODULEVAR_IMM8 || type == InstType::WIDE_STMODULEVAR_PREF_IMM16) {
            Inst st_module_input0 = inst.GetInputInsts()[0];
            switch (st_module_input0.GetType()) {
                case InstType::DEFINEFUNC_IMM8_ID16_IMM8:
                case InstType::DEFINEFUNC_IMM16_ID16_IMM8: {
                    auto export_func = ResolveDefineFuncInstCommon(func_main, st_module_input0);
                    ASSERT(export_func != nullptr);
                    export_func_list_.push_back(export_func);
                    break;
                }
                case InstType::DEFINECLASSWITHBUFFER_IMM8_ID16_ID16_IMM16_V8:
                case InstType::DEFINECLASSWITHBUFFER_IMM16_ID16_ID16_IMM16_V8: {
                    Class *export_clazz = GetClassByNameImpl(GetStringByInst(st_module_input0));
                    ASSERT(export_clazz != nullptr);
                    export_class_list_.push_back(export_clazz);
                    break;
                }
                default:
                    break;
            }
        }
    });
}

compiler::Graph *AbcFile::GenerateFunctionGraph(const panda_file::MethodDataAccessor &mda, std::string_view func_name)
{
    panda::BytecodeOptimizerRuntimeAdapter adapter(mda.GetPandaFile());
    auto method_ptr = reinterpret_cast<compiler::RuntimeInterface::MethodPtr>(mda.GetMethodId().GetOffset());
    compiler::Graph *graph = allocator_->New<compiler::Graph>(allocator_.get(), local_allocator_.get(), Arch::NONE,
                                                              method_ptr, &adapter, false, nullptr, true, true);
    if ((graph == nullptr) || !graph->RunPass<compiler::IrBuilder>()) {
        LOG(FATAL, DEFECT_SCAN_AUX) << "Cannot generate graph for function '" << func_name << "'";
    }
    return graph;
}

ResolveResult AbcFile::ResolveInstCommon(const Function *func, Inst inst) const
{
    auto type = inst.GetType();
    switch (type) {
        case InstType::DEFINEFUNC_IMM8_ID16_IMM8:
        case InstType::DEFINEFUNC_IMM16_ID16_IMM8: {
            std::string func_name = GetStringByInst(inst);
            const Function *func = GetFunctionByName(func_name);
            ASSERT(func != nullptr);
            return std::make_tuple(func, EMPTY_STR, ResolveType::FUNCTION_OBJECT);
        }
        case InstType::DEFINECLASSWITHBUFFER_IMM8_ID16_ID16_IMM16_V8:
        case InstType::DEFINECLASSWITHBUFFER_IMM16_ID16_ID16_IMM16_V8: {
            std::string class_name = GetStringByInst(inst);
            const Class *clazz = GetClassByName(class_name);
            ASSERT(clazz != nullptr);
            return std::make_tuple(clazz, EMPTY_STR, ResolveType::CLASS_OBJECT);
        }
        case InstType::NEWOBJAPPLY_IMM8_V8:
        case InstType::NEWOBJAPPLY_IMM16_V8:
        case InstType::NEWOBJRANGE_IMM8_IMM8_V8:
        case InstType::NEWOBJRANGE_IMM16_IMM8_V8:
        case InstType::WIDE_NEWOBJRANGE_PREF_IMM16_V8: {
            Inst newobj_input0 = inst.GetInputInsts()[0];
            auto resolve_res = ResolveInstCommon(func, newobj_input0);
            return HandleNewObjInstResolveResultCommon(resolve_res);
        }
        case InstType::LDOBJBYNAME_IMM8_ID16:
        case InstType::LDOBJBYNAME_IMM16_ID16: {
            Inst ld_obj_input0 = inst.GetInputInsts()[0];
            auto resolve_res = ResolveInstCommon(func, ld_obj_input0);
            return HandleLdObjByNameInstResolveResult(inst, resolve_res);
        }
        case InstType::LDLEXVAR_IMM4_IMM4:
        case InstType::LDLEXVAR_IMM8_IMM8:
        case InstType::WIDE_LDLEXVAR_PREF_IMM16_IMM16: {
            auto p = GetStLexInstByLdLexInst({func, inst});
            if (p == std::nullopt) {
                return std::make_tuple(nullptr, EMPTY_STR, ResolveType::UNRESOLVED_OTHER);
            }
            return ResolveInstCommon(p.value().first, p.value().second);
        }
        case InstType::STLEXVAR_IMM4_IMM4:
        case InstType::STLEXVAR_IMM8_IMM8:
        case InstType::WIDE_STLEXVAR_PREF_IMM16_IMM16: {
            Inst stlex_input0 = inst.GetInputInsts()[0];
            return ResolveInstCommon(func, stlex_input0);
        }
        case InstType::LDLOCALMODULEVAR_IMM8:
        case InstType::WIDE_LDLOCALMODULEVAR_PREF_IMM16: {
            size_t index = inst.GetImms()[0];
            const std::string &export_name = module_record_->GetExportNameByIndex(index);
            const Function *func = GetExportFunctionByExportName(export_name);
            if (func != nullptr) {
                return std::make_tuple(func, EMPTY_STR, ResolveType::FUNCTION_OBJECT);
            }
            const Class *clazz = GetExportClassByExportName(export_name);
            if (clazz != nullptr) {
                return std::make_tuple(clazz, EMPTY_STR, ResolveType::CLASS_OBJECT);
            }
            return std::make_tuple(nullptr, EMPTY_STR, ResolveType::UNRESOLVED_OTHER);
        }
        case InstType::LDEXTERNALMODULEVAR_IMM8:
        case InstType::WIDE_LDEXTERNALMODULEVAR_PREF_IMM16: {
            size_t index = inst.GetImms()[0];
            const std::string &inter_name = module_record_->GetImportInternalNameByIndex(index);
            return std::make_tuple(nullptr, inter_name, ResolveType::UNRESOLVED_MODULE);
        }
        case InstType::GETMODULENAMESPACE_IMM8:
        case InstType::WIDE_GETMODULENAMESPACE_PREF_IMM16: {
            size_t index = inst.GetImms()[0];
            const std::string &str = module_record_->GetImportNamespaceNameByIndex(index);
            return std::make_tuple(nullptr, str, ResolveType::UNRESOLVED_MODULE);
        }
        case InstType::LDGLOBAL: {
            // TODO(wangyantian): load a specific global variable, namely 'globalThis'
            return std::make_tuple(nullptr, EMPTY_STR, ResolveType::UNRESOLVED_OTHER);
        }
        case InstType::LDGLOBALVAR_IMM16_ID16:
        case InstType::TRYLDGLOBALBYNAME_IMM8_ID16:
        case InstType::TRYLDGLOBALBYNAME_IMM16_ID16: {
            std::string str = GetStringByInst(inst);
            auto p = GetStGlobalInstByLdGlobalInst({func, inst});
            if (p == std::nullopt) {
                return std::make_tuple(nullptr, str, ResolveType::UNRESOLVED_GLOBAL_VAR);
            }
            auto [ret_ptr, ret_sym, ret_type] = ResolveInstCommon(p.value().first, p.value().second);
            if (ret_ptr != nullptr) {
                return std::make_tuple(ret_ptr, str, ret_type);
            }
            return std::make_tuple(nullptr, str, ResolveType::UNRESOLVED_GLOBAL_VAR);
        }
        case InstType::TRYSTGLOBALBYNAME_IMM8_ID16:
        case InstType::TRYSTGLOBALBYNAME_IMM16_ID16:
        case InstType::STGLOBALVAR_IMM16_ID16:
        case InstType::STCONSTTOGLOBALRECORD_IMM16_ID16:
        case InstType::STTOGLOBALRECORD_IMM16_ID16: {
            Inst stglobal_input0 = inst.GetInputInsts()[0];
            return ResolveInstCommon(func, stglobal_input0);
        }
        case InstType::OPCODE_PHI: {
            // TODO(wangyantian): only the first path is considered for now, what about other paths?
            Inst phi_input0 = inst.GetInputInsts()[0];
            return ResolveInstCommon(func, phi_input0);
        }
        // don't deal with the situation that func obj comes from parameter or the output of another call inst
        default: {
            return std::make_tuple(nullptr, EMPTY_STR, ResolveType::UNRESOLVED_OTHER);
        }
    }
}

ResolveResult AbcFile::HandleLdObjByNameInstResolveResult(const Inst &ldobjbyname_inst,
                                                          const ResolveResult &resolve_res) const
{
    auto &[ret_ptr, ret_sym, ret_type] = resolve_res;
    std::string name = GetStringByInst(ldobjbyname_inst);
    switch (ret_type) {
        case ResolveType::UNRESOLVED_MODULE:
        case ResolveType::UNRESOLVED_GLOBAL_VAR: {
            return std::make_tuple(nullptr, ret_sym + "." + name, ret_type);
        }
        case ResolveType::FUNCTION_OBJECT: {
            ASSERT(ret_ptr != nullptr);
            if (name == CALL || name == APPLY) {
                return std::make_tuple(ret_ptr, EMPTY_STR, ResolveType::FUNCTION_OBJECT);
            }
            return std::make_tuple(nullptr, ret_sym + "." + name, ResolveType::UNRESOLVED_OTHER);
        }
        case ResolveType::CLASS_OBJECT:
        case ResolveType::CLASS_INSTANCE: {
            ASSERT(ret_ptr != nullptr);
            // TODO(wangyantian): distinguish static func from member func in a class
            const void *member_func = reinterpret_cast<const Class *>(ret_ptr)->GetMemberFunctionByName(name);
            if (member_func != nullptr) {
                return std::make_tuple(member_func, name, ResolveType::FUNCTION_OBJECT);
            }
            return std::make_tuple(nullptr, ret_sym + "." + name, ResolveType::UNRESOLVED_OTHER);
        }
        default: {
            return std::make_tuple(nullptr, ret_sym + "." + name, ResolveType::UNRESOLVED_OTHER);
        }
    }
}

ResolveResult AbcFile::HandleNewObjInstResolveResultCommon(const ResolveResult &resolve_res) const
{
    auto &[ret_ptr, ret_sym, ret_type] = resolve_res;
    switch (ret_type) {
        case ResolveType::CLASS_OBJECT: {
            ASSERT(ret_ptr != nullptr);
            return std::make_tuple(ret_ptr, EMPTY_STR, ResolveType::CLASS_INSTANCE);
        }
        case ResolveType::UNRESOLVED_GLOBAL_VAR:
        case ResolveType::UNRESOLVED_MODULE: {
            return std::make_tuple(nullptr, ret_sym, ret_type);
        }
        default: {
            return std::make_tuple(nullptr, ret_sym, ResolveType::UNRESOLVED_OTHER);
        }
    }
}

Function *AbcFile::ResolveDefineFuncInstCommon(const Function *func, const Inst &def_func_inst) const
{
    std::string def_func_name = GetStringByInst(def_func_inst);
    Function *def_func = GetFunctionByNameImpl(def_func_name);
    ASSERT(def_func != nullptr);
    return def_func;
}

std::unique_ptr<Class> AbcFile::ResolveDefineClassWithBufferInst(Function *func, const Inst &define_class_inst) const
{
    auto imms = define_class_inst.GetImms();
    auto m_id = EntityId(imms[1]);
    std::string class_name = GetStringByMethodId(m_id);
    std::unique_ptr<Class> def_class = std::make_unique<Class>(class_name, this, func);
    if (def_class == nullptr) {
        LOG(FATAL, DEFECT_SCAN_AUX) << "Can not allocate memory when processing '" << filename_ << "'";
    }
    LOG(DEBUG, DEFECT_SCAN_AUX) << "Create a new class: " << class_name;
    func->AddDefinedClass(def_class.get());

    // handle ctor of the class
    std::string ctor_name = GetStringByInst(define_class_inst);
    HandleMemberFunctionFromClassBuf(ctor_name, func, def_class.get());

    auto literal_array_id = EntityId(imms[2]);
    panda_file::LiteralDataAccessor lit_array_accessor(*panda_file_, panda_file_->GetLiteralArraysId());
    lit_array_accessor.EnumerateLiteralVals(
        literal_array_id, [&](const panda_file::LiteralDataAccessor::LiteralValue &value, const LiteralTag &tag) {
            if (tag == LiteralTag::METHOD || tag == LiteralTag::GENERATORMETHOD ||
                tag == LiteralTag::ASYNCGENERATORMETHOD) {
                auto method_id = EntityId(std::get<uint32_t>(value));
                std::string member_func_name = GetStringByMethodId(method_id);
                HandleMemberFunctionFromClassBuf(member_func_name, func, def_class.get());
            }
        });

    return def_class;
}

std::unique_ptr<CalleeInfo> AbcFile::ResolveCallInstCommon(Function *func, const Inst &call_inst,
                                                           uint32_t func_obj_idx) const
{
    std::unique_ptr<CalleeInfo> callee_info = std::make_unique<CalleeInfo>(call_inst, func);
    if (callee_info == nullptr) {
        LOG(FATAL, DEFECT_SCAN_AUX) << "Can not allocate memory when processing '" << filename_ << "'";
    }

    Inst call_input0 = call_inst.GetInputInsts()[func_obj_idx];
    auto [ret_ptr, ret_sym, ret_type] = ResolveInstCommon(func, call_input0);
    if (ret_ptr != nullptr && ret_type == ResolveType::FUNCTION_OBJECT) {
        auto callee = reinterpret_cast<const Function *>(ret_ptr);
        callee_info->SetCallee(callee);
    } else {
        size_t first_delim_idx = ret_sym.find_first_of(DELIM);
        size_t last_delim_idx = ret_sym.find_last_of(DELIM);
        std::string callee_name = ret_sym;
        std::string var_name = EMPTY_STR;
        if (first_delim_idx != std::string::npos) {
            callee_name = ret_sym.substr(last_delim_idx + 1);
            var_name = ret_sym.substr(0, first_delim_idx);
            callee_info->SetFunctionName(callee_name);
        }
        if (ret_type == ResolveType::UNRESOLVED_MODULE) {
            std::string imp_callee_name = GetImportNameByInternalName(callee_name);
            if (!imp_callee_name.empty()) {
                callee_info->SetFunctionName(imp_callee_name);
            }
            std::string inter_name = var_name.empty() ? callee_name : var_name;
            std::string module_name = GetModuleNameByInternalName(inter_name);
            if (!module_name.empty()) {
                callee_info->SetExternalModuleName(module_name);
            }
        } else if (ret_type == ResolveType::UNRESOLVED_GLOBAL_VAR) {
            callee_info->SetFunctionName(callee_name);
            var_name = var_name.empty() ? var_name : ret_sym.substr(0, last_delim_idx);
            callee_info->SetGlobalVarName(var_name);
        }
    }
    func->AddCalleeInfo(callee_info.get());
    return callee_info;
}

std::unique_ptr<CalleeInfo> AbcFile::ResolveSuperCallInst(Function *func, const Inst &call_inst) const
{
    std::unique_ptr<CalleeInfo> callee_info = std::make_unique<CalleeInfo>(call_inst, func);
    if (callee_info == nullptr) {
        LOG(FATAL, DEFECT_SCAN_AUX) << "Can not allocate memory when processing '" << filename_ << "'";
    }
    const Class *clazz = func->GetClass();
    if (clazz != nullptr && clazz->GetParentClass() != nullptr) {
        const std::string &parent_ctor_name = clazz->GetParentClass()->GetClassName();
        const Function *parent_ctor = GetFunctionByName(parent_ctor_name);
        ASSERT(parent_ctor != nullptr);
        callee_info->SetCallee(parent_ctor);
    }
    // TODO(wangyantian): deal with situations when above if doesn't hold
    func->AddCalleeInfo(callee_info.get());
    return callee_info;
}

void AbcFile::HandleMemberFunctionFromClassBuf(const std::string &func_name, Function *def_func, Class *def_class) const
{
    Function *member_func = GetFunctionByNameImpl(func_name);
    ASSERT(member_func != nullptr);
    BuildFunctionDefineChain(def_func, member_func);
    BuildClassAndMemberFuncRelation(def_class, member_func);
}

void AbcFile::AddDefinedClass(std::unique_ptr<Class> &&def_class)
{
    auto &class_name = def_class->GetClassName();
    ASSERT(def_class_map_.find(class_name) == def_class_map_.end());
    def_class_map_[class_name] = def_class.get();
    def_class_list_.emplace_back(std::move(def_class));
}

void AbcFile::AddDefinedFunction(std::unique_ptr<Function> &&def_func)
{
    const std::string &func_name = def_func->GetFunctionName();
    ASSERT(def_func_map_.find(func_name) == def_func_map_.end());
    def_func_map_[func_name] = def_func.get();
    if (func_name != ENTRY_FUNCTION_NAME) {
        def_func_list_.emplace_back(std::move(def_func));
    } else {
        // make def_func_list_[0] the 'func_main_0'
        def_func_list_.insert(def_func_list_.begin(), std::move(def_func));
    }
}

void AbcFile::AddCalleeInfo(std::unique_ptr<CalleeInfo> &&callee_info)
{
    callee_info_list_.emplace_back(std::move(callee_info));
}

Function *AbcFile::GetFunctionByNameImpl(std::string_view func_name) const
{
    auto iter = def_func_map_.find(std::string(func_name));
    if (iter != def_func_map_.end()) {
        return iter->second;
    }
    return nullptr;
}

Class *AbcFile::GetClassByNameImpl(std::string_view class_name) const
{
    auto iter = def_class_map_.find(std::string(class_name));
    if (iter != def_class_map_.end()) {
        return iter->second;
    }
    return nullptr;
}

std::string AbcFile::GetStringByMethodId(EntityId method_id) const
{
    panda_file::MethodDataAccessor mda {*panda_file_, method_id};
    return GetStringByStringId(mda.GetNameId());
}

std::string AbcFile::GetStringByStringId(EntityId string_id) const
{
    StringData sd = panda_file_->GetStringData(string_id);
    // TODO(wangyantian): what if sd.is_ascii equals false?
    return std::string(utf::Mutf8AsCString(sd.data));
}
}  // namespace panda::defect_scan_aux
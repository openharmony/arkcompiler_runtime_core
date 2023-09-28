static void VisitCallLaunchStatic([[maybe_unused]] panda::compiler::GraphVisitor *v,
                                  [[maybe_unused]] panda::compiler::Inst *inst)
{
    auto *re = static_cast<RegEncoder *>(v);
    re->Check4Width(inst);
}

static void VisitCallLaunchVirtual([[maybe_unused]] panda::compiler::GraphVisitor *v,
                                   [[maybe_unused]] panda::compiler::Inst *inst)
{
    auto *re = static_cast<RegEncoder *>(v);
    re->Check4Width(inst);
}
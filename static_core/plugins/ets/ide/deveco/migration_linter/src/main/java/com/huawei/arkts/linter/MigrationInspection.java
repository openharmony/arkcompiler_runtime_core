package com.huawei.arkts.linter;

import com.intellij.codeInspection.LocalInspectionTool;
import com.intellij.codeInspection.ex.ExternalAnnotatorBatchInspection;
import org.jetbrains.annotations.NotNull;

public class MigrationInspection extends LocalInspectionTool implements ExternalAnnotatorBatchInspection {
    public static final String SHORT_NAME = "ArkTSLinter";

    @Override
    public @NotNull String getShortName() {
        return SHORT_NAME;
    }
}

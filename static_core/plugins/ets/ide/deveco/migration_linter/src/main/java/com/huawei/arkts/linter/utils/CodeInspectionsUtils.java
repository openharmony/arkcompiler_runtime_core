package com.huawei.arkts.linter.utils;

import com.intellij.codeInsight.daemon.HighlightDisplayKey;
import com.intellij.codeInspection.ex.InspectionProfileImpl;
import com.intellij.openapi.project.Project;
import com.intellij.profile.codeInspection.InspectionProfileManager;
import org.jetbrains.annotations.NotNull;

public class CodeInspectionsUtils {
    protected CodeInspectionsUtils() {
    }

    public static boolean inspectionEnabled(String inspectionShortName,
                                            @NotNull Project project) {
        InspectionProfileImpl profile = InspectionProfileManager.getInstance(project).getCurrentProfile();
        return profile.isToolEnabled(HighlightDisplayKey.find(inspectionShortName));
    }
}

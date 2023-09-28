package com.huawei.arkts.linter;

import com.huawei.arkts.linter.highlighter.Issue;
import com.huawei.arkts.linter.highlighter.LinterAnnotator;
import com.intellij.codeInsight.daemon.HighlightDisplayKey;
import com.intellij.codeInspection.InspectionProfile;
import com.intellij.lang.annotation.AnnotationHolder;
import com.intellij.lang.annotation.ExternalAnnotator;
import com.intellij.lang.annotation.HighlightSeverity;
import com.intellij.openapi.diagnostic.Logger;
import com.intellij.openapi.editor.Editor;
import com.intellij.openapi.progress.ProcessCanceledException;
import com.intellij.profile.codeInspection.InspectionProfileManager;
import com.intellij.psi.PsiFile;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

import java.io.InterruptedIOException;
import java.util.Collections;
import java.util.List;

public class MigrationAnnotator
        extends ExternalAnnotator<MigrationAnnotator.InitialInfo, MigrationAnnotator.ToolResult> {
    private static final Logger LOG = Logger.getInstance(MigrationAnnotator.class);
    private static final ToolResult NO_ISSUES_FOUND = new ToolResult(null, Collections.emptyList());

    @Override
    public String getPairedBatchInspectionShortName() {
        return MigrationInspection.SHORT_NAME;
    }

    @NotNull
    @Override
    public InitialInfo collectInformation(@NotNull PsiFile file, @NotNull Editor editor, boolean hasErrors) {
        return collectInformation(file);
    }

    @NotNull
    @Override
    public InitialInfo collectInformation(@NotNull PsiFile file) {
        LOG.info("Collect information for " + file.getName());
        return new InitialInfo(file);
    }

    @NotNull
    @Override
    public ToolResult doAnnotate(@NotNull InitialInfo collectedInfo) {
        PsiFile file = collectedInfo.file;
        LOG.info("Triggered annotation for " + file.getName());
        try {
            return new ToolResult(file, LinterAnnotator.runLinter(file));
        } catch (final ProcessCanceledException | InterruptedException | InterruptedIOException exc) {
            LOG.debug("Process canceled during scanning file " + file.getName(), exc);
            return NO_ISSUES_FOUND;
        } catch (final Exception exc) {
            LOG.error("Unhandled exception: " + exc.getMessage(), exc);
            return NO_ISSUES_FOUND;
        }
    }

    @Override
    public void apply(@NotNull PsiFile file, ToolResult annotationResult, @NotNull AnnotationHolder holder) {
        LOG.info("Applying highlight in " + file.getName());

        if (annotationResult == null || annotationResult.file != file) {
            return;
        }

        final InspectionProfile profile =
                InspectionProfileManager.getInstance(file.getProject()).getCurrentProfile();
        final HighlightDisplayKey key = HighlightDisplayKey.find(MigrationInspection.SHORT_NAME);
        final HighlightSeverity severity = profile.getErrorLevel(key, file).getSeverity();
        for (Issue issue : annotationResult.issues) {
            issue.doAnnotate(holder, severity);
        }

        LOG.info("Highlighted " + annotationResult.issues.size() + " issues in " + file.getName());
    }

    static class InitialInfo {
        private final @NotNull PsiFile file;

        public InitialInfo(@NotNull PsiFile file) {
            this.file = file;
        }
    }

    static class ToolResult {
        private final @Nullable PsiFile file;
        private final @NotNull List<Issue> issues;

        public ToolResult(@Nullable PsiFile file, @NotNull List<Issue> issues) {
            this.file = file;
            this.issues = issues;
        }
    }
}

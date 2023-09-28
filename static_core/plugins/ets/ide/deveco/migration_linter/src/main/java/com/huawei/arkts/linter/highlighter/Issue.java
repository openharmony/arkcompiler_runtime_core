package com.huawei.arkts.linter.highlighter;

import com.huawei.arkts.linter.checker.LinterJSONReport;
import com.huawei.arkts.linter.utils.SystemFileUtils;
import com.intellij.diagnostic.PluginException;
import com.intellij.lang.annotation.AnnotationBuilder;
import com.intellij.lang.annotation.AnnotationHolder;
import com.intellij.lang.annotation.HighlightSeverity;
import com.intellij.openapi.diagnostic.Logger;
import com.intellij.openapi.util.TextRange;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

public class Issue {
    private static final Logger LOG = Logger.getInstance(Issue.class);

    private final int tokenStart;
    private final int tokenEnd;
    private final @NotNull String annotationMessage;
    private final @Nullable String tooltip;

    public Issue(int tokenStart,
                 int tokenEnd,
                 @NotNull String issueType,
                 @Nullable String rule,
                 @Nullable String tooltip) {
        this.tokenStart = tokenStart;
        this.tokenEnd = tokenEnd;
        this.annotationMessage = formatMessage(issueType, rule);
        this.tooltip = tooltip;
    }

    public static @NotNull Issue createIssue(final @NotNull LinterJSONReport.IssueInfo info) {
        return new Issue(info.getTokenStart(), info.getTokenEnd(), info.getType(), info.getRule(), info.getMessage());
    }

    public void doAnnotate(@NotNull AnnotationHolder holder, @NotNull HighlightSeverity severity) {
        try {
            AnnotationBuilder builder = holder
                    .newAnnotation(severity, annotationMessage)
                    .range(new TextRange(tokenStart, tokenEnd));
            if (tooltip != null) {
                builder = builder.tooltip(tooltip);
            }
            builder.create();
        } catch (final PluginException exc) {
            String fileName = holder.getCurrentAnnotationSession().getFile().getName();
            LOG.warn("Failed annotation in " + fileName + ": " + exc.getMessage(), exc);
        }
    }

    private static @NotNull String formatMessage(@NotNull String issueType, @Nullable String rule) {
        String description = SystemFileUtils.isNullOrEmpty(rule) ? issueType : rule;
        return "ArkTS Linter: " + description;
    }
}

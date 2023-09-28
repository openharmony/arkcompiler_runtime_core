package com.huawei.arkts.linter.highlighter;

import com.huawei.arkts.linter.checker.CheckerException;
import com.huawei.arkts.linter.checker.LinterJSONReport;
import com.huawei.arkts.linter.checker.ToolRunner;
import com.intellij.openapi.diagnostic.Logger;
import com.intellij.psi.PsiFile;
import org.jetbrains.annotations.NotNull;

import java.io.InterruptedIOException;
import java.util.*;

public class LinterAnnotator {
    public static final Logger LOG = Logger.getInstance(LinterAnnotator.class);

    protected LinterAnnotator() {
    }

    public static @NotNull List<Issue> runLinter(@NotNull PsiFile analyzedFile)
            throws InterruptedException, InterruptedIOException{
        try {
            return runImplementation(analyzedFile);
        } catch (final CheckerException exc) {
            LOG.warn("Checker failed with exception", exc);
        }
        return Collections.emptyList();
    }

    private static List<Issue> runImplementation(@NotNull PsiFile analyzedFile) throws CheckerException {
        List<LinterJSONReport.IssueInfo> issuesInfo = ToolRunner.run(analyzedFile);
        List<Issue> issues = new ArrayList<>();
        if (issuesInfo != null) {
            issuesInfo.forEach(info -> issues.add(Issue.createIssue(info)));
        }
        return issues;
    }
}

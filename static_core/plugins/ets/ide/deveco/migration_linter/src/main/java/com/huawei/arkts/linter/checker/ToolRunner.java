package com.huawei.arkts.linter.checker;

import com.huawei.arkts.linter.utils.SystemFileUtils;
import com.intellij.openapi.application.PathManager;
import com.intellij.openapi.diagnostic.Logger;
import com.intellij.openapi.editor.Document;
import com.intellij.psi.PsiDocumentManager;
import com.intellij.psi.PsiFile;
import com.squareup.moshi.JsonAdapter;
import com.squareup.moshi.Moshi;
import okio.Okio;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

import java.io.*;
import java.nio.file.Path;
import java.util.*;

public class ToolRunner {
    private static final Logger LOG = Logger.getInstance(ToolRunner.class);
    private static final String PLUGIN_BUILD_NAME = "migration-plugin";
    private static final Path PATH_TO_TOOL = Path.of(
            PathManager.getPluginsPath(),
            PLUGIN_BUILD_NAME,
            "tool",
            "package",
            "bin",
            "tslinter.js");

    private final @NotNull PsiFile analyzedFile;
    private @NotNull LinterJSONReport report = new LinterJSONReport();

    public ToolRunner(@NotNull PsiFile analyzedFile) {
        this.analyzedFile = analyzedFile;
    }

    public @Nullable List<LinterJSONReport.IssueInfo> getIssues() {
        return report.getIssues();
    }

    public static @Nullable List<LinterJSONReport.IssueInfo> run(@NotNull PsiFile analyzedFile) throws CheckerException {
        ToolRunner runner = new ToolRunner(analyzedFile);
        runner.run();
        return runner.getIssues();
    }

    public void run() throws CheckerException {
        ProcessBuilder commandLine = getToolCommandLine();
        final Process process;
        try {
            LOG.info("Running process: " + commandLine.command());
            process = commandLine.start();
            writeFileTextToProcess(process.getOutputStream());
            parseJSONReport(new BufferedInputStream(process.getInputStream()));
            process.waitFor();
        } catch (final InterruptedException | IOException exc) {
            LOG.warn("Process failed, command line was: " + commandLine.command(), exc);
            throw new CheckerException(exc);
        }
    }

    @NotNull
    private ProcessBuilder getToolCommandLine() throws CheckerException {
        String nodePath = SystemFileUtils.getNodePath();
        if (nodePath == null) {
            throw new CheckerException("node was not found");
        }

        List<String> arguments = List.of(nodePath, PATH_TO_TOOL.toString(), "--deveco-plugin-mode");
        ProcessBuilder builder = new ProcessBuilder(arguments);
        return builder.redirectError(new File(SystemFileUtils.getNullDestinationName()));
    }

    private void writeFileTextToProcess(@NotNull OutputStream stream) throws IOException {
        BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(stream));
        Document document = PsiDocumentManager.getInstance(analyzedFile.getProject()).getDocument(analyzedFile);
        writer.write(Objects.requireNonNull(document).getText());
        writer.flush();
        writer.close();
    }

    private void parseJSONReport(final @NotNull BufferedInputStream inputStream) throws CheckerException {
        try {
            Moshi moshi = new Moshi.Builder().build();
            JsonAdapter<LinterJSONReport> adapter = moshi.adapter(LinterJSONReport.class);
            if (checkIfInputStreamIsEmpty(inputStream)) {
                report = new LinterJSONReport();
            } else {
                report = Objects.requireNonNull(adapter.fromJson(Okio.buffer(Okio.source(inputStream))));
            }
        } catch (final IOException exc) {
            throw new CheckerException(exc);
        }
    }

    private static boolean checkIfInputStreamIsEmpty(final @NotNull BufferedInputStream inputStream)
            throws IOException {
        inputStream.mark(1);
        int data = inputStream.read();
        inputStream.reset();
        return data == -1;
    }
}

package com.huawei.arkts.linter.utils;

import com.huawei.arkts.linter.LinterBundle;
import com.intellij.execution.ExecutionException;
import com.intellij.execution.configurations.GeneralCommandLine;
import com.intellij.ide.util.PropertiesComponent;
import com.intellij.openapi.diagnostic.Logger;
import com.intellij.openapi.util.SystemInfo;
import com.intellij.openapi.util.io.FileUtil;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Optional;
import java.util.stream.Collectors;

public class SystemFileUtils {
    private static final Logger LOG = Logger.getInstance(SystemFileUtils.class);
    private static final boolean IS_WINDOWS = SystemInfo.isWindows;

    private SystemFileUtils() {}

    @Nullable
    public static String getNodePath() {
        String nodePath = PropertiesComponent.getInstance()
                .getValue(LinterBundle.message("nodejs.default.path"), "");
        if (!isNullOrEmpty(nodePath)) {
            return getNodeExecutablePath(nodePath);
        }
        return getSystemExecutablePath(getNodeExecutableName());
    }

    @Nullable
    public static String getSystemExecutablePath(String executableName) {
        GeneralCommandLine cmd = new GeneralCommandLine(getWhichExecutableName());
        cmd.addParameter(executableName);
        final Process process;
        try {
            process = cmd.createProcess();
            Optional<String> path = new BufferedReader(
                    new InputStreamReader(cmd.createProcess().getInputStream(), StandardCharsets.UTF_8))
                    .lines()
                    .findFirst();
            process.waitFor();
            String error = new BufferedReader(
                    new InputStreamReader(process.getErrorStream(), StandardCharsets.UTF_8))
                    .lines()
                    .collect(Collectors.joining("\n"));

            if (!error.isEmpty()) {
                LOG.info("Command Line string: " + cmd.getCommandLineString());
                LOG.info("Messages while checking " + executableName + " path: " + error);
            }
            if (process.exitValue() != 0 || path.isEmpty()) {
                LOG.info("Command Line string: " + cmd.getCommandLineString());
                LOG.warn("Path detect process.exitValue: " + process.exitValue());
                return null;
            }
            LOG.info("Detected path: " + path.get());
            return path.get();
        } catch (ExecutionException | InterruptedException e) {
            return null;
        }
    }

    public static String getNullDestinationName() {
        return IS_WINDOWS ? "NUL" : "/dev/null";
    }

    public static boolean isNullOrEmpty(String string) {
        return string == null || string.isEmpty();
    }

    @Nullable
    private static String getNodeExecutablePath(@NotNull String nodeDirPath) {
        nodeDirPath = FileUtil.toSystemDependentName(nodeDirPath);
        String nodeName = getNodeExecutableName();
        Path nodeJsPath = Path.of(nodeDirPath, nodeName);
        if (Files.exists(nodeJsPath)) {
            return nodeJsPath.toString();
        }
        // on Linux and macOS must look up under bin
        nodeJsPath = Path.of(nodeDirPath, "bin", nodeName);
        return Files.exists(nodeJsPath) ? nodeJsPath.toString() : null;
    }

    private static String getNodeExecutableName() {
        String name = "node";
        if (IS_WINDOWS) {
            return name + ".exe";
        }
        return name;
    }

    private static String getWhichExecutableName() {
        return IS_WINDOWS ? "where" : "which";
    }
}

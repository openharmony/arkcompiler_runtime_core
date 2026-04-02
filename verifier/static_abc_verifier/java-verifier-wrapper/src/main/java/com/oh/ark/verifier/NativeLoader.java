/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

package com.oh.ark.verifier;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

/**
 * Utility class for loading native libraries bundled inside the JAR.
 *
 * <p>The native .so files are expected under:
 * <pre>
 *   native/linux-x86_64/libstaticabcverifier_jni.so
 *   native/linux-aarch64/libstaticabcverifier_jni.so
 * </pre>
 *
 * <p>At runtime the loader detects the OS and architecture, extracts the
 * matching .so to a temporary directory, and calls {@link System#load}.
 */
final class NativeLoader {

    private static volatile boolean loaded = false;

    private NativeLoader() {
    }

    static synchronized void load() {
        if (loaded) {
            return;
        }

        // Try java.library.path first (for development/manual deployment)
        try {
            System.loadLibrary("staticabcverifier_jni");
            loaded = true;
            return;
        } catch (UnsatisfiedLinkError ignored) {
            // Fall through to extract from JAR
        }

        String resourcePath = getNativeResourcePath();
        try {
            File tempFile = extractResource(resourcePath);
            System.load(tempFile.getAbsolutePath());
            loaded = true;
        } catch (IOException e) {
            throw new UnsatisfiedLinkError(
                    "Failed to load native library from JAR resource '"
                            + resourcePath + "': " + e.getMessage());
        }
    }

    private static String getNativeResourcePath() {
        String os = System.getProperty("os.name", "").toLowerCase();
        String arch = System.getProperty("os.arch", "").toLowerCase();

        if (!os.contains("linux")) {
            throw new UnsatisfiedLinkError(
                    "Unsupported OS: " + os + ". Only Linux is supported.");
        }

        String archDir;
        if (arch.equals("amd64") || arch.equals("x86_64")) {
            archDir = "linux-x86_64";
        } else if (arch.equals("aarch64") || arch.equals("arm64")) {
            archDir = "linux-aarch64";
        } else {
            throw new UnsatisfiedLinkError(
                    "Unsupported architecture: " + arch
                            + ". Only x86_64 and aarch64 are supported.");
        }

        return "/native/" + archDir + "/libstaticabcverifier_jni.so";
    }

    private static File extractResource(String resourcePath) throws IOException {
        InputStream in = NativeLoader.class.getResourceAsStream(resourcePath);
        if (in == null) {
            throw new IOException("Native library resource not found: " + resourcePath);
        }

        File tempDir = new File(System.getProperty("java.io.tmpdir"),
                "static-abc-verifier-native");
        if (!tempDir.exists() && !tempDir.mkdirs()) {
            throw new IOException("Failed to create temp directory: " + tempDir);
        }

        File tempFile = new File(tempDir, "libstaticabcverifier_jni.so");
        try (FileOutputStream out = new FileOutputStream(tempFile)) {
            byte[] buf = new byte[8192];
            int bytesRead;
            while ((bytesRead = in.read(buf)) != -1) {
                out.write(buf, 0, bytesRead);
            }
        } finally {
            in.close();
        }

        if (!tempFile.setExecutable(true)) {
            throw new IOException("Failed to make library executable: " + tempFile);
        }
        tempFile.deleteOnExit();
        return tempFile;
    }
}

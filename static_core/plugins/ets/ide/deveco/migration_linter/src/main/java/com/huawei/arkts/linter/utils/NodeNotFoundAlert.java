package com.huawei.arkts.linter.utils;

import com.huawei.arkts.linter.MigrationInspection;
import com.intellij.openapi.fileEditor.FileEditor;
import com.intellij.openapi.project.Project;
import com.intellij.openapi.util.Key;
import com.intellij.openapi.vfs.VirtualFile;
import com.intellij.ui.EditorNotificationPanel;
import com.intellij.ui.EditorNotifications;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

// inherit from the deprecated Provider to support feature in DevEco
public class NodeNotFoundAlert extends EditorNotifications.Provider<EditorNotificationPanel> {
    private static final Key<EditorNotificationPanel> KEY = Key.create("com.huawei.migration.plugin.alert");
    private static final String ALERT_MESSAGE = "Node must be installed globally for correct linter work.";
    private static final String IGNORE_ACTION_NAME = "Ignore";
    private static boolean ignoreGlobally = false;

    @Override
    @NotNull
    public Key<EditorNotificationPanel> getKey() {
        return KEY;
    }

    @Override
    @Nullable
    public EditorNotificationPanel createNotificationPanel(@NotNull VirtualFile file,
                                                           @NotNull FileEditor fileEditor,
                                                           @NotNull Project project) {
        if (ignoreGlobally
                || !isFileTSOrJS(file)
                || !CodeInspectionsUtils.inspectionEnabled(MigrationInspection.SHORT_NAME, project)
                || SystemFileUtils.getNodePath() != null) {
            return null;
        }
        EditorNotificationPanel panel = new EditorNotificationPanel(fileEditor);
        panel.createActionLabel(IGNORE_ACTION_NAME, () -> {
            NodeNotFoundAlert.ignoreGlobally = true;
            EditorNotifications.getInstance(project).updateAllNotifications();
        });
        return panel.text(ALERT_MESSAGE);
    }

    private static boolean isFileTSOrJS(@NotNull VirtualFile file) {
        String extension = file.getExtension();
        return (extension != null) && (extension.equals("ts") || extension.equals("js"));
    }
}

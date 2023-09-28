package com.huawei.arkts.linter;

import com.intellij.AbstractBundle;
import org.jetbrains.annotations.NonNls;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.PropertyKey;

import java.lang.ref.Reference;
import java.lang.ref.SoftReference;
import java.util.ResourceBundle;

public final class LinterBundle {
    @NonNls
    private static final String BUNDLE = "com.huawei.arkts.linter.LinterBundle";
    private static Reference<ResourceBundle> bundleReference;

    private LinterBundle() {
    }

    @NotNull
    public static String message(@PropertyKey(resourceBundle = BUNDLE) final String key,
                                 final Object... params) {
        return AbstractBundle.message(getBundle(), key, params);
    }

    private static ResourceBundle getBundle() {
        ResourceBundle bundle = null;

        if (bundleReference != null) {
            bundle = bundleReference.get();
        }
        if (bundle == null) {
            bundle = ResourceBundle.getBundle(BUNDLE);
            bundleReference = new SoftReference<>(bundle);
        }

        return bundle;
    }
}

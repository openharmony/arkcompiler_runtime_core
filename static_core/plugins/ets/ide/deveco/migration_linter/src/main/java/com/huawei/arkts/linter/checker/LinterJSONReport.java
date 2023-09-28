package com.huawei.arkts.linter.checker;

import org.jetbrains.annotations.NotNull;
import com.squareup.moshi.Json;
import org.jetbrains.annotations.Nullable;

import java.util.List;

public class LinterJSONReport {
    @Json(name = "linter messages")
    private List<IssueInfo> issues;

    public List<IssueInfo> getIssues() {
        return issues;
    }

    public void setIssues(List<IssueInfo> issues) {
        this.issues = issues;
    }

    public static class IssueInfo {
        @Json(name = "line")
        private int line;
        @Json(name = "column")
        private int column;
        @Json(name = "start")
        private int tokenStart;
        @Json(name = "end")
        private int tokenEnd;
        @Json(name = "type")
        private @NotNull String type;
        @Json(name = "rule")
        private @Nullable String rule;
        @Json(name = "suggest")
        private @Nullable String message;

        public int getLine() {
            return line;
        }

        public void setLine(int line) {
            this.line = line;
        }

        public int getColumn() {
            return column;
        }

        public void setColumn(int column) {
            this.column = column;
        }

        public int getTokenStart() {
            return tokenStart;
        }

        public void setTokenStart(int tokenStart) {
            this.tokenStart = tokenStart;
        }

        public int getTokenEnd() {
            return tokenEnd;
        }

        public void setTokenEnd(int tokenEnd) {
            this.tokenEnd = tokenEnd;
        }

        public @NotNull String getType() {
            return type;
        }

        public void setType(@NotNull String type) {
            this.type = type;
        }

        public @Nullable String getRule() {
            return rule;
        }

        public void setRule(@Nullable String rule) {
            this.rule = rule;
        }

        public @Nullable String getMessage() {
            return message;
        }

        public void setMessage(@Nullable String message) {
            this.message = message;
        }

        @NotNull
        public String toString() {
            return new StringBuilder(line)
                    .append(':')
                    .append(column)
                    .append(' ')
                    .append(type)
                    .append("; ")
                    .append(message)
                    .toString();
        }
    }
}

#!/usr/bin/env python3
"""Generate a PDF report from review_linter.py output.

Usage (two modes):

  # Run linter and produce report in one step:
  python3 linter_report.py --path ../../../.. --output report.pdf

  # Use existing linter output:
  python3 linter_report.py --input violations.txt --output report.pdf
"""

import argparse
import datetime
import os
import subprocess
import sys
import tempfile
from collections import defaultdict

try:
    from reportlab.lib import colors
    from reportlab.lib.pagesizes import A4
    from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
    from reportlab.lib.units import cm
    from reportlab.platypus import (
        HRFlowable, PageBreak, Paragraph, SimpleDocTemplate, Spacer,
        Table, TableStyle,
    )
except ImportError:
    sys.exit(
        "reportlab is required. Install with:\n"
        "  pip3 install reportlab --break-system-packages"
    )


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

def parse_args():
    p = argparse.ArgumentParser(description="Generate PDF report from linter output")
    src = p.add_mutually_exclusive_group(required=True)
    src.add_argument("--input", metavar="FILE",
                     help="Pre-existing linter output file (one violation per line)")
    src.add_argument("--path", metavar="DIR",
                     help="Run review_linter.py --mode=full --path=DIR and report results")
    p.add_argument("--output", metavar="FILE", default="linter_report.pdf",
                   help="Output PDF path (default: linter_report.pdf)")
    p.add_argument("--linter", metavar="FILE",
                   default=os.path.join(os.path.dirname(__file__), "review_linter.py"),
                   help="Path to review_linter.py (default: same directory)")
    return p.parse_args()


# ---------------------------------------------------------------------------
# Data loading
# ---------------------------------------------------------------------------

def run_linter(linter_path: str, scan_path: str) -> str:
    """Run the linter and return its stdout."""
    print(f"Running linter on {scan_path} ...", file=sys.stderr)
    result = subprocess.run(
        [sys.executable, linter_path, "--mode=full", f"--path={scan_path}"],
        capture_output=True, text=True,
    )
    return result.stdout + result.stderr


def load_violations(text: str):
    """Parse linter output lines into (rule, severity, path_parts, lineno) tuples."""
    rule_severity = {}
    violations = []
    for line in text.splitlines():
        line = line.strip()
        if not line or not line.startswith("ETS"):
            continue
        parts = line.split(":", 4)
        if len(parts) < 4:
            continue
        rule, sev, filepath, lineno = parts[0], parts[1], parts[2], parts[3]
        rule_severity[rule] = sev
        violations.append((rule, sev, filepath.split("/"), lineno))
    return rule_severity, violations


# ---------------------------------------------------------------------------
# Aggregation
# ---------------------------------------------------------------------------

def aggregate(violations, key_fn):
    by_key = defaultdict(lambda: defaultdict(int))
    key_total = defaultdict(int)
    for rule, _sev, pparts, _lineno in violations:
        k = key_fn(pparts)
        by_key[k][rule] += 1
        key_total[k] += 1
    return by_key, key_total


def make_table_data(by_key, key_total, rules, label="Directory"):
    header = [label, "Total"] + rules
    rows = [header]
    for k, total in sorted(key_total.items(), key=lambda x: -x[1]):
        row = [k, str(total)] + [str(by_key[k].get(r, "")) for r in rules]
        rows.append(row)
    totals = ["TOTAL", str(sum(key_total.values()))]
    totals += [str(sum(by_key[k].get(r, 0) for k in key_total)) for r in rules]
    rows.append(totals)
    return rows


# ---------------------------------------------------------------------------
# Styles & PDF helpers
# ---------------------------------------------------------------------------

PALETTE = {
    "header_bg": colors.HexColor("#1a1a2e"),
    "header_fg": colors.white,
    "total_bg":  colors.HexColor("#e8e8e8"),
    "alt_row":   colors.HexColor("#f5f7fa"),
    "grid":      colors.HexColor("#cccccc"),
}
SEVERITY_COLOR = {
    "Error":   colors.HexColor("#c0392b"),
    "Warning": colors.HexColor("#e67e22"),
}


def make_table_style(rules, rule_severity, has_total_row=True):
    cmd = [
        ("BACKGROUND",    (0, 0),  (-1, 0),  PALETTE["header_bg"]),
        ("TEXTCOLOR",     (0, 0),  (-1, 0),  PALETTE["header_fg"]),
        ("FONTNAME",      (0, 0),  (-1, 0),  "Helvetica-Bold"),
        ("FONTSIZE",      (0, 0),  (-1, 0),  7),
        ("ALIGN",         (0, 0),  (-1, 0),  "CENTER"),
        ("FONTSIZE",      (0, 1),  (-1, -1), 7),
        ("ALIGN",         (1, 1),  (-1, -1), "CENTER"),
        ("ALIGN",         (0, 1),  (0, -1),  "LEFT"),
        ("GRID",          (0, 0),  (-1, -1), 0.3, PALETTE["grid"]),
        ("TOPPADDING",    (0, 0),  (-1, -1), 3),
        ("BOTTOMPADDING", (0, 0),  (-1, -1), 3),
        ("LEFTPADDING",   (0, 0),  (-1, -1), 4),
        ("RIGHTPADDING",  (0, 0),  (-1, -1), 4),
        ("ROWBACKGROUNDS", (0, 1), (-1, -2 if has_total_row else -1),
         [colors.white, PALETTE["alt_row"]]),
    ]
    if has_total_row:
        cmd += [
            ("BACKGROUND", (0, -1), (-1, -1), PALETTE["total_bg"]),
            ("FONTNAME",   (0, -1), (-1, -1), "Helvetica-Bold"),
        ]
    for ci, rule in enumerate(rules, start=2):
        bg = SEVERITY_COLOR.get(rule_severity.get(rule, ""), colors.white)
        cmd.append(("BACKGROUND", (ci, 0), (ci, 0), bg))
    return TableStyle(cmd)


def build_table(data, rules, rule_severity, col_widths):
    ts = make_table_style(rules, rule_severity)
    t = Table(data, colWidths=col_widths, repeatRows=1)
    t.setStyle(ts)
    return t


def col_widths_for(n_rules, W, dir_col_cm=4.5):
    total_col = 1.2
    fixed = dir_col_cm + total_col
    rule_w = max(1.0, (W / cm - fixed) / n_rules) * cm
    return [dir_col_cm * cm, total_col * cm] + [rule_w] * n_rules


def page_footer(canvas, doc):
    canvas.saveState()
    canvas.setFont("Helvetica", 7)
    canvas.setFillColor(colors.HexColor("#888888"))
    canvas.drawRightString(
        A4[0] - 1.5 * cm, 1.2 * cm,
        f"ArkCompiler Linter Report — page {doc.page}",
    )
    canvas.restoreState()


# ---------------------------------------------------------------------------
# Report builder
# ---------------------------------------------------------------------------

def build_pdf(output_path: str, rule_severity: dict, violations: list,
              scan_label: str, mode_label: str):
    styles = getSampleStyleSheet()

    title_style = ParagraphStyle(
        "ReportTitle", parent=styles["Title"],
        fontSize=20, spaceAfter=6, textColor=colors.HexColor("#1a1a2e"),
    )
    h1_style = ParagraphStyle(
        "H1", parent=styles["Heading1"],
        fontSize=14, spaceBefore=16, spaceAfter=6,
        textColor=colors.HexColor("#16213e"),
    )
    h2_style = ParagraphStyle(
        "H2", parent=styles["Heading2"],
        fontSize=11, spaceBefore=12, spaceAfter=4,
        textColor=colors.HexColor("#0f3460"),
    )

    doc = SimpleDocTemplate(
        output_path, pagesize=A4,
        leftMargin=1.5 * cm, rightMargin=1.5 * cm,
        topMargin=2 * cm, bottomMargin=2 * cm,
    )
    W = A4[0] - 3 * cm
    story = []

    # Order rules by global count (descending)
    rule_counts = defaultdict(int)
    for rule, _sev, _p, _ln in violations:
        rule_counts[rule] += 1
    all_rules = sorted(rule_counts, key=lambda r: -rule_counts[r])

    # ── Cover ──────────────────────────────────────────────────────────────
    story.append(Spacer(1, 2 * cm))
    story.append(Paragraph("ArkCompiler Runtime Core", title_style))
    story.append(Paragraph("Static Linter Report", ParagraphStyle(
        "Sub", parent=styles["Normal"], fontSize=14,
        textColor=colors.HexColor("#555555"), spaceAfter=4,
    )))
    story.append(Paragraph(
        f"Generated: {datetime.datetime.now().strftime('%Y-%m-%d %H:%M')} &nbsp;|&nbsp; "
        f"Scan path: <code>{scan_label}</code> &nbsp;|&nbsp; "
        f"Mode: <code>{mode_label}</code> &nbsp;|&nbsp; "
        f"Total violations: <b>{len(violations)}</b>",
        ParagraphStyle("Meta", parent=styles["Normal"], fontSize=9,
                       textColor=colors.HexColor("#666666"), spaceAfter=16),
    ))
    story.append(HRFlowable(width="100%", thickness=1, color=PALETTE["header_bg"]))
    story.append(Spacer(1, 0.5 * cm))

    cw = col_widths_for(len(all_rules), W)

    # ── 1. Summary by top-level directory ──────────────────────────────────
    story.append(Paragraph("1. Summary by Top-Level Directory", h1_style))
    by_top, top_total = aggregate(violations, lambda p: p[0])
    data = make_table_data(by_top, top_total, all_rules, label="Directory")
    story.append(build_table(data, all_rules, rule_severity, cw))
    story.append(Spacer(1, 0.4 * cm))

    # ── 2. static_core → second level ──────────────────────────────────────
    sc_viol = [(r, s, p, ln) for r, s, p, ln in violations if p[0] == "static_core"]
    if sc_viol:
        story.append(Paragraph("2. static_core — by Second-Level Directory", h1_style))
        by_l2, l2_total = aggregate(sc_viol, lambda p: "/".join(p[:2]))
        data = make_table_data(by_l2, l2_total, all_rules, label="static_core/…")
        story.append(build_table(data, all_rules, rule_severity, cw))
        story.append(Spacer(1, 0.4 * cm))

    # ── 3. plugins/ets → fourth level ──────────────────────────────────────
    ets_viol = [
        (r, s, p, ln) for r, s, p, ln in violations
        if len(p) >= 3 and "/".join(p[:3]) == "static_core/plugins/ets"
    ]
    if ets_viol:
        story.append(Paragraph(
            "3. static_core/plugins/ets — by Fourth-Level Directory", h1_style))
        by_l4, l4_total = aggregate(ets_viol, lambda p: "/".join(p[:4]))
        data = make_table_data(by_l4, l4_total, all_rules, label="…/ets/…")
        cw4 = col_widths_for(len(all_rules), W, dir_col_cm=5.5)
        story.append(build_table(data, all_rules, rule_severity, cw4))

    story.append(PageBreak())

    # ── 4. Per-rule summary ─────────────────────────────────────────────────
    story.append(Paragraph("4. Per-Rule Violation Summary", h1_style))
    total_all = len(violations)
    rule_data = [["Rule", "Severity", "Count", "% of Total"]]
    for rule in all_rules:
        cnt = rule_counts[rule]
        sev = rule_severity.get(rule, "")
        rule_data.append([rule, sev, str(cnt), f"{cnt / total_all * 100:.1f}%"])
    rule_data.append(["TOTAL", "", str(total_all), "100%"])

    ts_rule = TableStyle([
        ("BACKGROUND",    (0, 0),  (-1, 0),  PALETTE["header_bg"]),
        ("TEXTCOLOR",     (0, 0),  (-1, 0),  PALETTE["header_fg"]),
        ("FONTNAME",      (0, 0),  (-1, 0),  "Helvetica-Bold"),
        ("FONTSIZE",      (0, 0),  (-1, -1), 8),
        ("ALIGN",         (2, 1),  (-1, -1), "RIGHT"),
        ("ALIGN",         (0, 0),  (1, -1),  "LEFT"),
        ("GRID",          (0, 0),  (-1, -1), 0.3, PALETTE["grid"]),
        ("TOPPADDING",    (0, 0),  (-1, -1), 4),
        ("BOTTOMPADDING", (0, 0),  (-1, -1), 4),
        ("ROWBACKGROUNDS", (0, 1), (-1, -2), [colors.white, PALETTE["alt_row"]]),
        ("BACKGROUND",    (0, -1), (-1, -1), PALETTE["total_bg"]),
        ("FONTNAME",      (0, -1), (-1, -1), "Helvetica-Bold"),
    ])
    for ri, row in enumerate(rule_data[1:-1], start=1):
        sev = row[1]
        if sev in SEVERITY_COLOR:
            ts_rule.add("TEXTCOLOR", (1, ri), (1, ri), SEVERITY_COLOR[sev])
    rt = Table(rule_data, colWidths=[3 * cm, 2.5 * cm, 2.5 * cm, 2.5 * cm])
    rt.setStyle(ts_rule)
    story.append(rt)
    story.append(PageBreak())

    # ── 5. Detailed violations ──────────────────────────────────────────────
    story.append(Paragraph("5. Detailed Violations", h1_style))

    detail = defaultdict(lambda: defaultdict(list))
    for rule, _sev, pparts, lineno in violations:
        detail[pparts[0]][rule].append(f"{'/'.join(pparts)}:{lineno}")

    path_style = ParagraphStyle(
        "PathList", parent=styles["Normal"],
        fontSize=6.5, leading=9, leftIndent=24,
        fontName="Courier", textColor=colors.HexColor("#333333"),
    )
    rule_head_style = ParagraphStyle(
        "RuleHead", parent=styles["Normal"],
        fontSize=8, spaceBefore=6, spaceAfter=2, leftIndent=12,
    )

    for topdir in sorted(detail):
        story.append(Paragraph(f"Directory: <b>{topdir}/</b>", h2_style))
        for rule in all_rules:
            if rule not in detail[topdir]:
                continue
            paths = detail[topdir][rule]
            sev = rule_severity.get(rule, "")
            sev_col = "#c0392b" if sev == "Error" else "#e67e22"
            story.append(Paragraph(
                f'<font color="{sev_col}"><b>{rule}</b></font> '
                f'<font color="#888888">({sev})</font> — {len(paths)} violation(s)',
                rule_head_style,
            ))
            shown = paths[:60]
            text = "<br/>".join(shown)
            if len(paths) > 60:
                text += f"<br/><i>… and {len(paths) - 60} more</i>"
            story.append(Paragraph(text, path_style))
        story.append(Spacer(1, 0.3 * cm))

    doc.build(story, onFirstPage=page_footer, onLaterPages=page_footer)
    print(f"PDF written to {output_path}")


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    args = parse_args()

    if args.path:
        scan_label = args.path
        linter_output = run_linter(args.linter, args.path)
    else:
        scan_label = args.input
        with open(args.input) as f:
            linter_output = f.read()

    rule_severity, violations = load_violations(linter_output)
    if not violations:
        sys.exit("No violations found in input — nothing to report.")

    print(f"Loaded {len(violations)} violations, {len(rule_severity)} rules.", file=sys.stderr)
    build_pdf(args.output, rule_severity, violations,
              scan_label=scan_label, mode_label="full")


if __name__ == "__main__":
    main()

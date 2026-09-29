#!/usr/bin/env python3
"""Summarize a CTest JUnit file (ctest --output-junit) for a CI job.

    ctest --preset linux-clang --output-junit ctest-junit.xml --test-output-size-passed 65536
    python3 .github/scripts/ctest_report.py build/linux-clang/ctest-junit.xml \
        --require 'esia_conformance_(opengl|gles|vulkan)' --title linux-clang

* prints every test with its status, and every conformance scene line (PASS / FAIL / SKIP with its deltas) from
  the tests' output, and appends the same as Markdown to $GITHUB_STEP_SUMMARY when it is set;
* fails (exit 1) when a test matching a --require pattern is missing or was skipped: a conformance test that
  exits 77 (no device for the backend) shows as skipped in CTest, and a job must not pass by skipping the backends
  it exists to run;
* fails when a test failed, unless every scene it failed is a --known-failure 'TEST_REGEX:SCENE:REASON' (SCENE '*'
  for a test that fails without scene lines, e.g. a crash): those are listed with their reason and the scene's
  numbers, and a known failure that no longer fails is reported so its entry can go. The job's ctest step leaves
  the verdict to this script (it reads the JUnit file, which has every result).
"""
import argparse
import os
import re
import sys
import xml.etree.ElementTree as ET

SCENE = re.compile(r'^(PASS|FAIL|SKIP|NO GOLDEN)\s+(\S+)\s+(\S+)\s*(.*)$')


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('junit')
    ap.add_argument('--require', action='append', default=[], help='regex (full match) of tests that must run')
    ap.add_argument('--known-failure', action='append', default=[], help="'TEST_REGEX:SCENE:REASON'")
    ap.add_argument('--title', default='ctest')
    args = ap.parse_args()

    if not os.path.exists(args.junit):
        sys.exit('%s: missing (ctest did not run?)' % args.junit)
    root = ET.parse(args.junit).getroot()
    tests = []
    for tc in root.iter('testcase'):
        name = tc.get('name')
        if tc.find('failure') is not None:
            status = 'failed'
        elif tc.find('skipped') is not None or tc.get('status') in ('notrun', 'disabled'):
            status = 'skipped'
        else:
            status = 'passed'
        out = tc.findtext('system-out') or ''
        scenes = [m.groups() for m in (SCENE.match(l.strip()) for l in out.splitlines()) if m]
        # a skipped conformance test: its scenes' SKIP lines carry the reason
        reason = ''
        if status == 'skipped':
            reasons = sorted({s[3] for s in scenes if s[0] == 'SKIP' and s[3]})
            reason = '; '.join(reasons) if reasons else (tc.find('skipped').get('message', '') if tc.find('skipped') is not None else '')
        tests.append((name, status, float(tc.get('time') or 0), reason, scenes))

    lines = ['### %s: CTest' % args.title, '',
             '%d tests: %d passed, %d failed, %d skipped' % (
                 len(tests), sum(t[1] == 'passed' for t in tests), sum(t[1] == 'failed' for t in tests),
                 sum(t[1] == 'skipped' for t in tests)), '',
             '| Test | Status | Time (s) | Skip reason |', '| --- | --- | --- | --- |']
    for name, status, time, reason, _ in tests:
        lines.append('| `%s` | %s | %.1f | %s |' % (name, status, time, reason.replace('|', '/')))
    conf = [t for t in tests if t[4] and t[1] != 'skipped']
    if conf:
        lines += ['', '<details><summary>Conformance scenes</summary>', '', '| Test | Result | Scene | Detail |',
                  '| --- | --- | --- | --- |']
        for name, _, _, _, scenes in conf:
            for result, backend, scene, detail in scenes:
                lines.append('| `%s` | %s | %s | %s |' % (name, result, scene, detail.replace('|', '/')))
        lines += ['', '</details>']

    known = []
    for k in args.known_failure:
        test_rx, scene, reason = k.split(':', 2)
        known.append((re.compile(test_rx), scene, reason, [False]))

    def known_for(test, scene):
        for rx, sc, reason, used in known:
            if rx.fullmatch(test) and sc == scene:
                used[0] = True
                return reason
        return None

    errors = []
    excused = []
    for name, status, _, _, scenes in tests:
        if status != 'failed':
            continue
        failed = [s for s in scenes if s[0] in ('FAIL', 'NO GOLDEN')]
        if not failed:
            reason = known_for(name, '*')
            if reason:
                excused.append((name, '*', reason, 'failed without a scene result (crash or abort)'))
            else:
                errors.append('%s failed' % name)
            continue
        for result, _, scene, detail in failed:
            reason = known_for(name, scene)
            if reason:
                excused.append((name, scene, reason, detail))
            else:
                errors.append('%s: %s %s (%s)' % (name, result, scene, detail))
    if excused:
        lines += ['', '**Known failures** (not counted; each has an entry in the workflow):', '',
                  '| Test | Scene | Result | Why |', '| --- | --- | --- | --- |']
        for name, scene, reason, detail in excused:
            lines.append('| `%s` | %s | %s | %s |' % (name, scene, detail.replace('|', '/'), reason.replace('|', '/')))
    stale = ['%s:%s' % (rx.pattern, sc) for rx, sc, _, used in known if not used[0]]
    for entry in stale:
        lines.append('')
        lines.append('Known failure `%s` did not fail in this run: remove its entry if it stays so.' % entry)

    for pattern in args.require:
        rx = re.compile(pattern)
        matched = [t for t in tests if rx.fullmatch(t[0])]
        if not matched:
            errors.append('no test matches the required pattern %r' % pattern)
        for name, status, _, reason, _ in matched:
            if status == 'skipped':
                errors.append('%s was skipped (%s): this job exists to run it' % (name, reason or 'no reason'))
    if errors:
        lines += ['', '**Failures:**', ''] + ['* %s' % e for e in errors]

    text = '\n'.join(lines) + '\n'
    print(text)
    summary = os.environ.get('GITHUB_STEP_SUMMARY')
    if summary:
        with open(summary, 'a', encoding='utf-8') as f:
            f.write(text + '\n')
    for entry in stale:
        print('::warning::known failure %s did not fail' % entry)
    for name, scene, reason, _ in excused:
        print('::warning::known failure: %s %s (%s)' % (name, scene, reason))
    if errors:
        for e in errors:
            print('::error::%s' % e)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())

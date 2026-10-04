#!/bin/bash
# git_commit_all.sh
# Run this ONCE on Linux (after copying the project there) to create
# the full commit history representing each project stage.
#
# Usage: bash scripts/git_commit_all.sh

set -e
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

echo "[*] Setting up Git identity..."
git config user.name  "VSensor Project"
git config user.email "vsensor@project.local"

# ── Stage 1 & 2 ─────────────────────────────────────────────────────
git checkout develop 2>/dev/null || git checkout -b develop

git add docs/stage1/ docs/stage2/
git commit -m "docs: add stage 1 introduction and stage 2 PRD" \
           --allow-empty-message 2>/dev/null || true

# ── Stage 3 ─────────────────────────────────────────────────────────
git checkout -b feature/system-design 2>/dev/null || git checkout feature/system-design

git add docs/stage3/ diagrams/
git commit -m "docs: add stage 3 system architecture and UML diagrams"

git checkout develop
git merge --no-ff feature/system-design -m "merge: stage 3 system design"

# ── Stage 4 – Kernel module ─────────────────────────────────────────
git checkout -b feature/kernel-module

git add kernel/vsensor.h kernel/vsensor.c kernel/Makefile
git commit -m "feat: add vsensor kernel module with cdev, ring buffer, ioctl"

git checkout develop
git merge --no-ff feature/kernel-module -m "merge: kernel module"

# ── Stage 4 – Userspace ─────────────────────────────────────────────
git checkout -b feature/userspace-app

git add userspace/include/ userspace/src/ userspace/CMakeLists.txt
git commit -m "feat: add C++ userspace app (DeviceReader, Dashboard, Logger, Parser)"

git checkout develop
git merge --no-ff feature/userspace-app -m "merge: userspace application"

# ── Stage 4 – Scripts ───────────────────────────────────────────────
git add scripts/
git commit -m "chore: add build, load, unload, and test helper scripts"

# ── Stage 4 docs ────────────────────────────────────────────────────
git add docs/stage4/
git commit -m "docs: add stage 4 implementation notes and prototype documentation"

# ── Stage 5 – Tests ─────────────────────────────────────────────────
git checkout -b feature/tests

git add tests/
git commit -m "test: add 22 unit tests across 4 suites (CircularBuffer, Parser, Stats, Alerts)"

git checkout develop
git merge --no-ff feature/tests -m "merge: unit test suite"

git add docs/stage5/
git commit -m "docs: add stage 5 testing report and integration test documentation"

# ── Stage 6 – Final ─────────────────────────────────────────────────
git add docs/stage6/
git commit -m "docs: add stage 6 final report"

# ── Merge to master and tag ─────────────────────────────────────────
git checkout master
git merge --no-ff develop -m "release: v1.0 final delivery"
git tag -a v1.0 -m "VSensor v1.0 – final project submission"

echo ""
echo "[+] Git history created successfully."
echo "    Branches: master, develop, feature/kernel-module,"
echo "              feature/userspace-app, feature/tests, feature/system-design"
echo ""
git log --oneline --graph --all

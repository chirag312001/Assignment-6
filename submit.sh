#!/usr/bin/env bash
# Package submission: creates a tarball with code + metadata
set -euo pipefail

TEAM=${1:-team_unknown}
OUTPUT="submit_${TEAM}_$(date +%Y%m%d_%H%M%S).tar.gz"

echo "Preparing submission package for team: ${TEAM}"

# Remove old submission archives for this team (if any)
shopt -s nullglob
for f in *"${TEAM}"*.tar.gz; do
    rm -f "$f"
done
shopt -u nullglob


# Create tarball
tar -czf "${OUTPUT}" \
    1.minishell \
    3.lexor \
    4.VM(ASS)noGC \
    5.VM(ASS)withGC \
    assi \
    readme.md \
    report.pdf \
    submit.sh 

echo "Created submission package: ${OUTPUT}"
echo "Please upload ${OUTPUT} on Moodle."
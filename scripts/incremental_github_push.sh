#!/bin/bash
# Rebuild as small commits and push each chunk (avoids GitHub HTTP 408).
set -euo pipefail
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$ROOT"

git config http.postBuffer 524288000
git config http.lowSpeedLimit 0
git config http.lowSpeedTime 999999
git config http.version HTTP/1.1

commit_if_staged() {
	local msg="$1"
	if git diff --cached --quiet; then
		echo "skip (empty): $msg"
		return 0
	fi
	git -c core.hooksPath=/dev/null commit --quiet -m "$msg"
	echo "committed: $msg ($(git rev-parse --short HEAD))"
}

push_main() {
	local force="${1:-}"
	echo "== push $(git rev-parse --short HEAD) ${force} =="
	if [ "$force" = "--force" ]; then
		git push -u origin HEAD:main --force
	else
		git push origin HEAD:main
	fi
}

echo "== orphan rebuild =="
git checkout --orphan push-tmp
git reset

# 1) meta + apps
git add .gitignore install_tools.sh apps scripts/incremental_github_push.sh
git add README.md apps/README.md 2>/dev/null || true
commit_if_staged "Add apps demos, gitignore, and push helper"
push_main --force

# 2) device / build / buildroot / rtos
git add T113-i_v1.0/device T113-i_v1.0/build T113-i_v1.0/buildroot T113-i_v1.0/rtos-dsp
git add T113-i_v1.0/build.sh 2>/dev/null || true
git add T113-i_v1.0/*.sh T113-i_v1.0/*.md T113-i_v1.0/*.txt 2>/dev/null || true
commit_if_staged "Add device, build, buildroot, rtos"
push_main

# 3) brandy
git add T113-i_v1.0/brandy
commit_if_staged "Add brandy/u-boot sources"
push_main

# 4) tools + test
git add T113-i_v1.0/tools T113-i_v1.0/test
commit_if_staged "Add tools and test (slim)"
push_main

# 5) platform
git add T113-i_v1.0/platform
commit_if_staged "Add platform sources (slim)"
push_main

# 6) kernel core (no drivers yet)
git add \
	T113-i_v1.0/kernel/linux-5.4/Makefile \
	T113-i_v1.0/kernel/linux-5.4/Kbuild \
	T113-i_v1.0/kernel/linux-5.4/Kconfig \
	T113-i_v1.0/kernel/linux-5.4/COPYING \
	T113-i_v1.0/kernel/linux-5.4/CREDITS \
	T113-i_v1.0/kernel/linux-5.4/MAINTAINERS \
	T113-i_v1.0/kernel/linux-5.4/README \
	T113-i_v1.0/kernel/linux-5.4/arch \
	T113-i_v1.0/kernel/linux-5.4/include \
	T113-i_v1.0/kernel/linux-5.4/scripts \
	T113-i_v1.0/kernel/linux-5.4/kernel \
	T113-i_v1.0/kernel/linux-5.4/mm \
	T113-i_v1.0/kernel/linux-5.4/lib \
	T113-i_v1.0/kernel/linux-5.4/crypto \
	T113-i_v1.0/kernel/linux-5.4/block \
	T113-i_v1.0/kernel/linux-5.4/security \
	T113-i_v1.0/kernel/linux-5.4/virt \
	T113-i_v1.0/kernel/linux-5.4/usr \
	T113-i_v1.0/kernel/linux-5.4/init \
	T113-i_v1.0/kernel/linux-5.4/ipc \
	T113-i_v1.0/kernel/linux-5.4/certs \
	T113-i_v1.0/kernel/linux-5.4/fs \
	T113-i_v1.0/kernel/linux-5.4/net \
	T113-i_v1.0/kernel/linux-5.4/sound \
	T113-i_v1.0/kernel/linux-5.4/tools \
	T113-i_v1.0/kernel/linux-5.4/samples \
	T113-i_v1.0/kernel/linux-5.4/firmware 2>/dev/null || true
git add T113-i_v1.0/kernel/*.sh T113-i_v1.0/kernel/*.md 2>/dev/null || true
commit_if_staged "Add kernel core (without drivers)"
push_main

# 7) drivers except net/gpu
git add T113-i_v1.0/kernel/linux-5.4/drivers
git rm -r --cached --ignore-unmatch \
	T113-i_v1.0/kernel/linux-5.4/drivers/net \
	T113-i_v1.0/kernel/linux-5.4/drivers/gpu >/dev/null 2>&1 || true
commit_if_staged "Add kernel drivers (except net/gpu)"
push_main

# 8) gpu
git add T113-i_v1.0/kernel/linux-5.4/drivers/gpu
commit_if_staged "Add kernel drivers/gpu (slim)"
push_main

# 9) net
git add T113-i_v1.0/kernel/linux-5.4/drivers/net
commit_if_staged "Add kernel drivers/net"
push_main

# 10) leftovers
git add -A
commit_if_staged "Add remaining SDK files"
push_main

git branch -M main
echo "DONE → $(git rev-parse --short HEAD)"
git log --oneline | head -20
du -sh .git

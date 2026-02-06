#!/bin/bash
set -euo pipefail

BUILD_DIR="${TMPDIR:-/tmp}"/oly_build
if [[ -n "${BASH_SOURCE[0]:-}" && -f "${BASH_SOURCE[0]}" ]]; then
	SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
else
	SCRIPT_DIR=""
fi
if git -C "$SCRIPT_DIR" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
	BUILD_DIR="$SCRIPT_DIR"
else
	BUILD_DIR="${TMPDIR:-/tmp}/oly_build"
	if [[ ! -d "$BUILD_DIR/.git" ]]; then
		git clone https://github.com/anonymousgrasshopper/oly "$BUILD_DIR"
	fi
fi
cd "$BUILD_DIR"

cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build

sudo cp build/bin/oly /usr/local/bin/oly

[[ -d ~/.local/share/typst/packages/local/ ]] || mkdir -p ~/.local/share/typst/packages/local/
cp -r assets/typst/packages/local/oly ~/.local/share/typst/packages/local/
[[ -d /usr/local/share/zsh/site-functions ]] || sudo mkdir -p /usr/local/share/zsh/site-functions/
sudo cp assets/extras/_oly /usr/local/share/zsh/site-functions/

LOCAL_BIN="$HOME/.local/bin"
[[ -d ~/.local/bin ]] || mkdir "$LOCAL_BIN"
cp assets/typst/oly-scheme-handler/oly-handler "$LOCAL_BIN"
if [[ ":$PATH:" != *":$LOCAL_BIN:"* ]]; then
	echo "⚠️  $LOCAL_BIN is not in your PATH."
	echo "Add this to your shell config (e.g. ~/.zshrc or ~/.bashrc):"
	echo
	echo "    export PATH=\"\$HOME/.local/bin:\$PATH\""
	echo
fi
[[ -d ~/.local/share/applications ]] || mkdir ~/.local/share/applications
cp assets/typst/oly-scheme-handler/oly.desktop ~/.local/share/applications/
xdg-mime default oly.desktop x-scheme-handler/oly

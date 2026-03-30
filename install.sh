#!/bin/bash

set -euo pipefail

# parse arguments
GLOBAL=""
while [[ $# -gt 0 ]]; do
	case "$1" in
	--global | -g) GLOBAL=1 ;;
	esac
	shift
done

BUILD_DIR="${BUILD_DIR:-}"
if [[ -z "$GLOBAL" ]]; then
	INSTALL_DIR="${INSTALL_DIR:-$HOME/.local/bin}"
	SHARE_DIR="${SHARE_DIR:-$HOME/.local/share}"
	ZSH_SITE_FUNCTIONS="${ZSH_SITE_FUNCTIONS:-$HOME/.local/share/zsh/completions}"
	SUDO=""
else
	INSTALL_DIR="${INSTALL_DIR:-/usr/local/bin}"
	SHARE_DIR="${SHARE_DIR:-/usr/share}"
	ZSH_SITE_FUNCTIONS="${ZSH_SITE_FUNCTIONS:-/usr/local/share/zsh/site-functions}"
	SUDO="sudo"
fi

if [[ -z "$BUILD_DIR" && -n "${BASH_SOURCE[0]:-}" && -f "${BASH_SOURCE[0]}" ]]; then
	SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
	if git -C "$SCRIPT_DIR" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
		BUILD_DIR="$SCRIPT_DIR"
	fi
fi
if [[ -z "$BUILD_DIR" ]]; then
	BUILD_DIR="${TMPDIR:-/tmp}/oly_build"
	if [[ ! -d "$BUILD_DIR/.git" ]]; then
		git clone https://github.com/antinomie8/oly "$BUILD_DIR"
	fi
fi
cd "$BUILD_DIR"

cmake -S . -B build/Release -DCMAKE_BUILD_TYPE=Release
cmake --build build/Release --parallel

declare -A files
files["build/Release/oly"]="$INSTALL_DIR"
files["assets/typst/packages/local/oly"]="$SHARE_DIR/typst/packages/local"
files["assets/extras/_oly"]="$ZSH_SITE_FUNCTIONS"
files["assets/app/oly.desktop"]="$SHARE_DIR/applications"
files["assets/app/oly.png"]="$SHARE_DIR/icons/hicolor/48x48/apps"
for file in "${!files[@]}"; do
	dir="${files["$file"]}"
	if [[ ! -d "$dir" ]]; then
		$SUDO mkdir -p "$dir"
	fi
	$SUDO cp -r "$file" "$dir"
done

xdg-mime default oly.desktop x-scheme-handler/oly

# $PATH and $fpath check
if [[ -z "$GLOBAL" && ":$PATH:" != *":$HOME/.local/bin:"* ]]; then
	echo
	echo "⚠️ ~/.local/bin is not in your PATH."
	echo "Add this to your shell config:"
	echo 'export PATH="$HOME/.local/bin:$PATH"'
fi
if [[ -z "$GLOBAL" && "${SHELL:-}" = *zsh ]]; then
	if ! zsh -ic 'print -l $fpath' 2>/dev/null |
		grep -qx "$HOME/.local/share/zsh/completions"; then
		echo
		echo "⚠️  Zsh completion directory not in fpath."
		echo "Add this to your .zshrc:"
		echo 'fpath=("$HOME/.local/share/zsh/completions" $fpath)'
		echo 'autoload -Uz compinit && compinit'
	fi
fi

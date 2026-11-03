# Roadmap

[Português](ROADMAP.md) | **English**

octobrew-tui brings octopkg-tui (OctoPkg's terminal interface for `pkg(8)`) to Homebrew with the same features. Generic layers (processes, i18n, preferences, themes, icons, dialogs, menus, and list model) come from octopkg-tui. The package-manager core was rewritten: `core/brew` replaces `core/pkg`.

## pkg-to-brew mapping

| octopkg-tui (`pkg`) | octobrew-tui (`brew`) | Phase |
|:---|:---|:---|
| `pkg query -a` | `brew info --json=v2 --installed` | 1 ✅ |
| `pkg rquery -a` | Direct read of Homebrew API cache | 1 ✅ |
| `pkg upgrade -n` (outdated packages) | `outdated` field from `brew info` (same as `brew outdated`) | 1 ✅ |
| `automatic` (dependency) | `installed_on_request` | 1 ✅ |
| `pkg lock` (locked) | `brew pin` (pinned): state in phase 1, operation in phase 2 | 1 ✅ / 2 ✅ |
| Files tab | `brew list --formula --verbose` / `brew list --cask` | 1 ✅ |
| `pkg install -n` / `upgrade -n` | `brew install --dry-run` / `brew upgrade --dry-run` | 2 ✅ |
| `pkg remove -n -R` | No dry run: dependents come from already-read receipts (`runtime_dependencies`), without invoking brew | 2 ✅ |
| install / remove / upgrade | `brew install --no-ask [--cask]` / `brew uninstall` / `brew upgrade --no-ask` | 2 ✅ |
| `pkg update -f` (Ctrl+K) | `brew update` | 2 ✅ |
| `pkg clean -a` | `brew cleanup --prune=all` with `--dry-run` preview, plus `brew autoremove` | 2 ✅ |
| doas/sudo | Removed: brew runs as the user; casks with `.pkg` installers request sudo with the screen suspended | 2 ✅ |
| Boot Environment (`bectl`) | Brewfile snapshot (`brew bundle dump`) before the transaction | 2 ✅ |
| News tab (BSD feed) | Homebrew blog Atom feed (`brew.sh/atom.xml`), using `/usr/bin/curl` | 3 ✅ |

## Phases

### Phase 0 - Foundation ✅ (v0.1.0)
- Repository based on octopkg-tui v0.7.2, with GPL-2.0-or-later attribution.
- CMake + Homebrew FTXUI (7.x), with FTXUI 6.1.9 fallback.
- JSON: a dedicated C++17 reader (`core/json`) instead of nlohmann-json, keeping the core dependency-free like octopkg-tui.
- CI: macOS with runner Homebrew; Ubuntu with ASan, UBSan, and fake brew.

### Phase 1 - Read-only ✅ (v0.1.0)
- Installed and Repository lists; Type column (formula or cask).
- States: installed, dependency, outdated, pinned, newer than repository, not installed.
- Info tab with caveats; Files tab.
- Disk usage measured in the background.
- Features inherited with the interface: themes, Preferences, Ctrl+F, F7 (description), F8 (tap instead of category), F11, and alternatives to Mac function keys.

### Phase 2 - Transactions ✅ (v0.2.0)
- +/- marks, Actions tab (3), preview and Alt+M; Ctrl+U (upgrade), Ctrl+K (update), cleanup, pin/unpin, and `brew autoremove`.
- Casks installing `.pkg` files request `sudo`; the screen is suspended, as for doas in octopkg-tui.
- Brewfile snapshot before transactions, with restore support (`brew bundle install --no-upgrade`).
- Homebrew 7 findings: `uninstall` and `cleanup` automatically autoremove packages not shown in the preview (`HOMEBREW_NO_AUTOREMOVE=1` for operations; autoremove is now a separate action). Homebrew 7 asks before acting; without a terminal, the prompt is skipped, and octobrew-tui passes `--no-ask` regardless.

### Phase 3 - News ✅ (v0.3.0)
- News tab (5) with Homebrew blog feed, disk cache, and Ctrl+G.
- The feed is Atom (unlike the BSD projects' RSS); octobrew-tui reads both formats.

### Phase 4 - Homebrew-specific features ✅ (v0.4.0)
- Services tab (7): `brew services list --json`, with start, stop, and restart in the Homebrew menu (Alt+H).
- Taps: `brew tap` (with `brew trust --tap`, required for third-party taps by brew 6+) and `brew untap`; third-party tap packages in Repository view.
- Leaves (`l`), like `brew leaves`, and `brew doctor`, with warnings in Output.
- Export a Brewfile (`brew bundle dump`) and install from one (`brew bundle check` preview, `brew bundle install --no-upgrade`).

### Phase 5 - Distribution ✅ (v1.0.0)
- Own tap, [cleberalves40/homebrew-tap](https://github.com/cleberalves40/homebrew-tap): `brew install cleberalves40/tap/octobrew-tui`. The formula is also in this repository (`homebrew/`), validated in CI with `brew audit --strict --online`, source installation, and `brew test` (see `docs/RELEASING.en.md`).
- Tags and releases from v0.1.0 onward.
- Submit to homebrew-core when the project meets notability requirements (`brew audit --new` passes; only core checks notability).
- Homebrew on Linux: tap CI also installs the formula from source and runs `brew test` on Ubuntu, with Homebrew under `/home/linuxbrew/.linuxbrew`. Integration and UI tests against Linux Homebrew remain.
- Bottles (prebuilt binaries in the tap, using `brew test-bot`) to avoid source builds at install time.

## Risks and open decisions

1. **Mac keyboard:** function keys require fn. Addressed in v0.1.0 with `?`, `s`, `t`, and `Alt+D`. Alt requires enabling "Use Option as Meta" in the terminal.
2. **Homebrew startup time:** each call incurs Ruby startup. The application makes one call for installed packages, reads API cache directly, and runs everything in the background.
3. **API cache format:** internal to Homebrew (`api/internal/`) and may change. The app reads brew 5+ and brew 4 formats, and tests use real samples. If the format changes, Repository view warns; Installed view is unaffected.
4. **Formulas removed from homebrew-core** (exa, neofetch, youtube-dl, etc.) appear only as installed and can be removed (-). Highlighting them in the list remains.
5. **Reinstalling** a current installed package (OctoPkg's `pkg install -f`) was not ported: `brew reinstall` has no `--dry-run` preview.
6. **Intel CI runner:** only `macos-latest` (arm64) for now.

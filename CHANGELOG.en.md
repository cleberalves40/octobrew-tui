# Changelog

[Português](CHANGELOG.md) | **English**

All notable changes to octobrew-tui are recorded here. This changelog follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and the project follows [Semantic Versioning](https://semver.org/) with `vX.Y.Z` tags.

## [1.0.1] - 2026-10-25

Apple keyboard notation for shortcuts.

### Changed
- On macOS, Help, menus, the key bar, and on-screen hints now show shortcuts in the notation used by macOS menus: <kbd>⌃</kbd> (Control), <kbd>⌥</kbd> (Option), <kbd>⇧</kbd> (Shift), <kbd>↩</kbd>, <kbd>⇥</kbd>, <kbd>⌦</kbd> (forward Delete, fn+Backspace), and fn+arrow keys for Page Up, Page Down, Home, and End. For example, ⌥M applies, ⌃K updates Homebrew, and ⌥H opens the Homebrew menu. Insert, which Apple keyboards lack, is removed from the text. The key bar shows ⌥M instead of M-m.
- The Help note for Mac keyboards explains how to enable ⌥ (Option) as Meta in the terminal, and is shown only on macOS.
- Linux is unchanged: PC keyboard names (Alt+M, Ctrl+L, M-m).
- The keys themselves are unchanged, and ⌘ is still unused because the terminal does not pass it to the program.
- README and `docs/KEYMAP.md` now use the same notation and include a Linux equivalence table.

### Fixed
- In an 80-column Portuguese terminal (or with many files in the count), the tab bar could still exceed the width without icons, cutting off Services. Tab names are now removed one at a time from the end until the bar fits, leaving numbers and counts; the current tab always retains its name.
- In Help, a long description (the leaves description in Portuguese) narrowed the shortcut column and misaligned the row; descriptions are now clipped.

## [1.0.0] - 2026-10-25

Distribution (roadmap phase 5): octobrew-tui installs through Homebrew itself, and the interface and shortcuts are stable.

### Added
- Homebrew formula in `homebrew/octobrew-tui.rb` for the [cleberalves40/homebrew-tap](https://github.com/cleberalves40/homebrew-tap): `brew install cleberalves40/tap/octobrew-tui`. Builds from the tag source with CMake and Ninja and depends on Homebrew's `ftxui`; `brew test` checks `--version`, `--help`, and unknown-option output.
- `ci/formula-sha.sh`: points the formula at a published version (tag tarball URL and `sha256`) and verifies it with `--check`. The formula tracks the latest published tag, like the octopkg-tui port.
- CI **Formula** job (`ci/formula-test.sh`): creates a local tap and runs `brew style`, `brew audit --strict --online`, source installation, `brew test`, `brew linkage --test`, and the installed program in a pseudo-terminal. `brew audit --new` reports homebrew-core's new-formula rules without failing (repository notability is only checked in core). The macOS job checks that the formula is not ahead of `CMakeLists.txt` and that `sha256` matches.
- `docs/RELEASING.md`: versioning process (one PR per version against `main`), tags, formula, tap, and submission to homebrew-core.
- Tags and releases v0.1.0 through v0.4.0 on `main` merge commits.

## [0.4.0] - 2026-10-25

Homebrew-specific features (roadmap phase 4): services, taps, leaves, `brew doctor`, and Brewfiles in the new **Homebrew** menu (**Alt+H**).

### Added
- **Services** tab (**7** or **Alt+7**): installed formula services (`brew services list --json`), with state (started, stopped, scheduled, error, not running), user, exit code, and plist. Loaded in the background on first visit, F5, and after each action; Ctrl+F searches it. Homebrew menu actions start, stop, and restart the selected formula service (`brew services start|stop|restart`). The Info tab shows the formula's service.
- **Leaves** (**l** or Search menu): only installed formulas with no dependents, like `brew leaves`; these can be removed without taking anything else with them. The header shows "· leaves"; the filter and tap filter still apply.
- **Taps**: Homebrew > Add a tap... (`brew tap user/repo`, with optional `brew trust --tap`, required by brew 6+ for third-party taps) and Remove a tap... (`brew untap`), which first shows installed packages from that tap because brew refuses to remove a tap containing installed packages.
- Repository view now includes **third-party tap** packages, which are absent from the API cache (`brew tap-info --json --installed` supplies names and taps; versions and descriptions come from package details). If tap-info fails, API packages remain and a warning appears in Output.
- Homebrew > **Check system** (`brew doctor`): shows "ready to brew" or the warning list, with full text in Output. Doctor exits with status 1 when warnings exist; this is not treated as failure.
- Homebrew > **Export a Brewfile...** (`brew bundle dump`, asks before replacing an existing file) and **Install from a Brewfile...**: preview via `brew bundle check --verbose` (missing packages), then `brew bundle install --no-upgrade`; with casks and no cached sudo password, the screen is suspended.
- In narrow terminals where seven tabs exceed 80 columns, tab icons are removed and numbers and names remain.
- Tests cover services, taps and their repository packages, doctor, bundle check, each action with fake brew, leaves in model and view, tabs at 80 columns, and real Homebrew read-only integration for services, taps, doctor, and bundle check.

## [0.3.0] - 2026-10-25

News tab (roadmap phase 3): all OctoPkg tabs are now in octobrew-tui.

### Added
- **News** tab (**5** or **Alt+5**), as in OctoPkg and octopkg-tui: the 10 latest Homebrew blog posts (`https://brew.sh/atom.xml`) with title, date, link, and summary. **Ctrl+G** or File > Fetch latest news downloads again.
- **Atom** reader for brew.sh's Jekyll feed, in addition to octopkg-tui's RSS 2.0: reads `alternate` link, `published` date (`2026-09-13`), `summary` (or start of `content`) as a description clipped to 600 characters, and formats release posts such as "Homebrew 7.0.0" when the title is only the version.
- Feed downloaded with macOS `/usr/bin/curl`, HTTPS only with certificate verification (OctoPkg uses `curl -k`, which does not verify), and cached at `~/.config/octobrew-tui/news.xml`. On first visit, cached posts appear immediately and new ones arrive in the background. If offline or the feed cannot be read, cached posts remain with a warning. New posts mark the tab with `*`.
- **Ctrl+F** now searches the News tab.
- Tests for Homebrew Atom and RSS, description clipping, fake-curl download (arguments, cache, updates, failures, invalid feed), keyboard-driven tab, and real feed download in integration.

## [0.2.0] - 2026-10-25

Transactions: mark, install, remove, upgrade, clean, and pin, as in OctoPkg (roadmap phase 2).

### Added
- OctoPkg marks: **+** (or Insert) to install and **-** (or Delete) to remove; press again to unmark. An installed package is marked for install only if outdated (brew install updates it); pinned packages are not marked; `ftxui` (a dependency) and `octobrew-tui` itself cannot be removed.
- **Actions** tab (3), listing packages to install and remove; **Ctrl+E** clears marks; status bar shows marked count.
- **Alt+M** applies changes. First, previews with `brew install --dry-run` (formulas and casks in separate calls, which brew does not accept together) and lists installed dependents for removals. Because brew will not remove packages used by others, dependents are included, as in OctoPkg's `pkg remove -R`; a red **Warning** dialog appears with Cancel focused. A pinned or protected dependent blocks removal. Then `brew uninstall` (casks first) and `brew install` run.
- **Ctrl+U** installs updates (`brew upgrade`) with `brew upgrade --dry-run` preview; pinned packages are skipped and brew warnings (disabled casks) go to Output. **Ctrl+K** updates Homebrew and the list (`brew update`).
- File > Clear local cache (`brew cleanup --prune=all`), with preview of removals and space reclaimed; File > Remove unnecessary dependencies (`brew autoremove`), with a list; Actions > Pin and Unpin (`brew pin`/`unpin`, formulas and casks).
- **Brewfile snapshot** before transactions, in place of OctoPkg Boot Environments: `brew bundle dump` to `~/.config/octobrew-tui/snapshots/YYMMDD-HHMMSS.Brewfile`, enabled for install and upgrade and disabled for removal. No shell is used; if snapshot creation fails, nothing changes. File > Restore a snapshot... (`brew bundle install --no-upgrade`) installs missing packages from it.
- Casks requiring **sudo** (such as `.pkg` installers or system steps during uninstall) are identified in the dialog. If sudo has no cached password, the screen is suspended while it prompts, as with doas in octopkg-tui. Otherwise, operations run in the background with live Output.
- **Actions** menu (**Alt+C**); key bar shows **+/-** and **M-m**.
- Tests cover brew 7 previews (install, upgrade, autoremove, cleanup), dependents, casks requiring sudo, snapshots, operations against fake brew (arguments and environment), marks, keyboard-driven dialogs and menus, and previews against real brew. With `--transactions` (`tests/macos-e2e.sh`, in CI), integration installs, pins, and removes figlet.

### Changed
- Operations skip auto-update (so behavior matches preview), brew 7 prompts (`--no-ask`, `HOMEBREW_NO_ASK`), and automatic autoremove during `uninstall` and `cleanup` (`HOMEBREW_NO_AUTOREMOVE`), which could remove packages missing from the preview. Without a terminal, sudo requiring a password fails immediately (`SUDO_ASKPASS=/usr/bin/false`) rather than competing for the screen.
- Quit (q, F10, Ctrl+Q) is rejected while an operation is running.

## [0.1.0] - 2026-10-25

First release: read-only (roadmap phases 0 and 1).

### Added
- Repository based on octopkg-tui v0.7.2. Unchanged: shell-free process execution, i18n (en_US, pt_BR, es_ES), preferences, 15 themes, icons, dialogs, menus, tab search (Ctrl+F), description search (F7), and F11 layouts.
- Homebrew core (`src/core/brew.*`) replacing `pkg.*`:
  - installed packages from `brew info --json=v2 --installed`: formulas and casks in one call, with outdated status, pins (`brew pin`, analogous to pkg locks), dependencies (`installed_on_request`), and dependent counts;
  - repository read directly from Homebrew API cache, without network or Ruby: `packages.<tag>.jws.json` for brew 5+ and `formula.jws.json` plus `cask.jws.json` for brew 4; about 16,000 packages in 0.3 seconds;
  - details via `brew info --json=v2 --formula|--cask`, files via `brew list`;
  - all queries use `HOMEBREW_NO_AUTO_UPDATE=1`, without colors or emoji; a query never triggers `brew update`.
- Dedicated JSON reader (`src/core/json.*`) in pure C++17, like the rest of the core.
- **Type** column (formula or cask). A formula and cask with the same name are separate rows with distinct selection.
- Info tab with tap, homepage, license, keg-only status, cask auto-update, deprecated/disabled warnings, runtime and build dependencies, "Required by", conflicts, cask artifacts, and caveats wrapped to tab width.
- Disk usage calculated in the background after listing. For formulas, includes all Cellar versions; for casks, Caskroom and app under `/Applications`. A new listing or exit cancels calculation.
- **F8** filters by tap (homebrew/core, homebrew/cask, third-party taps), replacing the port category filter.
- Alternatives to Mac function keys, which may require fn: `?` (F1), `s` (F6), `Alt+D` (F7), and `t` (F8).
- Tests: `test_core` (JSON, brew 7, API cache, fake brew, sizes), `test_model`, `test_view` (including full workflow with fake brew), `test_integration` (real Homebrew, read-only), and `tests/tui-smoke.sh`.
- CI on macOS (runner Homebrew) and Ubuntu (FTXUI 6.1.9, ASan, and UBSan).

### Not yet included
- Actions (3) and News (5) tabs; marking, install, removal, upgrade, and pin: phases 2 and 3.

# octobrew-tui

[Português](README.md) | **English**

[![Build & Test](https://github.com/cleberalves40/octobrew-tui/actions/workflows/build.yml/badge.svg)](https://github.com/cleberalves40/octobrew-tui/actions/workflows/build.yml)
[![License: GPL v2+](https://img.shields.io/badge/License-GPL%20v2%2B-blue.svg)](LICENSE)

A terminal interface (TUI) for [Homebrew](https://brew.sh), the package manager for macOS (and Linux). It is the sibling of [octopkg-tui](https://github.com/cleberalves40/octopkg-tui), the terminal interface for [OctoPkg](https://github.com/aarnt/octopkg) and FreeBSD's `pkg(8)`: the same tabs, shortcuts, themes, and [Nerd Fonts](https://www.nerdfonts.com/) icons, now for formulas and casks.

```
 OctoBrew — Homebrew [# Installed packages]                 / Filter by name…
────────────────────────────────────────────────────────────────────────────────────
   Name ^                       Type     Version                Size  Installed on
────────────────────────────────────────────────────────────────────────────────────
 I alacritty                    Cask     0.17.0            13.97 MiB  2026-04-14 22:13
 d brotli                       Formula  1.2.0              1.67 MiB  2026-10-19 09:12
 U curl                         Formula  8.9.1              4.72 MiB  2026-10-19 09:12
 P wget                         Formula  1.24.5             4.01 MiB  2026-09-02 15:40
────────────────────────────────────────────────────────────────────────────────────
 1 Info  2 Files  3 Actions  4 Output  5 News  6 Help  7 Services
curl  Get a file from an HTTP, HTTPS or FTP server
Status:             Outdated (version 8.10.0 is available)
Type:               Formula
Installed version:  8.9.1
Version:            8.10.0
Tap:                homebrew/core
────────────────────────────────────────────────────────────────────────────────────
Ready.                                   Theme: System  4 package(s)  2 outdated
F1Help  F5Refresh  F6Sort  F8Tap  F9Menu  F10Exit  +/-Mark  M-mApply
```

(With `--icons=ascii`: **I** installed, **d** dependency, **U** outdated, **N** newer than repository, **P** pinned with `brew pin`, **-** not installed; **+** marked for install and **x** for removal.)

## Status

**v1.0.0** (Roadmap phases 1 through 5): all OctoPkg tabs, Homebrew-specific features, and installation through Homebrew itself. The application:

- marks packages for install (**+**) and removal (**-**), previews the transaction, and runs it (**⌥M**) with a Brewfile snapshot first;
- installs updates (**⌃U**), updates Homebrew (**⌃K**), clears the cache, removes unnecessary dependencies, pins and unpins packages, and restores a snapshot;
- lists installed formulas and casks and those in the repository;
- displays details for the selected package, including caveats;
- displays installed package files and disk usage;
- searches by name or description (**F7**) and filters by tap (**F8**);
- sorts by any column (**F6**) and maximizes the list or tabs (**F11**);
- includes octopkg-tui's 15 themes and Preferences;
- displays Homebrew blog news (tab **5**, **⌃G**);
- displays services (tab **7**) and starts, stops, or restarts them; adds and removes taps (including trust support in brew 6+); filters leaves (**l**, like `brew leaves`); runs `brew doctor`; and exports to and installs from Brewfiles, all in the **Homebrew** menu (**⌥H**).

See the [CHANGELOG](CHANGELOG.en.md).

## Install

```sh
brew install cleberalves40/tap/octobrew-tui
brew install --cask font-jetbrains-mono-nerd-font   # icons; optional
octobrew-tui
```

The fully qualified name (`cleberalves40/tap/...`) adds the tap and, with brew 6+, trusts it automatically. Homebrew builds from source with CMake and Ninja; `brew upgrade` installs new versions. See [docs/RELEASING.en.md](docs/RELEASING.en.md) for the release process.

## Build requirements

```sh
brew install ftxui cmake ninja
```

For icons, configure the terminal to use **JetBrainsMono Nerd Font Mono**. Without a Nerd Font, use `--icons=unicode` or `--icons=ascii`.

No Qt, X11, or bundled Ruby: only FTXUI and the system Homebrew installation. It also works over SSH.

## Build

```sh
cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/octobrew-tui
```

Options: `--remote` (start in Repository view), `--icons=nerd|unicode|ascii`, `--lang=en_US|pt_BR|es_ES`, `--theme=THEME`, `--config=FILE`, `--brew=EXECUTABLE`, and `--version`. Preferences are stored in `~/.config/octobrew-tui/config`: table order, search mode, theme, language, and icons (through Preferences, **⌥,**).

## How it reads Homebrew

| Data | Source | Cost |
|:---|:---|:---|
| Installed packages (formulas and casks, including outdated status, pins, and dependencies) | One call to `brew info --json=v2 --installed` | ~2 s (Homebrew Ruby startup) |
| Repository (~16,000 formulas and casks) | Reads Homebrew's API cache directly (`$(brew --cache)/api/internal/packages.*.jws.json`; on brew 4, `formula.jws.json` and `cask.jws.json`), without network access or Ruby | ~0.3 s |
| News (tab 5) | `https://brew.sh/atom.xml`, fetched with `/usr/bin/curl` (HTTPS only, certificate verified), cached in `~/.config/octobrew-tui/news.xml` | On first visit and **⌃G**, in background |
| Third-party tap packages (Repository view) | `brew tap-info --json --installed`: package name and tap | ~1.5 s, alongside repository listing |
| Services (tab 7) | `brew services list --json` | ~2 s, on first visit, F5, and after each action |
| Selected package details | `brew info --json=v2 --formula|--cask NAME` | ~0.5-1 s, in background and cached |
| Files (tab 2) | `brew list --formula --verbose` / `brew list --cask` | Similar |
| Disk usage | Cellar (all versions), Caskroom, and the app in `/Applications`, measured by the application | Background, after listing |

All queries run with `HOMEBREW_NO_AUTO_UPDATE=1`, `HOMEBREW_NO_ENV_HINTS=1`, `HOMEBREW_NO_COLOR=1`, and `HOMEBREW_NO_EMOJI=1`, without a shell. A query never triggers `brew update` or downloads the API. If the API cache is missing (for example, with `HOMEBREW_NO_INSTALL_FROM_API`), Repository view shows a warning and asks for `brew update`.

Formulas removed from homebrew-core remain in the installed list as local-only packages. The same applies to third-party tap formulas, which are not in the API. A formula and cask with the same name (`docker`) appear as separate rows.

Homebrew does not run as root, and neither does octobrew-tui.

## Transactions

| Action | Preview (no changes) | Operation |
|:---|:---|:---|
| ⌥M: install packages marked + | `brew install --dry-run --formula ...` / `--cask ...` | `brew install --no-ask --formula ...` / `--cask ...` |
| ⌥M: remove packages marked - | Dependents from already-read Homebrew receipts | `brew uninstall --cask ...`, then `--formula ...` |
| ⌃U: install updates | `brew upgrade --dry-run` | `brew upgrade --no-ask` |
| ⌃K: update Homebrew | - | `brew update` |
| Clear local cache | `brew cleanup --dry-run --prune=all` | `brew cleanup --prune=all` |
| Remove unnecessary dependencies | `brew autoremove --dry-run` | `brew autoremove` |
| Pin / unpin | - | `brew pin` / `brew unpin` |
| Restore a snapshot | - | `brew bundle install --file=... --no-upgrade` |
| Install from a Brewfile | `brew bundle check --verbose` (missing items) | `brew bundle install --file=... --no-upgrade` |
| Export a Brewfile | - | `brew bundle dump --file=... --force` (asks before replacing) |
| Start / stop / restart a service | - | `brew services start|stop|restart NAME` |
| Add / remove a tap | - | `brew tap user/repo` (and `brew trust --tap`) / `brew untap` |
| Check system | `brew doctor` (read-only) | - |

- **Dependents:** Homebrew does not remove packages that other installed packages use. Direct and indirect dependents of selected packages are included, like OctoPkg's `pkg remove -R`; the dialog becomes a **Warning** with Cancel focused. A pinned dependent blocks removal.
- **Snapshot:** before a transaction, the dialog offers `brew bundle dump` to `~/.config/octobrew-tui/snapshots/YYMMDD-HHMMSS.Brewfile`, the Homebrew equivalent of OctoPkg's Boot Environment. Enabled by default for install and upgrade, disabled for removal. If snapshot creation fails, nothing changes.
- **No surprises:** operations skip auto-update, brew 7's prompt, and automatic autoremove in `uninstall` (which could remove packages not shown in the preview; autoremove is a separate File menu action).
- **sudo:** casks with `.pkg` installers or system steps may request sudo; the dialog warns about this. If sudo has no cached password, the screen is suspended while it prompts in the terminal. Otherwise, the operation runs in the background with live output in the Output tab.
- `ftxui`, a dependency of octobrew-tui, and `octobrew-tui` itself cannot be removed, like `pkg` in OctoPkg.

## Keyboard and mouse

Shortcuts follow octopkg-tui and use **Apple keyboard notation**: <kbd>⌃</kbd> Control, <kbd>⌥</kbd> Option, <kbd>⇧</kbd> Shift, <kbd>⌦</kbd> forward delete (fn+Backspace), and fn+arrow keys for Page Up, Page Down, Home, and End. On Mac keyboards, function keys may require **fn**, so each has an alternative. On Linux, ⌥ is Alt and ⌃ is Ctrl; the interface displays PC keyboard names (Alt+M, Ctrl+L).

| Key | Action |
|:---|:---|
| <kbd>F5</kbd> / <kbd>⌃R</kbd> | Refresh the list |
| <kbd>↑</kbd> <kbd>↓</kbd> <kbd>fn↑</kbd> <kbd>fn↓</kbd> <kbd>fn←</kbd> <kbd>fn→</kbd> (or <kbd>j</kbd> <kbd>k</kbd>) | Move selection |
| <kbd>⌃L</kbd> / <kbd>/</kbd> | Search by name or description (<kbd>Esc</kbd>, <kbd>↩</kbd>, or <kbd>⌃P</kbd> returns to the list) |
| <kbd>F7</kbd> / <kbd>⌥D</kbd> | Search by name or description |
| <kbd>F8</kbd> / <kbd>t</kbd> | Filter by tap (homebrew/core, homebrew/cask, third-party taps); press a letter to jump to a matching tap |
| <kbd>i</kbd> / <kbd>⌥I</kbd> | Switch between installed and repository views |
| <kbd>+</kbd> | Mark package for install (press again to unmark) |
| <kbd>-</kbd> / <kbd>⌦</kbd> | Mark package for removal |
| <kbd>⌥M</kbd> | Apply: preview, confirm, and run transaction |
| <kbd>⌃E</kbd> | Clear marks |
| <kbd>⌃U</kbd> / <kbd>⌃K</kbd> | Install updates / update Homebrew |
| <kbd>F6</kbd> / <kbd>s</kbd> | Choose sort column (press again on the same column to reverse) |
| Click a header | Sort ascending, descending, or in brew order |
| <kbd>1</kbd>...<kbd>6</kbd> / <kbd>⌥1</kbd>...<kbd>⌥6</kbd> | Info, Files, Actions, Output, News, and Help tabs, using OctoPkg numbering |
| <kbd>7</kbd> / <kbd>⌥7</kbd> | Services tab (`brew services`) |
| <kbd>l</kbd> | Show leaves: installed formulas that no other package depends on (`brew leaves`) |
| <kbd>⌃G</kbd> | Fetch the latest Homebrew news (News tab) |
| <kbd>F1</kbd> / <kbd>?</kbd> | Help |
| <kbd>↩</kbd> | Open the selected package's Info tab |
| <kbd>⇥</kbd> | Scroll a tab with the arrows (press again or <kbd>Esc</kbd> to return to the list) |
| <kbd>⌃F</kbd> | Search text in Files, Output, News, and Help tabs (<kbd>F3</kbd> / <kbd>⇧F3</kbd>) |
| <kbd>F9</kbd> / <kbd>⌥A</kbd> <kbd>⌥B</kbd> <kbd>⌥C</kbd> <kbd>⌥H</kbd> <kbd>⌥J</kbd> | File, Search, Actions, Homebrew, and Help menus |
| <kbd>⌥,</kbd> / <kbd>⌥T</kbd> | Preferences (theme, language, icons) / next theme |
| <kbd>F11</kbd> / <kbd>⌥Z</kbd> | Maximize list, then tabs, then restore both |
| <kbd>⌃Q</kbd> / <kbd>F10</kbd> / <kbd>q</kbd> | Quit (not during an operation) |

In macOS Terminal, enable **Use Option as Meta key** for ⌥ shortcuts (Terminal > Settings > Profiles > Keyboard; in iTerm2, *Left Option key: Esc+*). ⌘ does not work because the terminal does not pass it through. The full map is in [docs/KEYMAP.en.md](docs/KEYMAP.en.md).

## Architecture

| Layer | Location | Responsibility |
|:---|:---|:---|
| Core | `src/core/` | `brew.*`: invoke brew without a shell; parse its JSON, API cache, and previews (`--dry-run`); states, dependents, operations, snapshots, and disk usage. `json.*`: a dedicated JSON reader. `news.*`: Atom/RSS feed and cache. `process`, `i18n`, and `settings` are unchanged from octopkg-tui. Pure C++17, no dependencies. |
| Model | `src/model/` | Filtering, sorting (version, type, size, date, state), tap filter, and selection that follows package identity (`f:name` / `c:token`) |
| Interface | `src/tui/` | FTXUI screen, themes, keyboard, mouse, dialogs, menus, and tab search from octopkg-tui; background listing, details, and size calculation |

## Tests

```sh
cmake -B build -S . -DOCTOBREW_TUI_BUILD_TESTS=ON && cmake --build build && ctest --test-dir build
sh tests/macos-e2e.sh build    # all tests, plus real Homebrew query and preview integration
sh tests/macos-e2e.sh --transactions build   # also installs, pins, and removes figlet (CI does this)
sh tests/tui-smoke.sh build    # TUI in a pseudo-terminal with fake brew
```

- `test_core`: JSON reader, brew 7 output (installed packages, formula and cask details), API cache (brew 5+ and brew 4 formats), and a fake brew that checks each call's arguments and environment; disk usage, processes, and preferences.
- `test_model`: filtering, sorting, tap, selection, and scrolling with 40,000 packages.
- `test_view`: renders the screen as text and drives the interface with keyboard events, including the complete listing, repository, details, and size workflow with fake brew.
- `test_integration`: real Homebrew queries, previews, and news feed without changes. With `OCTOBREW_TEST_TRANSACTIONS=1`, installs, pins, and removes figlet (if not already installed).
- `tests/tui-smoke.sh`: TUI in a pseudo-terminal with fake brew; also runs on Ubuntu.

CI runs on macOS (with the runner's Homebrew) and Ubuntu (FTXUI 6.1.9, ASan, and UBSan).

## License

[GPL-2.0-or-later](LICENSE). The interface and model are from octopkg-tui, whose core is a port of OctoPkg code, © Alexandre Albuquerque Arnt (GPL-2.0-or-later).

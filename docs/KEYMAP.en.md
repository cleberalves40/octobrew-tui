# Shortcuts: OctoPkg to octopkg-tui to octobrew-tui

[Português](KEYMAP.md) | **English**

octobrew-tui preserves octopkg-tui shortcuts, which in turn preserves OctoPkg 0.4.2 shortcuts whenever the terminal passes them through. The interface displays **Apple keyboard notation**, like macOS menus:

| Symbol | Key | Linux equivalent |
|:---|:---|:---|
| ⌃ | Control | Ctrl |
| ⌥ | Option | Alt |
| ⇧ | Shift | Shift |
| ↩ | Return | Enter |
| ⇥ | Tab | Tab |
| ⌦ | Forward Delete (fn+Backspace) | Delete |
| fn↑ fn↓ fn← fn→ | Page Up, Page Down, Home, End | PgUp, PgDn, Home, End |

On Linux, the interface shows PC keyboard names (Alt+M, Ctrl+L). On macOS:

- F1-F12 may require **fn** (unless "Use F1, F2, etc. keys as standard function keys" is enabled). Each has an alternative that does not require fn.
- <kbd>⌥</kbd> shortcuts reach the program only when **Use Option as Meta key** is enabled (Terminal > Settings > Profiles > Keyboard; in iTerm2, *Left Option key: Esc+*).
- <kbd>⌘</kbd> does not work: the terminal does not pass it through, so OctoPkg's Ctrl shortcuts use <kbd>⌃</kbd> instead.

As in octopkg-tui, the terminal sends <kbd>⌃I</kbd> as <kbd>⇥</kbd> and <kbd>⌃M</kbd> as <kbd>↩</kbd>, while <kbd>⌃⇧letter</kbd> arrives the same as <kbd>⌃letter</kbd>. Insert, unavailable on Apple keyboards, remains supported on PC keyboards (as does <kbd>+</kbd>).

| Action | OctoPkg | octobrew-tui | Mac alternative (no fn) | Available |
|:---|:---|:---|:---|:---|
| Info / Files / Output / Help tabs | Alt+1 / Alt+2 / Alt+4 / Alt+6 | **1 2 4 6** or **⌥1 ⌥2 ⌥4 ⌥6** | - | v0.1.0 |
| Help | F1 | **F1** | **?** | v0.1.0 |
| Search package list | Ctrl+L | **⌃L** or **/** | - | v0.1.0 |
| Search by name / description | Search menu | **F7**; Search menu (**⌥B**) | **⌥D** | v0.1.0 |
| Filter by tap | - | **F8**; Search menu | **t** | v0.1.0 |
| Maximize list / tabs | F10 / F11 | **F11**, cycles | **⌥Z** | v0.1.0 |
| Return to package list | Ctrl+P | **⌃P** or **Esc** | - | v0.1.0 |
| Installed / repository | Ctrl+I | **i** or **⌥I** | - | v0.1.0 |
| Info tab for selected package | Enter | **↩** | - | v0.1.0 |
| Sort | Click header | Click header or **F6** | **s** | v0.1.0 |
| Refresh list | - | **F5** | **⌃R** | v0.1.0 |
| Scroll current tab | - (mouse) | **⇥**, then arrows, fn↑, fn↓ | - | v0.1.0 |
| Menus | Menu bar | **F9** or **⌥A** (File), **⌥B** (Search), **⌥J** (Help) | **⌥A** | v0.1.0 |
| Search text in Files, Output, News, and Help tabs | Ctrl+F (F3 / Shift+F3) | **⌃F**; **↩**/**F3** and **⇧F3** (or ↓ ↑) | ↓ ↑ | v0.1.0 |
| Preferences / next theme | - | **⌥,** / **⌥T** | - | v0.1.0 |
| Quit | Ctrl+Q | **⌃Q**, **F10**, or **q** | **q** | v0.1.0 |
| Mark for install / removal | + / - | **+** / **-**, or **⌦** | - | v0.2.0 |
| Actions tab | Alt+3 | **3** or **⌥3** | - | v0.2.0 |
| Apply | Ctrl+M | **⌥M** (<kbd>⌃M</kbd> is Enter) | - | v0.2.0 |
| Cancel (clear marks) | Ctrl+E | **⌃E** | - | v0.2.0 |
| Check for updates | Ctrl+K | **⌃K** (`brew update`) | - | v0.2.0 |
| Install updates | Ctrl+U | **⌃U** (`brew upgrade`) | - | v0.2.0 |
| Clear cache | Ctrl+Shift+C | File menu (`brew cleanup`), as in octopkg-tui | - | v0.2.0 |
| Remove unnecessary dependencies | - | File menu (`brew autoremove`) | - | v0.2.0 |
| Lock / unlock | Context menu | Actions menu: Pin / Unpin (`brew pin` / `unpin`) | - | v0.2.0 |
| Restore a snapshot | - (Boot Environments) | File menu | - | v0.2.0 |
| Actions menu | Menu bar | **⌥C** | - | v0.2.0 |
| News tab / fetch latest news | Alt+5 / Ctrl+G | **5** or **⌥5** / **⌃G** | - | v0.3.0 |
| Services tab (`brew services`) | - | **7** or **⌥7** | - | v0.4.0 |
| Leaves only (`brew leaves`) | - | **l**; Search menu | - | v0.4.0 |
| Homebrew menu: services, taps, Brewfiles, brew doctor | - | **⌥H**, or F9 and arrow keys | - | v0.4.0 |

In dialogs, <kbd>⇥</kbd> or the arrow keys change fields, <kbd>Space</kbd> toggles the snapshot option, <kbd>↩</kbd> confirms, and <kbd>Esc</kbd> cancels. Confirmation dialogs also accept <kbd>y</kbd>/<kbd>s</kbd> and <kbd>n</kbd>.

In the sort-column selector (F6/s), tap selector (F8/t), and dialogs, keys follow octopkg-tui: arrows navigate, <kbd>↩</kbd> selects, <kbd>Esc</kbd> closes. In the tap selector, type a letter to jump to the next matching entry.

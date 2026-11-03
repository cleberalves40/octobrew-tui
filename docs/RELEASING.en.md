# Versioning, formula, and tap

[Português](RELEASING.md) | **English**

## Versioning

The project follows [Semantic Versioning](https://semver.org/) with `vX.Y.Z` tags:

- **patch** (`v1.0.1`): fixes and packaging, no new features;
- **minor** (`v1.1.0`): new backward-compatible features;
- **major** (`v2.0.0`): incompatible changes (command-line options, preferences file format, or shortcuts).

The program version is in `CMakeLists.txt`; the formula version is in the `url` and `sha256` in `homebrew/octobrew-tui.rb`. The formula tracks the **latest published tag**: the release cannot contain its own `sha256`, because the tag tarball includes the formula and its checksum would change if that checksum were added. This is the same arrangement as the octopkg-tui port.

| File | Field |
|:---|:---|
| `CMakeLists.txt` | `project(octobrew_tui VERSION X.Y.Z ...)`: source for the version shown by `--version` and About |
| `homebrew/octobrew-tui.rb` | `url` and `sha256`: latest published tag |

CI fails if the CHANGELOG lacks an entry for the CMake version, if the formula is ahead of it, or if `sha256` does not match the tag tarball (`ci/formula-sha.sh --check`).

## Publishing a release

1. On a `release-X.Y.Z` branch, update the version in `CMakeLists.txt` and record the changes in `CHANGELOG.md` using [Keep a Changelog](https://keepachangelog.com/en/1.1.0/). Leave the formula unchanged.
2. Open a pull request **against `main`**. Open one PR per release: a chained PR (based on a `release-*` branch) merges into the lower branch rather than `main` if that branch has not been deleted. All three CI jobs must pass:
   - **macOS**: tests, integration with real Homebrew (including the figlet transaction), and the interface in a pseudo-terminal;
   - **Formula**: `brew audit`, install from source at the tag, `brew test`, and the installed program in a pseudo-terminal (`ci/formula-test.sh`);
   - **Ubuntu**: tests against FTXUI 6.1.9, also with ASan and UBSan.
3. Merge with a *merge commit*.
4. After CI passes on the merge commit, create an annotated tag there and publish the release with the CHANGELOG text and comparison link:

   ```sh
   git fetch origin
   git tag -a vX.Y.Z -m "octobrew-tui vX.Y.Z" origin/main
   git push origin vX.Y.Z
   gh release create vX.Y.Z --verify-tag --title "octobrew-tui vX.Y.Z" --notes-file notes.md
   ```

5. After publishing the tag, point the formula to it on a `formula-X.Y.Z` branch and open a pull request (`formula: vX.Y.Z`). This updates only the formula, not the program version:

   ```sh
   sh ci/formula-sha.sh X.Y.Z          # URL and sha256 for the tag
   sh ci/formula-sha.sh X.Y.Z --check  # same check used by CI
   ```

6. After merging, copy the formula to the tap (below).

## The tap

The tap is [cleberalves40/homebrew-tap](https://github.com/cleberalves40/homebrew-tap), with the formula at `Formula/octobrew-tui.rb`, matching this repository's `homebrew/` copy. To publish a version there:

```sh
git clone https://github.com/cleberalves40/homebrew-tap.git
cp octobrew-tui/homebrew/octobrew-tui.rb homebrew-tap/Formula/
cd homebrew-tap && git checkout -b octobrew-tui-X.Y.Z && git commit -am "octobrew-tui X.Y.Z"
gh pr create --fill   # tap CI runs audit, install, and tests
```

Tap CI (`.github/workflows/tests.yml`) runs `brew style`, `brew audit`, source installation, and `brew test` on macOS and Linux, on every change and every Monday.

Users install and upgrade with:

```sh
brew install cleberalves40/tap/octobrew-tui   # fully qualified name trusts the tap (brew 6+)
brew upgrade octobrew-tui
```

Bottles are not available yet: brew builds from source, with CMake and Ninja as build dependencies (about one minute).

## Submitting to homebrew-core

homebrew-core accepts formulas for projects with demonstrated use. Under its [acceptance rules](https://docs.brew.sh/Acceptable-Formulae), the repository must be notable (stars, forks, watchers) and stable, with published versions. The formula job's `brew audit --new` passes in the local tap since v1.0.0, but notability is checked only in homebrew-core; that is the remaining requirement.

Once the repository is notable:

1. Copy `homebrew/octobrew-tui.rb` to `Formula/o/octobrew-tui.rb` in a fork of [Homebrew/homebrew-core](https://github.com/Homebrew/homebrew-core) (omit `head`, which homebrew-core does not accept for new formulas, if the audit requires it).
2. Run `brew audit --new octobrew-tui`, `brew install --build-from-source octobrew-tui`, and `brew test octobrew-tui`; open a pull request titled `octobrew-tui X.Y.Z (new formula)`.
3. Once the formula is in homebrew-core, the tap becomes a redirect (`tap_migrations.json`) and the formula under `homebrew/` is removed from this repository.

// Amostras da saída do brew 7.0.10 (macOS 27, arm64), reduzidas aos campos
// que o octobrew-tui lê, mais os casos que o brew real nem sempre mostra:
// pin, desatualizado, tap de terceiros e fórmula e cask de mesmo nome.
#ifndef OCTOBREW_TUI_TEST_FIXTURES_H
#define OCTOBREW_TUI_TEST_FIXTURES_H

#include "support.h"

#include <string>
#include <vector>

// "brew info --json=v2 --installed"
const char kInstalledJson[] = R"json({
  "formulae": [
    {
      "name": "curl", "full_name": "curl", "tap": "homebrew/core",
      "desc": "Get a file from an HTTP, HTTPS or FTP server",
      "versions": {"stable": "8.10.0", "head": "HEAD", "bottle": true},
      "revision": 0, "pinned": false, "outdated": true, "linked_keg": null,
      "installed": [
        {"version": "8.9.1", "time": 1760000000, "installed_on_request": true,
         "runtime_dependencies": [
           {"full_name": "brotli", "version": "1.2.0", "declared_directly": true},
           {"full_name": "zstd", "version": "1.5.7", "declared_directly": false}
         ]}
      ]
    },
    {
      "name": "brotli", "full_name": "brotli", "tap": "homebrew/core",
      "desc": "Generic-purpose lossless compression algorithm",
      "versions": {"stable": "1.2.0"}, "revision": 0, "pinned": false, "outdated": false,
      "linked_keg": "1.2.0",
      "installed": [
        {"version": "1.1.0", "time": 1750000000, "installed_on_request": false, "runtime_dependencies": []},
        {"version": "1.2.0", "time": 1760000100, "installed_on_request": false, "runtime_dependencies": []}
      ]
    },
    {
      "name": "wget", "full_name": "wget", "tap": "homebrew/core",
      "desc": "Internet file retriever",
      "versions": {"stable": "1.25.0"}, "revision": 1, "pinned": true, "outdated": true, "linked_keg": "1.24.5",
      "installed": [
        {"version": "1.24.5", "time": 1740000000, "installed_on_request": true,
         "runtime_dependencies": [{"full_name": "brotli", "version": "1.2.0", "declared_directly": true}]}
      ]
    },
    {
      "name": "foo", "full_name": "someone/tap/foo", "tap": "someone/tap",
      "desc": "A formula from a third-party tap",
      "versions": {"stable": "0.3"}, "revision": 0, "pinned": false, "outdated": false, "linked_keg": "0.3",
      "installed": [{"version": "0.3", "time": 1745000000, "installed_on_request": true, "runtime_dependencies": []}]
    }
  ],
  "casks": [
    {
      "token": "alacritty", "full_token": "alacritty", "tap": "homebrew/cask",
      "name": ["Alacritty"], "desc": "GPU-accelerated terminal emulator",
      "version": "0.17.0", "installed": "0.17.0", "installed_time": 1776215584,
      "outdated": false, "pinned": false, "auto_updates": null,
      "depends_on": {"macos": {}},
      "artifacts": [
        {"app": ["Alacritty.app"], "target": "APPDIR/Alacritty.app"},
        {"binary": ["/Applications/Alacritty.app/Contents/MacOS/alacritty"], "target": "/opt/homebrew/bin/alacritty"},
        {"zap": [{"trash": ["~/Library/Saved Application State/org.alacritty.savedState"]}]}
      ]
    },
    {
      "token": "docker", "full_token": "docker", "tap": "homebrew/cask",
      "name": ["Docker Desktop"], "desc": null,
      "version": "4.40.0", "installed": "4.38.0", "installed_time": 1770000000,
      "outdated": true, "pinned": false, "auto_updates": true,
      "depends_on": {"formula": ["brotli"]}, "artifacts": []
    }
  ]
})json";

// O cache da API do brew 5 ou mais novo (api/internal/packages.<tag>.jws.json):
// o JSON vai como string no "payload" do envelope JWS.
const char kApiPayload[] = R"json({"metadata":{"homebrew_version":"7.0.10","bottle_tag":"arm64_golden_gate"},)json"
                           R"json("formulae":{)json"
                           R"json("brotli":{"desc":"Generic-purpose lossless compression algorithm","stable_version":"1.2.0"},)json"
                           R"json("curl":{"desc":"Get a file from an HTTP, HTTPS or FTP server","stable_version":"8.10.0","stable_dependencies":["brotli",{"pkgconf":":build"}]},)json"
                           R"json("docker":{"desc":"Pack, ship and run any application as a lightweight container","stable_version":"28.0.4"},)json"
                           R"json("wget":{"desc":"Internet file retriever","stable_version":"1.25.0","revision":1},)json"
                           R"json("vim":{"desc":"Vi 'workalike' with many additional features","stable_version":"9.1.1000"})json"
                           R"json(},"casks":{)json"
                           R"json("alacritty":{"desc":"GPU-accelerated terminal emulator","names":["Alacritty"],"tap_string":"homebrew/cask","version":"0.16.0"},)json"
                           R"json("docker":{"names":["Docker Desktop"],"tap_string":"homebrew/cask","version":"4.40.0"},)json"
                           R"json("kitty":{"desc":"GPU-based terminal emulator","names":["kitty"],"tap_string":"homebrew/cask","version":"0.49.2"})json"
                           R"json(}})json";

// formula.jws.json do brew 4: um array no formato do "brew info --json=v2".
const char kLegacyFormulae[] = R"json([
  {"name": "curl", "full_name": "curl", "tap": "homebrew/core", "desc": "Get a file",
   "versions": {"stable": "8.10.0"}, "revision": 0, "installed": []},
  {"name": "wget", "full_name": "wget", "tap": "homebrew/core", "desc": "Internet file retriever",
   "versions": {"stable": "1.25.0"}, "revision": 1, "installed": []}
])json";

// cask.jws.json do brew 4.
const char kLegacyCasks[] = R"json([
  {"token": "kitty", "full_token": "kitty", "tap": "homebrew/cask", "name": ["kitty"],
   "desc": "GPU-based terminal emulator", "version": "0.49.2", "installed": null}
])json";

// "brew info --json=v2 --formula curl"
const char kCurlInfo[] = R"json({
  "formulae": [{
    "name": "curl", "full_name": "curl", "tap": "homebrew/core",
    "desc": "Get a file from an HTTP, HTTPS or FTP server",
    "homepage": "https://curl.se", "license": "curl",
    "versions": {"stable": "8.10.0"}, "revision": 0,
    "caveats": "curl is keg-only, which means it was not symlinked into /opt/homebrew,\nbecause macOS already provides this software.\n\nIf you need to have curl first in your PATH, run:\n  echo 'export PATH=\"/opt/homebrew/opt/curl/bin:$PATH\"' >> ~/.zshrc\n",
    "keg_only": true, "keg_only_reason": {"reason": ":provided_by_macos", "explanation": ""},
    "deprecated": false, "deprecation_reason": null, "disabled": false, "disable_reason": null,
    "dependencies": ["brotli", "libnghttp2"], "build_dependencies": ["pkgconf"], "conflicts_with": [],
    "installed": [{"version": "8.9.1", "time": 1760000000, "installed_on_request": true}],
    "linked_keg": null
  }],
  "casks": []
})json";

// "brew info --json=v2 --cask kitty" (não instalado)
const char kKittyInfo[] = R"json({
  "formulae": [],
  "casks": [{
    "token": "kitty", "full_token": "kitty", "tap": "homebrew/cask", "name": ["kitty"],
    "desc": "GPU-based terminal emulator", "homepage": "https://github.com/kovidgoyal/kitty",
    "version": "0.49.2", "installed": null, "installed_time": null, "auto_updates": null, "caveats": null,
    "depends_on": {"macos": {">=": ["12"]}}, "conflicts_with": {"cask": ["kitty@nightly"]},
    "deprecated": true, "deprecation_reason": ":unmaintained",
    "artifacts": [
      {"app": ["kitty.app"], "target": "/Applications/kitty.app"},
      {"command_wrapper": ["kitty", {"executable": "/Applications/kitty.app/Contents/MacOS/kitty"}], "target": "/opt/homebrew/bin/kitty"},
      {"uninstall": [{"quit": "net.kovidgoyal.kitty"}]},
      {"zap": [{"trash": ["~/.config/kitty"]}]}
    ]
  }]
})json";

// Uma string JSON com o texto (para montar o envelope JWS).
inline std::string quoted(const std::string &s)
{
    std::string out = "\"";
    for (char c : s) {
        if (c == '\n') {
            out += "\\n";
            continue;
        }
        if (c == '"' || c == '\\')
            out += '\\';
        out += c;
    }
    return out + "\"";
}

/** O envelope JWS do cache da API: {"payload": "<json>", "signatures": [...]}. */
inline std::string jws(const std::string &payload)
{
    return "{\"payload\":" + quoted(payload) + ",\"signatures\":[{\"signature\":\"x\"}]}";
}

/**
 * brew falso que responde como o brew 7.0.10: os diretórios (--cellar,
 * --caskroom, --cache, todos dentro de dir), --version, o info dos
 * instalados (kInstalledJson), o info de curl e kitty e o list. extra vem
 * antes, para mudar uma resposta.
 */
struct FakeBrew {
    TempDir dir;
    std::string log = dir.file("brew.log");
    std::string path;

    explicit FakeBrew(const std::string &extra = std::string())
    {
        writeFile(dir.file("installed.json"), kInstalledJson);
        writeFile(dir.file("curl.json"), kCurlInfo);
        writeFile(dir.file("kitty.json"), kKittyInfo);
        path = fakeBrew(dir, log,
                        extra + "\n"
                        "case \"$1\" in\n"
                        "--cellar) echo '" + dir.file("Cellar") + "' ;;\n"
                        "--caskroom) echo '" + dir.file("Caskroom") + "' ;;\n"
                        "--cache) echo '" + dir.file("cache") + "' ;;\n"
                        "--version) echo 'Homebrew 7.0.10'; echo 'Homebrew/homebrew-core (git revision x)' ;;\n"
                        "info)\n"
                        "  case \"$3\" in\n"
                        "  --installed) cat '" + dir.file("installed.json") + "' ;;\n"
                        "  *) f='" + dir.path + "/'\"$4\".json\n"
                        "     [ -f \"$f\" ] && cat \"$f\" && exit 0\n"
                        "     echo \"Error: No available formula with the name \\\"$4\\\".\" >&2; exit 1 ;;\n"
                        "  esac ;;\n"
                        "list) echo /opt/homebrew/Cellar/curl/8.9.1/bin/curl; echo /opt/homebrew/Cellar/curl/8.9.1/lib/libcurl.4.dylib ;;\n"
                        "esac\n");
    }

    /** O cache da API do brew 7, com kApiPayload. */
    void writeApiCache() const
    {
        const std::string internal = dir.file("cache/api/internal");
        (void)std::system(("mkdir -p '" + internal + "'").c_str());
        writeFile(internal + "/packages.arm64_golden_gate.jws.json", jws(kApiPayload));
    }

    std::vector<std::string> calls() const { return readLines(log); }
};

#endif

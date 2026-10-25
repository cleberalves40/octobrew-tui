// SPDX-License-Identifier: GPL-2.0-or-later
//
// Integração com o Homebrew: a contraparte do pkg.h do octopkg-tui (o porte
// das classes Package e UnixCommand do OctoPkg), lendo o JSON do brew em vez
// da saída do pkg(8).
// OctoPkg: Copyright (C) 2015 Alexandre Albuquerque Arnt.

#ifndef OCTOBREW_TUI_BREW_H
#define OCTOBREW_TUI_BREW_H

#include <atomic>
#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace octo {

/** Fórmula (Cellar) ou cask (Caskroom: aplicativos, fontes, pacotes .pkg). */
enum class Kind { Formula = 0, Cask };

/**
 * A ordem do PackageStatus do OctoPkg, que é a ordem da coluna de estado;
 * o bloqueio do pkg é o pin do brew.
 */
enum class Status { Pinned = 0, Installed, NotInstalled, Outdated, Newer };

struct Package {
    std::string name;               // fórmula ou token do cask ("curl", "alacritty")
    std::string fullName;           // com o tap, fora dos oficiais ("user/tap/foo"); vai aos comandos
    Kind kind = Kind::Formula;
    std::string version;            // disponível, na visão Repositório; instalada, na visão Instalados
    std::string tap;                // "homebrew/core", "homebrew/cask", "user/tap"
    std::string comment;            // desc
    std::int64_t size = 0;          // tamanho em disco, calculado depois da listagem; 0: desconhecido
    std::int64_t installedOn = 0;   // UNIX; 0 se não instalado
    bool installed = false;
    bool pinned = false;
    bool automatic = false;         // instalado como dependência (não installed_on_request)
    int requiredBy = 0;             // quantos instalados dependem dele diretamente
    std::string installedVersion;
    std::string newVersion;         // versão disponível, se desatualizado
    std::vector<std::string> dependencies;   // diretas, de execução (só instalados)
    std::vector<std::string> paths;          // casks instalados: onde os artifacts foram parar
    Status status = Status::NotInstalled;

    /**
     * Identidade na lista: fórmula e cask podem ter o mesmo nome ("docker"),
     * e taps diferentes, a mesma fórmula. "f:curl", "c:alacritty".
     */
    std::string key() const;
};

/** Aba Informação e aba Arquivos. */
struct Details {
    std::string name;
    std::string fullName;
    Kind kind = Kind::Formula;
    std::string version;
    std::string tap;
    std::string homepage;
    std::string license;
    std::string description;
    std::string caveats;
    std::string kegOnly;                     // motivo; vazio se não for keg-only
    std::string deprecated;                  // motivo, se obsoleto
    std::string disabled;                    // motivo, se desativado
    bool autoUpdates = false;                // cask que se atualiza sozinho
    std::vector<std::string> names;          // casks: nomes do aplicativo
    std::vector<std::string> dependencies;
    std::vector<std::string> buildDependencies;
    std::vector<std::string> requiredBy;     // instalados que dependem dele (a App preenche)
    std::vector<std::string> conflicts;
    std::vector<std::string> artifacts;      // casks: "App: Alacritty.app"
    std::vector<std::string> files;          // só instalados
    bool filesTruncated = false;
    bool installed = false;
    std::int64_t installedOn = 0;
};

// --- Leitura do JSON (funções puras, testadas sem o brew) ---

/** "brew info --json=v2 --installed": fórmulas e casks instalados, com o estado. */
std::vector<Package> parseInstalled(const std::string &json);
/**
 * O cache da API do Homebrew: o "packages.<tag>.jws.json" do brew 5 ou mais
 * novo (um objeto com "formulae" e "casks") ou o "formula.jws.json" e o
 * "cask.jws.json" do brew 4 (um array). Com ou sem o envelope JWS ("payload").
 */
std::vector<Package> parseApi(const std::string &json);
/** "brew info --json=v2 --formula|--cask nome": o primeiro registro. */
Details parseDetails(const std::string &json);

/**
 * Visão Repositório: os pacotes da API, com o estado dos instalados, mais os
 * instalados que a API não tem (taps de terceiros).
 */
std::vector<Package> mergeRemote(const std::vector<Package> &remote, const std::vector<Package> &installed);
/** Instalados que dependem diretamente de fullName (o "Requerido por"). */
std::vector<std::string> dependents(const std::vector<Package> &installed, const std::string &fullName);

/** Versão com a revisão do brew ("3.4" e 1 → "3.4_1"). */
std::string formulaVersion(const std::string &stable, int revision);
/**
 * Compara versões por partes numéricas e de texto ("1.10" > "1.9"), como o
 * rpmvercmp do OctoPkg. -1, 0 ou 1.
 */
int compareVersions(const std::string &a, const std::string &b);

/** Tamanho legível, como o Package::kbytesToSize do OctoPkg ("12.34 MiB"). */
std::string formatSize(std::int64_t bytes);
/** Data e hora local "AAAA-MM-DD HH:MM"; vazio para 0. */
std::string formatTime(std::int64_t unixTime);

/**
 * Nome aceito como argumento do brew: não vazio, sem "-" inicial (seria lido
 * como opção) e só com letras, dígitos e "@+-._/".
 */
bool validPackageName(const std::string &name);

/**
 * Bytes em disco da árvore em path, sem seguir links simbólicos (o tamanho
 * aparente dos arquivos, como o flatsize do pkg). cancel, se dado, interrompe.
 */
std::int64_t treeSize(const std::string &path, const std::atomic<bool> *cancel = nullptr);

class Manager
{
public:
    /** brewPath: executável no lugar do brew; vazio: o do Homebrew instalado ou o do PATH. */
    explicit Manager(std::string brewPath = std::string());

    std::vector<Package> listInstalled() const;
    /** Do cache da API, sem rede e sem o Ruby do brew. */
    std::vector<Package> listRemote() const;
    /** brew info, mais brew list nos instalados (a aba Arquivos). */
    Details details(const Package &p) const;
    /** Tamanho de cada instalado, por Package::key(); os que não deram para medir ficam de fora. */
    std::map<std::string, std::int64_t> sizes(const std::vector<Package> &installed,
                                              const std::atomic<bool> *cancel = nullptr) const;
    /** "Homebrew 7.0.10" */
    std::string version() const;

    struct Paths {
        std::string cellar;     // brew --cellar
        std::string caskroom;   // brew --caskroom
        std::string cache;      // brew --cache
    };
    /** Os diretórios do Homebrew, perguntados ao brew uma vez. */
    Paths paths() const;

    const std::string &brewPath() const { return m_brewPath; }

    /** Limite da aba Arquivos, como no OctoPkg (2 MiB de saída). */
    static constexpr std::size_t kMaxFilesOutput = 2097152;

private:
    std::string query(const std::vector<std::string> &args) const;

    std::string m_brewPath;
    mutable std::mutex m_pathsMutex;
    mutable bool m_hasPaths = false;
    mutable Paths m_paths;
};

} // namespace octo

#endif

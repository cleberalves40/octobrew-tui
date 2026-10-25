// Núcleo: o leitor de JSON, a leitura da saída do brew e do cache da API,
// chamadas ao brew com um brew falso, os tamanhos em disco, a execução de
// processos e as preferências. Roda em qualquer sistema, sem o Homebrew.

#include "brew.h"
#include "fixtures.h"
#include "i18n.h"
#include "json.h"
#include "process.h"
#include "settings.h"
#include "support.h"

#include <atomic>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>   // CHECK_THROWS_WITH
#include <catch2/matchers/catch_matchers_string.hpp>
#include <cstdlib>
#include <stdexcept>
#include <sys/stat.h>
#include <sys/time.h>

using octo::Kind;
using octo::Status;
using Catch::Matchers::ContainsSubstring;

namespace {

const octo::Package &find(const std::vector<octo::Package> &list, const std::string &key)
{
    for (const auto &p : list)
        if (p.key() == key)
            return p;
    throw std::runtime_error("pacote não encontrado: " + key);
}

bool has(const std::vector<octo::Package> &list, const std::string &key)
{
    for (const auto &p : list)
        if (p.key() == key)
            return true;
    return false;
}

bool contains(const std::string &s, const std::string &part)
{
    return s.find(part) != std::string::npos;
}

void makeDirs(const std::string &path)
{
    (void)std::system(("mkdir -p '" + path + "'").c_str());
}

void writeBytes(const std::string &path, size_t n)
{
    writeFile(path, std::string(n, 'x'));
}

} // namespace

// ---------------------------------------------------------------------------
// JSON
// ---------------------------------------------------------------------------

TEST_CASE("lê JSON: tipos, objetos e arrays", "[json]")
{
    const json::Value v = json::parse(R"({"s": "a\"b\\c\/d\n", "n": -12.5e1, "i": 1792932487,
        "t": true, "f": false, "z": null, "a": [1, "dois", {"tres": 3}], "o": {}, "e": []})");
    REQUIRE(v.isObject());
    CHECK(v["s"].toString() == "a\"b\\c/d\n");
    CHECK(v["n"].toNumber() == -125.0);
    CHECK(v["i"].toInt() == 1792932487);
    CHECK(v["t"].toBool());
    CHECK_FALSE(v["f"].toBool(true));
    CHECK(v["z"].isNull());
    CHECK(v["a"].size() == 3);
    CHECK(v["a"][1].toString() == "dois");
    CHECK(v["a"][2]["tres"].toInt() == 3);
    CHECK(v["a"].strings() == std::vector<std::string>{"dois"});
    CHECK(v["o"].isObject());
    CHECK(v["e"].isArray());
    // Ausentes e tipos trocados dão o fallback, sem exceção.
    CHECK(v["nao"].isNull());
    CHECK(v["nao"]["mais"][3].toString("x") == "x");
    CHECK(v["s"].toInt(7) == 7);
    CHECK(v["a"][9].isNull());
    CHECK(v.contains("z"));
    CHECK_FALSE(v.contains("nao"));
    // A ordem dos membros é a do texto.
    CHECK(v.members().front().first == "s");
}

TEST_CASE("JSON: escapes \\u, com pares substitutos", "[json]")
{
    CHECK(json::parse(R"("Café — 🍺")").toString() == "Café — 🍺");
    CHECK(json::parse(R"("ok \udc00")").toString() == "ok \xEF\xBF\xBD");   // substituto solto
    CHECK(json::parse("\"já em UTF-8: ç\"").toString() == "já em UTF-8: ç");
}

TEST_CASE("JSON inválido lança erro com a posição", "[json]")
{
    CHECK_THROWS_WITH(json::parse("{\"a\": }"), ContainsSubstring("offset 6"));
    CHECK_THROWS(json::parse(""));
    CHECK_THROWS(json::parse("[1, 2"));
    CHECK_THROWS(json::parse("{\"a\" 1}"));
    CHECK_THROWS(json::parse("\"sem fim"));
    CHECK_THROWS(json::parse("\"\x01\""));
    CHECK_THROWS(json::parse("01x"));
    CHECK_THROWS(json::parse("[1] lixo"));
    CHECK_THROWS(json::parse("tru"));
    CHECK_THROWS(json::parse(std::string(1000, '[')));   // aninhamento demais
}

// ---------------------------------------------------------------------------
// Leitura da saída do brew
// ---------------------------------------------------------------------------

TEST_CASE("lê os instalados do brew info --json=v2 --installed", "[parse]")
{
    const auto list = octo::parseInstalled(kInstalledJson);
    REQUIRE(list.size() == 6);

    const auto &curl = find(list, "f:curl");
    CHECK(curl.kind == Kind::Formula);
    CHECK(curl.tap == "homebrew/core");
    CHECK(curl.version == "8.9.1");   // na visão Instalados, a versão instalada
    CHECK(curl.installedVersion == "8.9.1");
    CHECK(curl.newVersion == "8.10.0");
    CHECK(curl.status == Status::Outdated);
    CHECK(curl.installedOn == 1760000000);
    CHECK_FALSE(curl.automatic);
    CHECK(curl.comment == "Get a file from an HTTP, HTTPS or FTP server");
    // Só as dependências declaradas diretamente.
    CHECK(curl.dependencies == std::vector<std::string>{"brotli"});

    // Duas versões no Cellar: vale a ligada (linked_keg).
    const auto &brotli = find(list, "f:brotli");
    CHECK(brotli.installedVersion == "1.2.0");
    CHECK(brotli.installedOn == 1760000100);
    CHECK(brotli.automatic);   // installed_on_request: false
    CHECK(brotli.status == Status::Installed);
    CHECK(brotli.requiredBy == 3);   // curl, wget e o cask docker

    // O pin prevalece sobre a atualização, como o bloqueio no OctoPkg; a
    // versão nova continua conhecida.
    const auto &wget = find(list, "f:wget");
    CHECK(wget.status == Status::Pinned);
    CHECK(wget.pinned);
    CHECK(wget.newVersion == "1.25.0_1");

    // Tap de terceiros: o nome completo vai aos comandos.
    const auto &foo = find(list, "f:someone/tap/foo");
    CHECK(foo.name == "foo");
    CHECK(foo.fullName == "someone/tap/foo");
    CHECK(foo.tap == "someone/tap");

    const auto &alacritty = find(list, "c:alacritty");
    CHECK(alacritty.kind == Kind::Cask);
    CHECK(alacritty.version == "0.17.0");
    CHECK(alacritty.status == Status::Installed);
    CHECK(alacritty.installedOn == 1776215584);
    CHECK(alacritty.paths == std::vector<std::string>{"APPDIR/Alacritty.app", "/opt/homebrew/bin/alacritty"});

    // Sem desc, o nome do aplicativo serve de descrição.
    const auto &docker = find(list, "c:docker");
    CHECK(docker.comment == "Docker Desktop");
    CHECK(docker.status == Status::Outdated);
    CHECK(docker.newVersion == "4.40.0");
    CHECK(docker.dependencies == std::vector<std::string>{"brotli"});
}

TEST_CASE("instalados sem nenhum pacote e JSON inválido", "[parse]")
{
    CHECK(octo::parseInstalled(R"({"formulae": [], "casks": []})").empty());
    CHECK_THROWS(octo::parseInstalled("Error: something"));
}

TEST_CASE("lê o cache da API do brew 5+, com o envelope JWS", "[parse][api]")
{
    const auto list = octo::parseApi(jws(kApiPayload));
    REQUIRE(list.size() == 8);
    const auto &wget = find(list, "f:wget");
    CHECK(wget.version == "1.25.0_1");   // a revisão entra na versão, como no Cellar
    CHECK(wget.tap == "homebrew/core");
    CHECK(wget.status == Status::NotInstalled);
    CHECK_FALSE(wget.installed);
    const auto &docker = find(list, "c:docker");
    CHECK(docker.kind == Kind::Cask);
    CHECK(docker.comment == "Docker Desktop");
    CHECK(docker.tap == "homebrew/cask");
    // Fórmula e cask de mesmo nome são pacotes diferentes.
    CHECK(has(list, "f:docker"));
    // Sem o envelope também serve.
    CHECK(octo::parseApi(kApiPayload).size() == 8);
}

TEST_CASE("lê o cache da API do brew 4 (arrays)", "[parse][api]")
{
    const auto formulae = octo::parseApi(jws(kLegacyFormulae));
    REQUIRE(formulae.size() == 2);
    CHECK(find(formulae, "f:wget").version == "1.25.0_1");
    const auto casks = octo::parseApi(jws(kLegacyCasks));
    REQUIRE(casks.size() == 1);
    CHECK(casks[0].kind == Kind::Cask);
    CHECK(casks[0].version == "0.49.2");
    CHECK_FALSE(casks[0].installed);
}

TEST_CASE("visão Repositório junta o estado dos instalados", "[status]")
{
    const auto installed = octo::parseInstalled(kInstalledJson);
    const auto merged = octo::mergeRemote(octo::parseApi(jws(kApiPayload)), installed);
    // Os 8 da API, mais o foo, que só o tap de terceiros tem.
    REQUIRE(merged.size() == 9);
    CHECK(find(merged, "f:vim").status == Status::NotInstalled);
    CHECK(find(merged, "f:curl").status == Status::Outdated);
    CHECK(find(merged, "f:curl").version == "8.10.0");   // a disponível
    CHECK(find(merged, "f:curl").installedVersion == "8.9.1");
    CHECK(find(merged, "f:wget").status == Status::Pinned);
    CHECK(find(merged, "f:brotli").status == Status::Installed);
    CHECK(find(merged, "f:brotli").automatic);
    // O cache da API está atrás do que foi instalado.
    CHECK(find(merged, "c:alacritty").status == Status::Newer);
    CHECK(find(merged, "c:docker").status == Status::Outdated);
    CHECK_FALSE(find(merged, "f:docker").installed);   // instalado é o cask, não a fórmula
    CHECK(find(merged, "f:someone/tap/foo").status == Status::Installed);
}

TEST_CASE("dependentes diretos, para o Requerido por", "[status]")
{
    const auto installed = octo::parseInstalled(kInstalledJson);
    CHECK(octo::dependents(installed, "brotli") == std::vector<std::string>{"curl", "wget", "docker (cask)"});
    CHECK(octo::dependents(installed, "zstd").empty());   // só indireta
}

TEST_CASE("lê os detalhes de uma fórmula", "[parse][details]")
{
    const octo::Details d = octo::parseDetails(kCurlInfo);
    CHECK(d.kind == Kind::Formula);
    CHECK(d.name == "curl");
    CHECK(d.version == "8.10.0");
    CHECK(d.homepage == "https://curl.se");
    CHECK(d.license == "curl");
    CHECK(d.kegOnly == "provided by macos");
    CHECK(d.dependencies == std::vector<std::string>{"brotli", "libnghttp2"});
    CHECK(d.buildDependencies == std::vector<std::string>{"pkgconf"});
    CHECK(d.installed);
    CHECK(d.installedOn == 1760000000);
    CHECK(contains(d.caveats, "curl is keg-only"));
    CHECK(d.caveats.back() != '\n');
    CHECK(d.deprecated.empty());
}

TEST_CASE("lê os detalhes de um cask, com os artifacts", "[parse][details]")
{
    const octo::Details d = octo::parseDetails(kKittyInfo);
    CHECK(d.kind == Kind::Cask);
    CHECK(d.name == "kitty");
    CHECK(d.names == std::vector<std::string>{"kitty"});
    CHECK(d.version == "0.49.2");
    CHECK_FALSE(d.installed);
    CHECK(d.caveats.empty());
    CHECK(d.conflicts == std::vector<std::string>{"kitty@nightly"});
    CHECK(d.deprecated == "unmaintained");
    // O que o cask instala; desinstalar e apagar os dados (zap) ficam de fora.
    CHECK(d.artifacts == std::vector<std::string>{"App: kitty.app", "Command wrapper: kitty"});
}

TEST_CASE("versões, nomes e tamanhos", "[format]")
{
    CHECK(octo::formulaVersion("3.4", 0) == "3.4");
    CHECK(octo::formulaVersion("3.4", 2) == "3.4_2");
    CHECK(octo::formulaVersion("", 2).empty());
    CHECK(octo::compareVersions("1.10", "1.9") > 0);
    CHECK(octo::compareVersions("3.4_1", "3.4") > 0);
    CHECK(octo::compareVersions("0.17.0", "0.17.0") == 0);
    CHECK(octo::compareVersions("8.9.1", "8.10.0") < 0);

    CHECK(octo::validPackageName("openssl@3"));
    CHECK(octo::validPackageName("someone/tap/foo"));
    CHECK(octo::validPackageName("font-jetbrains-mono-nerd-font"));
    CHECK(octo::validPackageName("c++utilities"));
    CHECK_FALSE(octo::validPackageName(""));
    CHECK_FALSE(octo::validPackageName("-v"));
    CHECK_FALSE(octo::validPackageName("a b"));
    CHECK_FALSE(octo::validPackageName("x;rm"));

    CHECK(octo::formatSize(512) == "512.00 Bytes");
    CHECK(octo::formatSize(4572000) == "4.36 MiB");
    CHECK(octo::formatSize(1073741824LL * 3) == "3.00 GiB");
    CHECK(octo::formatTime(0).empty());
}

// ---------------------------------------------------------------------------
// O brew, com um brew falso
// ---------------------------------------------------------------------------

namespace {

octo::Package package(const std::string &name, Kind kind, bool installed)
{
    octo::Package p;
    p.name = p.fullName = name;
    p.kind = kind;
    p.installed = installed;
    return p;
}

} // namespace

TEST_CASE("lista os instalados com o brew, sem atualizar nada", "[brew]")
{
    FakeBrew brew;
    octo::Manager m(brew.path);
    const auto list = m.listInstalled();
    CHECK(list.size() == 6);
    REQUIRE(brew.calls() == std::vector<std::string>{"info|--json=v2|--installed"});
    // Sem auto-update nem download da API, sem cores, em UTF-8.
    const auto env = readLines(brew.log + ".env");
    REQUIRE(env.size() == 1);
    CHECK(env[0].rfind("1|1|", 0) == 0);
    CHECK(contains(env[0], "UTF-8"));
}

TEST_CASE("detalhes: brew info e, nos instalados, brew list", "[brew][details]")
{
    FakeBrew brew;
    octo::Manager m(brew.path);

    const octo::Details curl = m.details(package("curl", Kind::Formula, true));
    CHECK(curl.version == "8.10.0");
    CHECK(curl.files == std::vector<std::string>{"/opt/homebrew/Cellar/curl/8.9.1/bin/curl",
                                                 "/opt/homebrew/Cellar/curl/8.9.1/lib/libcurl.4.dylib"});
    // Não instalado: sem brew list.
    const octo::Details kitty = m.details(package("kitty", Kind::Cask, false));
    CHECK(kitty.files.empty());
    CHECK(brew.calls() == std::vector<std::string>{"info|--json=v2|--formula|curl",
                                                   "list|--formula|--verbose|curl",
                                                   "info|--json=v2|--cask|kitty"});
}

TEST_CASE("detalhes: nome inválido não chega ao brew, e o erro dele é repassado", "[brew][details]")
{
    FakeBrew brew;
    octo::Manager m(brew.path);
    CHECK_THROWS_WITH(m.details(package("--force", Kind::Formula, false)), ContainsSubstring("Invalid package name"));
    CHECK(brew.calls().empty());
    // O "Error: " do brew sai da mensagem.
    CHECK_THROWS_WITH(m.details(package("naoexiste", Kind::Formula, false)),
                      "No available formula with the name \"naoexiste\".");
}

TEST_CASE("erros do brew são repassados", "[brew]")
{
    SECTION("brew que falha")
    {
        TempDir dir;
        const std::string path = fakeBrew(dir, dir.file("log"), "echo 'Warning: x' >&2; echo 'Error: Running Homebrew as root is extremely dangerous' >&2; exit 1");
        CHECK_THROWS_WITH(octo::Manager(path).listInstalled(),
                          ContainsSubstring("Running Homebrew as root is extremely dangerous"));
    }
    SECTION("saída que não é JSON")
    {
        TempDir dir;
        const std::string path = fakeBrew(dir, dir.file("log"), "echo '==> Formulae'");
        CHECK_THROWS_WITH(octo::Manager(path).listInstalled(), ContainsSubstring("Could not read the output of brew info"));
    }
    SECTION("brew inexistente")
    {
        CHECK_THROWS_WITH(octo::Manager("/nao/existe/brew").listInstalled(), ContainsSubstring("Failed to start"));
    }
    SECTION("avisos no stderr não atrapalham")
    {
        FakeBrew brew("echo 'Warning: Treating curl as a formula.' >&2");
        CHECK(octo::Manager(brew.path).listInstalled().size() == 6);
    }
}

TEST_CASE("repositório: lê o cache da API do brew, sem o Ruby", "[brew][api]")
{
    FakeBrew brew;
    octo::Manager m(brew.path);

    SECTION("brew 5+: o packages.*.jws.json mais recente")
    {
        makeDirs(brew.dir.file("cache/api/internal"));
        writeFile(brew.dir.file("cache/api/internal/packages.x86_64_sonoma.jws.json"), jws(R"({"formulae":{},"casks":{}})"));
        ::utimes(brew.dir.file("cache/api/internal/packages.x86_64_sonoma.jws.json").c_str(), nullptr);
        const std::string current = brew.dir.file("cache/api/internal/packages.arm64_golden_gate.jws.json");
        writeFile(current, jws(kApiPayload));
        // O antigo fica com uma data de 2020.
        const struct timeval old[2] = {{1600000000, 0}, {1600000000, 0}};
        ::utimes(brew.dir.file("cache/api/internal/packages.x86_64_sonoma.jws.json").c_str(), old);
        // Arquivos auxiliares do brew no mesmo diretório são ignorados.
        writeFile(current + ".etag", "W/\"x\"");
        writeFile(current + ".payload", "não é JSON");
        CHECK(m.listRemote().size() == 8);
        // Só os diretórios, perguntados ao brew; o cache é lido direto.
        CHECK(brew.calls() == std::vector<std::string>{"--cellar", "--caskroom", "--cache"});
        // Uma segunda leitura não pergunta de novo.
        m.listRemote();
        CHECK(brew.calls().size() == 3);
    }
    SECTION("brew 4: formula.jws.json e cask.jws.json")
    {
        makeDirs(brew.dir.file("cache/api"));
        writeFile(brew.dir.file("cache/api/formula.jws.json"), jws(kLegacyFormulae));
        writeFile(brew.dir.file("cache/api/cask.jws.json"), jws(kLegacyCasks));
        const auto list = m.listRemote();
        CHECK(list.size() == 3);
        CHECK(has(list, "c:kitty"));
    }
    SECTION("sem cache: pede o brew update")
    {
        CHECK_THROWS_WITH(m.listRemote(), ContainsSubstring("brew update"));
    }
    SECTION("cache corrompido")
    {
        makeDirs(brew.dir.file("cache/api/internal"));
        writeFile(brew.dir.file("cache/api/internal/packages.arm64_golden_gate.jws.json"), "{\"payload\": \"{\"}");
        CHECK_THROWS_WITH(m.listRemote(), ContainsSubstring("Could not read"));
    }
}

TEST_CASE("tamanho em disco: o rack no Cellar e o aplicativo do cask", "[brew][size]")
{
    FakeBrew brew;
    makeDirs(brew.dir.file("Cellar/curl/8.9.1/bin"));
    makeDirs(brew.dir.file("Cellar/curl/8.10.0"));
    writeBytes(brew.dir.file("Cellar/curl/8.9.1/bin/curl"), 3000);
    writeBytes(brew.dir.file("Cellar/curl/8.10.0/INSTALL_RECEIPT.json"), 1000);
    // Links simbólicos não são seguidos nem somados.
    (void)::symlink("/usr", brew.dir.file("Cellar/curl/8.9.1/usr").c_str());
    makeDirs(brew.dir.file("Caskroom/alacritty/0.17.0"));
    writeBytes(brew.dir.file("Caskroom/alacritty/0.17.0/x"), 10);
    makeDirs(brew.dir.file("Applications/Alacritty.app/Contents"));
    writeBytes(brew.dir.file("Applications/Alacritty.app/Contents/alacritty"), 5000);

    std::string text = kInstalledJson;
    text.replace(text.find("APPDIR"), 6, brew.dir.file("Applications"));
    const auto installed = octo::parseInstalled(text);

    octo::Manager m(brew.path);
    const auto sizes = m.sizes(installed);
    CHECK(sizes.at("f:curl") == 4000);   // as duas versões
    CHECK(sizes.at("c:alacritty") == 5010);
    CHECK_FALSE(sizes.count("f:wget"));  // sem nada no disco
    CHECK(octo::treeSize(brew.dir.file("Cellar/curl/8.9.1/bin/curl")) == 3000);
    CHECK(octo::treeSize(brew.dir.file("nao-existe")) == 0);

    std::atomic<bool> cancel{true};
    CHECK(m.sizes(installed, &cancel).empty());
}

TEST_CASE("versão do brew", "[brew]")
{
    FakeBrew brew;
    CHECK(octo::Manager(brew.path).version() == "Homebrew 7.0.10");
}

// ---------------------------------------------------------------------------
// Processos, preferências e i18n (os mesmos do octopkg-tui)
// ---------------------------------------------------------------------------

TEST_CASE("mensagens seguem o idioma", "[i18n]")
{
    i18n::setLocale("pt_BR");
    CHECK(i18n::trf("Invalid package name: %1", {"-a"}) == "Nome de pacote inválido: -a");
    CHECK(i18n::tr("Pinned") == "Fixado");
    i18n::setLocale("es_ES");
    CHECK(i18n::tr("Outdated") == "Desactualizado");
    i18n::setLocale("xx_YY");
    CHECK(i18n::locale() == "en_US");
    CHECK(i18n::tr("Outdated") == "Outdated");
}

TEST_CASE("trf substitui os argumentos uma vez", "[i18n]")
{
    i18n::setLocale("en_US");
    CHECK(i18n::trf("Failed to start '%1': %2", {"%2", "x"}) == "Failed to start '%2': x");
    i18n::setLocale("pt_BR");
    CHECK(i18n::trf("%1 package(s) in the repository.", {"3"}) == "3 pacote(s) no repositório.");
    i18n::setLocale("en_US");
}

TEST_CASE("execução de processos", "[process]")
{
    SECTION("captura stdout, stderr e status")
    {
        const auto r = octo::runProcess({"/bin/sh", "-c", "echo out; echo err >&2; exit 4"}, 5000);
        CHECK(r.started);
        CHECK(r.out == "out\n");
        CHECK(r.err == "err\n");
        CHECK(r.status == 4);
    }
    SECTION("ambiente do filho")
    {
        ::setenv("OCTOBREW_TUI_TEST_VAR", "pai", 1);
        const auto r = octo::runProcess({"/bin/sh", "-c", "echo \"$OCTOBREW_TUI_TEST_VAR $LC_ALL\""}, 5000,
                                        {{"OCTOBREW_TUI_TEST_VAR", "filho"}, {"LC_ALL", "C"}});
        CHECK(r.out == "filho C\n");
        CHECK(std::string(std::getenv("OCTOBREW_TUI_TEST_VAR")) == "pai");   // o pai não muda
        ::unsetenv("OCTOBREW_TUI_TEST_VAR");
    }
    SECTION("entrada é /dev/null")
    {
        const auto r = octo::runProcess({"/bin/sh", "-c", "cat; echo fim"}, 5000);
        CHECK(r.out == "fim\n");
    }
    SECTION("executável inexistente")
    {
        const auto r = octo::runProcess({"/nao/existe/brew"}, 5000);
        CHECK_FALSE(r.started);
        CHECK_FALSE(r.startError.empty());
    }
    SECTION("tempo limite")
    {
        const auto r = octo::runProcess({"/bin/sh", "-c", "sleep 5"}, 200);
        CHECK(r.timedOut);
    }
    SECTION("término por sinal")
    {
        const auto r = octo::runProcess({"/bin/sh", "-c", "kill -9 $$"}, 5000);
        CHECK(r.crashed);
    }
}

TEST_CASE("a saída chega aos poucos", "[process]")
{
    std::vector<std::string> chunks;
    const auto r = octo::runProcess({"/bin/sh", "-c", "echo um; sleep 0.3; echo dois >&2"}, 5000, {},
                                    [&chunks](const std::string &c) { chunks.push_back(c); });
    REQUIRE(chunks.size() >= 2);
    CHECK(chunks.front() == "um\n");
    CHECK(r.out == "um\n");
    CHECK(r.err == "dois\n");
}

TEST_CASE("preferências gravam e releem", "[settings]")
{
    TempDir dir;
    const std::string path = dir.file("sub/dir/config");
    {
        octo::Settings s(path);
        CHECK(s.value("x", "padrão") == "padrão");
        s.setValue("sort.column", 4);
        s.setValue("icons", "ascii");
        REQUIRE(s.save());
    }
    octo::Settings s(path);
    CHECK(s.intValue("sort.column", -1) == 4);
    CHECK(s.value("icons") == "ascii");
    CHECK(s.intValue("icons", 7) == 7);
}

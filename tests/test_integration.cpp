// Integração contra o Homebrew real, só lendo: as mesmas consultas que a
// interface faz, no Cellar, no Caskroom e no cache da API do sistema. Nada é
// instalado nem removido (a Fase 1 é somente leitura).
//
// OCTOBREW_TEST_BREW aponta o brew; sem a variável, os testes são ignorados.
// tests/macos-e2e.sh a define.

#include "brew.h"

#include <catch2/catch_test_macros.hpp>
#include <cstdlib>
#include <stdexcept>

namespace {

bool enabled()
{
    const char *cmd = std::getenv("OCTOBREW_TEST_BREW");
    return cmd && *cmd;
}

const octo::Manager &mgr()
{
    static octo::Manager m(std::getenv("OCTOBREW_TEST_BREW") ? std::getenv("OCTOBREW_TEST_BREW") : "");
    return m;
}

const std::vector<octo::Package> &installed()
{
    static const std::vector<octo::Package> list = mgr().listInstalled();
    return list;
}

#define REQUIRE_BREW()                                                                                  \
    if (!enabled())                                                                                     \
    SKIP("OCTOBREW_TEST_BREW não definido; execute via tests/macos-e2e.sh")

} // namespace

TEST_CASE("lista os instalados, com o estado de cada um", "[integration]")
{
    REQUIRE_BREW();
    const auto &list = installed();
    // Os testes compilam com o ftxui e o catch2 do Homebrew.
    REQUIRE_FALSE(list.empty());
    bool ftxui = false;
    for (const auto &p : list) {
        CHECK_FALSE(p.name.empty());
        CHECK(p.installed);
        CHECK_FALSE(p.installedVersion.empty());
        CHECK(octo::validPackageName(p.fullName));
        CHECK(p.status != octo::Status::NotInstalled);
        if (p.key() == "f:ftxui") {
            ftxui = true;
            CHECK(p.tap == "homebrew/core");
            CHECK(p.installedOn > 0);
        }
    }
    CHECK(ftxui);
}

TEST_CASE("detalhes e arquivos de um instalado", "[integration]")
{
    REQUIRE_BREW();
    for (const auto &p : installed()) {
        if (p.key() != "f:ftxui")
            continue;
        const octo::Details d = mgr().details(p);
        CHECK(d.name == "ftxui");
        CHECK_FALSE(d.homepage.empty());
        CHECK_FALSE(d.files.empty());
        bool header = false;
        for (const auto &f : d.files)
            header = header || f.find("/include/ftxui/") != std::string::npos;
        CHECK(header);
        return;
    }
    FAIL("ftxui não está instalado");
}

TEST_CASE("lê o cache da API e junta com os instalados", "[integration]")
{
    REQUIRE_BREW();
    const auto remote = mgr().listRemote();
    // O homebrew-core tem milhares de fórmulas, e o homebrew-cask, milhares de casks.
    CHECK(remote.size() > 5000);
    bool cask = false;
    for (const auto &p : remote)
        cask = cask || p.kind == octo::Kind::Cask;
    CHECK(cask);
    const auto merged = octo::mergeRemote(remote, installed());
    int installedCount = 0;
    for (const auto &p : merged)
        installedCount += p.installed ? 1 : 0;
    CHECK(installedCount == static_cast<int>(installed().size()));
}

TEST_CASE("mede o espaço em disco dos instalados", "[integration]")
{
    REQUIRE_BREW();
    const auto sizes = mgr().sizes(installed());
    CHECK(sizes.count("f:ftxui"));
    CHECK(sizes.at("f:ftxui") > 100 * 1024);
}

TEST_CASE("versão do Homebrew", "[integration]")
{
    REQUIRE_BREW();
    CHECK(mgr().version().rfind("Homebrew ", 0) == 0);
}

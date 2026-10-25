// Tela: renderiza a interface em texto e confere o que aparece, e dirige a
// App com eventos de teclado, sem terminal.

#include "app.h"
#include "fixtures.h"
#include "i18n.h"
#include "support.h"
#include "view.h"

#include <catch2/catch_test_macros.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/screen/terminal.hpp>
#include <memory>
#include <set>

using namespace ftxui;
using octo::Kind;
using octo::Status;

namespace {

octo::Package installed(const std::string &name, const std::string &version, std::int64_t size,
                        Kind kind = Kind::Formula)
{
    octo::Package p;
    p.name = p.fullName = name;
    p.kind = kind;
    p.tap = kind == Kind::Cask ? "homebrew/cask" : "homebrew/core";
    p.version = version;
    p.installedVersion = version;
    p.size = size;
    p.installedOn = 1760000000;
    p.installed = true;
    p.comment = name + " comment";
    p.status = Status::Installed;
    return p;
}

// Na ordem do brew: fórmulas e depois casks.
std::vector<octo::Package> sample()
{
    octo::Package curl = installed("curl", "8.9.1", 4 * 1024 * 1024);
    curl.newVersion = "8.10.0";
    curl.status = Status::Outdated;
    curl.dependencies = {"brotli"};
    octo::Package brotli = installed("brotli", "1.2.0", 1024 * 1024);
    brotli.automatic = true;
    octo::Package wget = installed("wget", "1.24.5", 25 * 1024 * 1024);
    wget.pinned = true;
    wget.newVersion = "1.25.0_1";
    wget.status = Status::Pinned;
    wget.dependencies = {"brotli"};
    return {curl, brotli, wget, installed("alacritty", "0.17.0", 14 * 1024 * 1024, Kind::Cask)};
}

tui::Listing listing()
{
    tui::Listing l;
    l.installed = sample();
    return l;
}

octo::Details curlDetails()
{
    octo::Details d = octo::parseDetails(kCurlInfo);
    d.files = {"/opt/homebrew/Cellar/curl/8.9.1/bin/curl", "/opt/homebrew/Cellar/curl/8.9.1/lib/libcurl.4.dylib"};
    return d;
}

std::string draw(const tui::ViewState &s, int width = 120, int height = 40)
{
    auto screen = Screen::Create(Dimension::Fixed(width), Dimension::Fixed(height));
    Render(screen, tui::render(s, text("")));
    // Sem as sequências de cor e estilo, que o FTXUI intercala no texto.
    const std::string raw = screen.ToString();
    std::string out;
    for (size_t i = 0; i < raw.size(); ++i) {
        if (raw[i] == '\x1b' && i + 1 < raw.size() && raw[i + 1] == '[') {
            i += 2;
            while (i < raw.size() && !(raw[i] >= '@' && raw[i] <= '~'))
                ++i;
            continue;
        }
        out += raw[i];
    }
    return out;
}

bool has(const std::string &haystack, const std::string &needle)
{
    return haystack.find(needle) != std::string::npos;
}

// CHECK de cada texto de uma lista, dizendo qual faltou.
#define CHECK_INFO(out, t)                                                                              \
    do {                                                                                                \
        INFO(t);                                                                                        \
        CHECK(has(out, t));                                                                             \
    } while (false)

tui::Options options(const TempDir &dir, const std::string &brew)
{
    tui::Options o;
    o.brewPath = brew;
    o.iconMode = "ascii";
    o.language = "en_US";
    o.settingsPath = dir.file("config");
    return o;
}

// App com a lista de sample(), sem brew: nada aqui chama o brew.
struct AppFixture {
    TempDir dir;
    tui::App app;
    AppFixture()
        : app(options(dir, dir.file("brew-nao-usado")))
    {
        app.applyListing(listing());
    }
    std::string selected()
    {
        const tui::ViewState s = app.viewState();
        return s.hasCurrent ? s.current.key() : std::string();
    }
};

// App com o brew falso dos fixtures: a listagem, os tamanhos, o repositório
// e os detalhes passam pelo núcleo de verdade (sem tela, tudo roda na hora).
struct BrewFixture {
    FakeBrew brew;
    std::unique_ptr<tui::App> app;
    BrewFixture()
    {
        brew.writeApiCache();
        // Algo no Cellar e no Caskroom, para os tamanhos.
        (void)std::system(("mkdir -p '" + brew.dir.file("Cellar/curl/8.9.1") + "' '"
                           + brew.dir.file("Caskroom/alacritty/0.17.0") + "'").c_str());
        writeFile(brew.dir.file("Cellar/curl/8.9.1/curl"), std::string(2048, 'x'));
        writeFile(brew.dir.file("Caskroom/alacritty/0.17.0/a"), std::string(100, 'x'));
        app = std::make_unique<tui::App>(options(brew.dir, brew.path));
        app->refresh();
    }
    void select(const std::string &key)
    {
        app->handleEvent(Event::Home);
        for (int i = 0; i < 20 && app->viewState().current.key() != key; ++i)
            app->handleEvent(Event::ArrowDown);
        REQUIRE(app->viewState().current.key() == key);
    }
    std::string screen() { return draw(app->viewState()); }
};

} // namespace

TEST_CASE("tela mostra colunas, linhas e estado", "[view]")
{
    i18n::setLocale("en_US");
    AppFixture f;
    const std::string out = draw(f.app.viewState());
    for (const char *t : {"OctoBrew — Homebrew", "[# Installed packages]", "Name ^", "Type", "Version", "Size",
                          "Installed on", "curl", "brotli", "alacritty", "Formula", "Cask", "4.00 MiB", "25.00 MiB",
                          "Ready.", "4 package(s)", "2 outdated", "1 i Info", "2 f Files", "4 > Output", "6 ? Help",
                          "F1", "F5", "F8", "F9", "F10"})
        CHECK_INFO(out, t);
    // Só as abas da v0.1.0: as de Ações (3) e Notícias (5) ainda não existem.
    CHECK_FALSE(has(out, "3 a Actions"));
    CHECK_FALSE(has(out, "5 n News"));
    // Ordem pelo nome; brotli é dependência ("d"), curl desatualizado ("U"), wget fixado ("P").
    CHECK(out.find("alacritty") < out.find("brotli"));
    CHECK(has(out, " d brotli"));
    CHECK(has(out, " U curl"));
    CHECK(has(out, " P wget"));
    // Aba Informação do pacote selecionado (alacritty), ainda sem os detalhes.
    CHECK(has(out, "Status:"));
    CHECK(has(out, "Loading…"));
}

TEST_CASE("aba Informação de uma fórmula, com os caveats", "[view]")
{
    i18n::setLocale("en_US");
    AppFixture f;
    f.app.handleEvent(Event::ArrowDown);   // brotli
    f.app.handleEvent(Event::ArrowDown);   // curl
    REQUIRE(f.selected() == "f:curl");
    octo::Details d = curlDetails();
    d.requiredBy = {"wget"};
    f.app.applyDetails(f.app.currentDetailsKey(), d, "");
    tui::ViewState s = f.app.viewState();
    std::string out = draw(s);
    for (const char *t : {"Outdated (version 8.10.0 is available)", "Type:", "Formula", "Installed version:",
                          "8.9.1", "Tap:", "homebrew/core", "https://curl.se", "License:", "Keg-only:",
                          "provided by macos", "Installed size:", "4.00 MiB", "2 f Files (2)"})
        CHECK_INFO(out, t);
    // Dependências e caveats ficam abaixo: aparecem rolando a aba (Tab e setas).
    CHECK_FALSE(has(out, "Caveats"));
    s.contentFocused = true;
    s.contentScroll = 11;
    out = draw(s);
    for (const char *t : {"Depends on:", "libnghttp2", "Build dependencies:", "pkgconf", "Required by:", "wget",
                          "Caveats", "curl is keg-only", "  echo 'export PATH="})
        CHECK_INFO(out, t);
}

TEST_CASE("aba Informação de um cask, com os artifacts", "[view]")
{
    i18n::setLocale("en_US");
    AppFixture f;
    REQUIRE(f.selected() == "c:alacritty");
    octo::Details d = octo::parseDetails(kKittyInfo);
    d.autoUpdates = true;
    f.app.applyDetails(f.app.currentDetailsKey(), d, "");
    tui::ViewState s = f.app.viewState();
    s.contentFocused = true;
    s.contentScroll = 3;
    const std::string out = draw(s);
    for (const char *t : {"Cask", "Application:", "Auto-updates:", "yes (brew upgrade skips it)", "Deprecated:",
                          "unmaintained", "Conflicts with:", "kitty@nightly", "Artifacts:", "App: kitty.app",
                          "Command wrapper: kitty"})
        CHECK_INFO(out, t);
}

TEST_CASE("aba Arquivos e erro nos detalhes", "[view]")
{
    i18n::setLocale("en_US");
    AppFixture f;
    f.app.handleEvent(Event::ArrowDown);
    f.app.handleEvent(Event::ArrowDown);   // curl
    f.app.applyDetails(f.app.currentDetailsKey(), curlDetails(), "");
    f.app.handleEvent(Event::Character('2'));
    std::string out = draw(f.app.viewState());
    CHECK(has(out, "/opt/homebrew/Cellar/curl/8.9.1/lib/libcurl.4.dylib"));

    f.app.handleEvent(Event::ArrowDown);   // wget, sem detalhes
    f.app.applyDetails(f.app.currentDetailsKey(), {}, "boom");
    out = draw(f.app.viewState());
    CHECK(has(out, "ERROR: boom"));
}

TEST_CASE("ícones Nerd Font no modo padrão", "[view]")
{
    tui::ViewState s;
    s.rows = sample();
    s.selected = 0;
    s.iconMode = tui::IconMode::Nerd;
    const std::string out = draw(s);
    CHECK(has(out, ""));   // nf-fa-check_circle: instalado
    CHECK(has(out, ""));   // nf-fa-check_circle_o: dependência
    CHECK(has(out, ""));   // nf-fa-thumb_tack: fixado
    CHECK(has(out, ""));   // nf-fa-folder_open: aba Arquivos
}

TEST_CASE("tela em português", "[view]")
{
    i18n::setLocale("pt_BR");
    tui::ViewState s;
    s.rows = sample();
    s.selected = 0;
    s.hasCurrent = true;
    s.current = s.rows[2];
    s.iconMode = tui::IconMode::Ascii;
    const std::string out = draw(s);
    for (const char *t : {"Nome", "Tipo", "Versão", "Tamanho", "Instalado em", "Informação", "Arquivos", "Saída",
                          "Ajuda", "Instalados", "Fórmula", "Fixado (a versão 1.25.0_1 está disponível)",
                          "Carregando…", "Instalados/Repositório"})
        CHECK_INFO(out, t);
    i18n::setLocale("en_US");
}

TEST_CASE("enquanto os tamanhos são calculados", "[view]")
{
    i18n::setLocale("en_US");
    tui::ViewState s;
    s.rows = sample();
    s.rows[0].size = 0;
    s.sizing = true;
    s.iconMode = tui::IconMode::Ascii;
    const std::string out = draw(s);
    CHECK(has(out, "Calculating sizes…"));
    CHECK(has(out, "…  20"));   // o "…" na coluna do tamanho, antes da data
}

TEST_CASE("caveats quebram na largura da aba, com o recuo", "[view]")
{
    const auto lines = tui::wrapText("one two three four five six\n\n  indented words go here and wrap", 20);
    REQUIRE(lines.size() >= 4);
    CHECK(lines[0] == "one two three four");
    CHECK(lines[1] == "five six");
    CHECK(lines[2].empty());
    CHECK(lines[3].rfind("  indented", 0) == 0);
    for (const auto &l : lines)
        CHECK(l.size() <= 20);
    CHECK(lines.back().rfind("  ", 0) == 0);
}

TEST_CASE("F6 (ou s) escolhe a coluna e Enter ordena, e a ordem é salva", "[app]")
{
    AppFixture f;
    REQUIRE(f.app.handleEvent(Event::F6));
    CHECK(f.app.viewState().sortChooserOpen);
    for (int i = 0; i < 3; ++i)   // Nome → Tipo → Versão → Tamanho
        f.app.handleEvent(Event::ArrowDown);
    f.app.handleEvent(Event::Return);
    tui::ViewState s = f.app.viewState();
    CHECK_FALSE(s.sortChooserOpen);
    CHECK(s.sort == model::SortSpec{model::Size, true});
    CHECK(s.rows.front().name == "brotli");
    CHECK(f.app.settings().intValue("sort.column", -1) == model::Size);

    // De novo na mesma coluna: inverte. "s" faz o mesmo que F6 (sem o fn do Mac).
    f.app.handleEvent(Event::Character('s'));
    f.app.handleEvent(Event::Return);
    CHECK(f.app.viewState().sort == model::SortSpec{model::Size, false});
    CHECK(f.app.viewState().rows.front().name == "wget");
    // "(ordem do brew)": volta à ordem original.
    f.app.handleEvent(Event::F6);
    for (int i = 0; i < 8; ++i)
        f.app.handleEvent(Event::ArrowUp);
    f.app.handleEvent(Event::Return);
    CHECK(f.app.viewState().rows.front().name == "curl");
    // Pelo tipo: as fórmulas, depois os casks.
    f.app.handleEvent(Event::F6);
    f.app.handleEvent(Event::ArrowDown);
    f.app.handleEvent(Event::ArrowDown);
    f.app.handleEvent(Event::ArrowDown);
    f.app.handleEvent(Event::Return);
    CHECK(f.app.viewState().sort == model::SortSpec{model::Type, true});
    CHECK(f.app.viewState().rows.back().name == "alacritty");
}

TEST_CASE("seleção sobrevive à ordenação e ao filtro", "[app]")
{
    AppFixture f;
    CHECK(f.selected() == "c:alacritty");
    f.app.handleEvent(Event::ArrowDown);
    f.app.handleEvent(Event::ArrowDown);
    CHECK(f.selected() == "f:curl");
    f.app.handleEvent(Event::F6);
    for (int i = 0; i < 3; ++i)
        f.app.handleEvent(Event::ArrowDown);
    f.app.handleEvent(Event::Return);   // por tamanho
    CHECK(f.selected() == "f:curl");
    f.app.filterText() = "wg";          // curl some: seleciona a primeira
    CHECK(f.selected() == "f:wget");
    f.app.filterText().clear();
    CHECK(f.selected() == "f:wget");
}

TEST_CASE("janela da tabela rola com a seleção", "[app]")
{
    AppFixture f;
    tui::Listing l;
    for (int i = 0; i < 100; ++i)
        l.installed.push_back(installed("p" + std::string(i < 10 ? "0" : "") + std::to_string(i), "1", i));
    f.app.applyListing(l);
    f.app.setTableHeight(10);
    tui::ViewState s = f.app.viewState();
    CHECK(s.rows.size() == 10);
    CHECK(s.rowCount == 100);
    f.app.handleEvent(Event::End);
    s = f.app.viewState();
    CHECK(s.firstRow == 90);
    CHECK(s.rows.back().name == "p99");
    CHECK(s.selected == 9);
    f.app.handleEvent(Event::PageUp);
    s = f.app.viewState();
    CHECK(s.current.name == "p90");
    CHECK(s.firstRow == 90);
    f.app.handleEvent(Event::ArrowUp);
    s = f.app.viewState();
    CHECK(s.firstRow == 89);
    CHECK(s.selected == 0);
}

TEST_CASE("atalhos de busca, abas e foco", "[app]")
{
    AppFixture f;
    CHECK(f.app.handleEvent(Event::Character('/')));
    CHECK(f.app.filterMode());
    CHECK_FALSE(f.app.handleEvent(Event::Character('x')));   // vai para o campo de filtro
    f.app.handleEvent(Event::Escape);
    CHECK_FALSE(f.app.filterMode());
    f.app.handleEvent(Event::Special("\x0c"));                 // Ctrl+L, como no OctoPkg
    CHECK(f.app.filterMode());
    f.app.handleEvent(Event::Special("\x10"));                 // Ctrl+P volta à lista
    CHECK_FALSE(f.app.filterMode());

    f.app.handleEvent(Event::Character('2'));
    CHECK(f.app.viewState().tab == tui::FilesTab);
    f.app.handleEvent(Event::Special("\x1b" "4"));             // Alt+4: Saída
    CHECK(f.app.viewState().tab == tui::OutputTab);
    f.app.handleEvent(Event::Special("\x1b" "6"));             // Alt+6: Ajuda
    CHECK(f.app.viewState().tab == tui::HelpTab);
    // 3 e 5 são das abas Ações e Notícias, que ainda não existem: nada muda.
    f.app.handleEvent(Event::Character('3'));
    CHECK(f.app.viewState().tab == tui::HelpTab);
    f.app.handleEvent(Event::Special("\x1b" "5"));
    CHECK(f.app.viewState().tab == tui::HelpTab);
    f.app.handleEvent(Event::Return);                          // Enter: aba Informação
    CHECK(f.app.viewState().tab == tui::InfoTab);
    f.app.handleEvent(Event::F1);
    CHECK(f.app.viewState().tab == tui::HelpTab);
    f.app.handleEvent(Event::Character('1'));
    f.app.handleEvent(Event::Character('?'));                  // ? faz o F1 sem o fn
    CHECK(f.app.viewState().tab == tui::HelpTab);

    // Tab: as setas rolam a aba em vez de mover a seleção.
    f.app.handleEvent(Event::Tab);
    CHECK(f.app.viewState().contentFocused);
    const std::string before = f.selected();
    (void)draw(f.app.viewState());
    f.app.handleEvent(Event::ArrowDown);
    CHECK(f.selected() == before);
    f.app.handleEvent(Event::Escape);
    CHECK_FALSE(f.app.viewState().contentFocused);
}

TEST_CASE("Saída lista os desatualizados; erro mantém a última lista", "[app]")
{
    AppFixture f;
    tui::ViewState s = f.app.viewState();
    bool curl = false, wget = false, summary = false;
    for (const auto &l : s.log) {
        curl = curl || has(l, "curl  8.9.1 -> 8.10.0");
        wget = wget || has(l, "wget  1.24.5 -> 1.25.0_1  [Pinned]");
        summary = summary || has(l, "4 package(s) installed: 3 formula(e) and 1 cask(s).");
    }
    CHECK(curl);
    CHECK(wget);
    CHECK(summary);

    tui::Listing bad;
    bad.error = "boom";
    f.app.applyListing(bad);
    s = f.app.viewState();
    CHECK(s.error);
    CHECK(s.status == "ERROR: boom");
    CHECK(s.log.back().find("ERROR: boom") != std::string::npos);
    CHECK(s.totalPackages == 4);
}

// --- Com o brew falso: o caminho completo ---

TEST_CASE("listagem pelo brew, com os tamanhos em disco", "[app][brew]")
{
    i18n::setLocale("en_US");
    BrewFixture f;
    tui::ViewState s = f.app->viewState();
    CHECK(s.totalPackages == 6);
    CHECK(s.status == "Ready.");
    CHECK_FALSE(s.sizing);
    std::string out = f.screen();
    for (const char *t : {"foo", "docker", "2.00 KiB", "100.00 Bytes", "6 package(s)", "3 outdated"})
        CHECK_INFO(out, t);
    bool disk = false;
    for (const auto &l : s.log)
        disk = disk || has(l, "Disk usage of the installed packages: 2.10 KiB");
    CHECK(disk);
}

TEST_CASE("detalhes pelo brew, com o Requerido por", "[app][brew][details]")
{
    i18n::setLocale("en_US");
    BrewFixture f;
    f.select("f:curl");
    f.app->loadDetails();
    tui::ViewState s = f.app->viewState();
    REQUIRE(s.detailsReady);
    CHECK(s.details.homepage == "https://curl.se");
    CHECK(s.details.files.size() == 2);
    CHECK(s.details.requiredBy.empty());

    // brotli: o brew falso não tem o info dele; o erro aparece na aba.
    f.select("f:brotli");
    f.app->loadDetails();
    s = f.app->viewState();
    CHECK(has(s.detailsError, "No available formula with the name \"brotli\""));
    CHECK(has(f.screen(), "ERROR: No available formula"));
}

TEST_CASE("visão Repositório pelo cache da API", "[app][brew][remote]")
{
    i18n::setLocale("en_US");
    BrewFixture f;
    f.app->handleEvent(Event::Character('i'));
    tui::ViewState s = f.app->viewState();
    CHECK(s.remoteView);
    CHECK(s.totalPackages == 9);
    const std::string out = f.screen();
    for (const char *t : {"[@ Repository]", " - vim", " - kitty", " N alacritty", "9 package(s)"})
        CHECK_INFO(out, t);
    // Fórmula e cask de mesmo nome: duas linhas, e a seleção distingue.
    f.select("c:docker");
    CHECK(f.app->viewState().current.installed);
    CHECK(f.app->viewState().current.status == Status::Outdated);
    f.select("f:docker");
    CHECK_FALSE(f.app->viewState().current.installed);
    // O tamanho do instalado vale também nesta visão.
    f.select("f:curl");
    CHECK(f.app->viewState().current.size == 2048);
    CHECK(f.app->viewState().current.version == "8.10.0");

    // Alt+I volta aos instalados sem chamar o brew de novo.
    const size_t calls = f.brew.calls().size();
    f.app->handleEvent(Event::Special("\x1b" "i"));
    CHECK_FALSE(f.app->viewState().remoteView);
    CHECK(f.brew.calls().size() == calls);
}

TEST_CASE("sem o cache da API, a visão Repositório avisa e volta aos instalados", "[app][brew][remote]")
{
    i18n::setLocale("en_US");
    FakeBrew brew;   // sem writeApiCache()
    TempDir dir;
    tui::App app(options(dir, brew.path));
    app.refresh();
    app.handleEvent(Event::Character('i'));
    const tui::ViewState s = app.viewState();
    CHECK_FALSE(s.remoteView);
    CHECK(s.error);
    CHECK(has(s.status, "brew update"));
    CHECK(s.totalPackages == 6);
}

TEST_CASE("menus Arquivo, Busca e Ajuda, e o Sobre", "[app][menu]")
{
    i18n::setLocale("en_US");
    BrewFixture f;
    f.app->handleEvent(Event::F9);
    REQUIRE(f.app->dialog().kind == tui::Dialog::Menu);
    std::string out = f.screen();
    for (const char *t : {"File", "Search", "Help", "Refresh the package list", "Preferences…", "Exit"})
        CHECK_INFO(out, t);
    CHECK_FALSE(has(out, "Actions"));
    // Instalados/Repositório pelo menu.
    f.app->handleEvent(Event::ArrowDown);
    f.app->handleEvent(Event::Return);
    CHECK(f.app->viewState().remoteView);

    f.app->handleEvent(Event::Special("\x1b" "j"));   // Alt+J: Ajuda
    REQUIRE(f.app->dialog().menu == 2);               // Arquivo, Busca, Ajuda
    f.app->handleEvent(Event::ArrowDown);
    f.app->handleEvent(Event::Return);
    REQUIRE(f.app->dialog().kind == tui::Dialog::Message);
    CHECK(f.app->dialog().title == "About");
    CHECK(has(f.app->dialog().message, std::string("octobrew-tui ") + OCTOBREW_TUI_VERSION));
    CHECK(has(f.app->dialog().message, "Homebrew 7.0.10"));
    f.app->handleEvent(Event::Return);
    CHECK(f.app->dialog().kind == tui::Dialog::None);
}

// --- Temas, preferências e busca nas abas (portados do octopkg-tui) ---

namespace {

// A tela com as sequências de cor, para conferir as cores do tema.
std::string drawRaw(const tui::ViewState &s, int width = 120, int height = 40)
{
    Terminal::SetColorSupport(Terminal::Color::TrueColor);
    auto screen = Screen::Create(Dimension::Fixed(width), Dimension::Fixed(height));
    Render(screen, tui::render(s, text("")));
    return screen.ToString();
}

} // namespace

TEST_CASE("os temas do bectl-gui, em ciclo", "[theme]")
{
    const auto &list = tui::themes();
    REQUIRE(list.size() == 15);
    CHECK(std::string(list.front().id) == "system");
    std::set<std::string> ids;
    for (const auto &t : list)
        ids.insert(t.id);
    CHECK(ids.size() == list.size());
    CHECK(std::string(tui::theme("nao-existe").id) == "system");
    CHECK(std::string(tui::nextTheme("system").id) == "octopkg");
    CHECK(std::string(tui::nextTheme("one_dark").id) == "system");
}

TEST_CASE("tema pinta o fundo, o texto e a seleção", "[theme][view]")
{
    i18n::setLocale("en_US");
    tui::ViewState s;
    s.rows = sample();
    s.selected = 0;
    s.iconMode = tui::IconMode::Ascii;
    s.themeId = "nord";
    const std::string raw = drawRaw(s);
    CHECK(raw.find("48;2;46;52;64") != std::string::npos);    // fundo #2e3440
    CHECK(raw.find("38;2;216;222;233") != std::string::npos); // texto #d8dee9
    CHECK(raw.find("48;2;129;161;193") != std::string::npos); // seleção #81a1c1
    CHECK(draw(s).find("Theme: Nord") != std::string::npos);

    s.themeId = "system";
    const std::string plain = drawRaw(s);
    CHECK(plain.find("48;2;") == std::string::npos);          // sem cores próprias
}

TEST_CASE("Alt+T troca o tema e o salva", "[theme][app]")
{
    AppFixture f;
    CHECK(f.app.viewState().themeId == "system");
    f.app.handleEvent(Event::Special("\x1b" "t"));
    CHECK(f.app.viewState().themeId == "octopkg");
    CHECK(f.app.settings().value("theme") == "octopkg");
    for (int i = 0; i < 14; ++i)
        f.app.handleEvent(Event::Special("\x1b" "t"));
    CHECK(f.app.viewState().themeId == "system");
}

TEST_CASE("Preferências: tema, idioma e ícones, aplicados e salvos", "[app][preferences]")
{
    AppFixture f;
    f.app.handleEvent(Event::Special("\x1b" ","));   // Alt+,
    REQUIRE(f.app.viewState().dialog.kind == tui::Dialog::Form);
    const tui::Dialog d = f.app.viewState().dialog;
    CHECK(d.title == "Preferences");
    REQUIRE(d.fields.size() == 3);
    CHECK(d.fields[0].choices.size() == 15);
    CHECK(d.focus == 0);

    for (int i = 0; i < 12; ++i)   // Tema: system → … → nord
        f.app.handleEvent(Event::ArrowRight);
    f.app.handleEvent(Event::Tab);                   // Idioma
    f.app.handleEvent(Event::ArrowRight);            // en_US
    f.app.handleEvent(Event::ArrowRight);            // pt_BR
    f.app.handleEvent(Event::Tab);                   // Ícones
    f.app.handleEvent(Event::ArrowLeft);             // ASCII → Unicode
    f.app.handleEvent(Event::Return);

    const tui::ViewState s = f.app.viewState();
    CHECK(s.dialog.kind == tui::Dialog::None);
    CHECK(s.themeId == "nord");
    CHECK(s.iconMode == tui::IconMode::Unicode);
    CHECK(i18n::locale() == "pt_BR");
    CHECK(s.status == "Preferências salvas.");
    // Relido do arquivo, numa nova App.
    tui::Options o;
    o.brewPath = f.dir.file("brew-nao-usado");
    o.settingsPath = f.dir.file("config");
    tui::App again(o);
    CHECK(again.viewState().themeId == "nord");
    CHECK(again.viewState().iconMode == tui::IconMode::Unicode);
    i18n::setLocale("en_US");
}

TEST_CASE("Ctrl+F busca nas abas Arquivos, Saída e Ajuda", "[app][find]")
{
    i18n::setLocale("en_US");
    AppFixture f;
    f.app.handleEvent(Event::ArrowDown);
    f.app.handleEvent(Event::ArrowDown);   // curl
    octo::Details d = curlDetails();
    d.files = {"/opt/homebrew/Cellar/curl/8.9.1/bin/curl", "/opt/homebrew/Cellar/curl/8.9.1/include/curl/curl.h",
               "/opt/homebrew/Cellar/curl/8.9.1/lib/libcurl.4.dylib",
               "/opt/homebrew/Cellar/curl/8.9.1/share/man/man1/curl.1"};
    f.app.applyDetails(f.app.currentDetailsKey(), d, "");

    // Na aba Informação, não há onde buscar.
    f.app.handleEvent(Event::Special("\x06"));
    CHECK_FALSE(f.app.viewState().findOpen);
    CHECK(f.app.viewState().status == "Find works in the Files, Output and Help tabs.");

    f.app.handleEvent(Event::Character('2'));
    f.app.handleEvent(Event::Special("\x06"));
    REQUIRE(f.app.viewState().findOpen);
    for (char c : std::string("LIB/"))   // sem diferenciar maiúsculas
        f.app.handleEvent(Event::Character(c));
    tui::ViewState s = f.app.viewState();
    CHECK(s.findCount == 1);
    CHECK(s.findMatch == 2);
    CHECK(draw(s).find("1/1") != std::string::npos);
    for (int i = 0; i < 4; ++i)
        f.app.handleEvent(Event::Backspace);
    for (char c : std::string("/curl."))
        f.app.handleEvent(Event::Character(c));
    s = f.app.viewState();
    CHECK(s.findCount == 2);   // curl.h e curl.1
    f.app.handleEvent(Event::F3);
    CHECK(f.app.viewState().findMatch == 3);
    f.app.handleEvent(Event::Special("\x1b[1;2R"));   // Shift+F3
    CHECK(f.app.viewState().findMatch == 1);
    // q é texto da busca, não sair.
    CHECK(f.app.handleEvent(Event::Character('q')));
    CHECK(f.app.viewState().findText == "/curl.q");
    f.app.handleEvent(Event::Escape);
    CHECK_FALSE(f.app.viewState().findOpen);

    // Na Ajuda, encontra o próprio atalho.
    f.app.handleEvent(Event::F1);
    f.app.handleEvent(Event::Special("\x06"));
    for (char c : std::string("Ctrl+F"))
        f.app.handleEvent(Event::Character(c));
    CHECK(f.app.viewState().findCount >= 1);
}

TEST_CASE("F7 (ou Alt+D) alterna a busca por nome e por descrição, e a escolha fica salva", "[app][search]")
{
    i18n::setLocale("en_US");
    AppFixture f;
    f.app.filterText() = "comment";   // sample(): comment = nome + " comment"
    CHECK(f.app.viewState().rowCount == 0);

    f.app.handleEvent(Event::F7);
    tui::ViewState s = f.app.viewState();
    CHECK(s.searchMode == model::SearchMode::Description);
    CHECK(s.status == "Search by description (name and description).");
    CHECK(f.app.settings().value("search") == "description");
    CHECK(s.rowCount == 4);
    f.app.filterText() = "wget comment";
    s = f.app.viewState();
    REQUIRE(s.rowCount == 1);
    CHECK(s.current.name == "wget");
    CHECK(draw(s).find("Description:") != std::string::npos);

    f.app.handleEvent(Event::Special("\x1b" "d"));   // Alt+D: o F7 sem o fn
    CHECK(f.app.viewState().searchMode == model::SearchMode::Name);
    CHECK(f.app.viewState().rowCount == 0);
    CHECK(f.app.settings().value("search") == "name");
}

TEST_CASE("menu Busca, com o modo atual marcado", "[app][search][menu]")
{
    i18n::setLocale("en_US");
    AppFixture f;
    f.app.handleEvent(Event::Special("\x1b" "b"));   // Alt+B
    REQUIRE(f.app.viewState().dialog.kind == tui::Dialog::Menu);
    CHECK(f.app.viewState().dialog.menu == 1);
    CHECK(draw(f.app.viewState()).find("• By name") != std::string::npos);
    f.app.handleEvent(Event::ArrowDown);   // Por descrição
    f.app.handleEvent(Event::Return);
    CHECK(f.app.viewState().searchMode == model::SearchMode::Description);
    f.app.handleEvent(Event::Special("\x1b" "b"));
    CHECK(draw(f.app.viewState()).find("• By description") != std::string::npos);
    f.app.handleEvent(Event::Escape);
}

TEST_CASE("F8 (ou t) filtra pelo tap, que combina com o filtro", "[app][category]")
{
    i18n::setLocale("en_US");
    AppFixture f;
    tui::Listing l = listing();
    l.installed[1].tap = "someone/tap";   // brotli
    f.app.applyListing(l);
    f.app.handleEvent(Event::F8);
    tui::ViewState s = f.app.viewState();
    REQUIRE(s.categoryChooserOpen);
    CHECK(s.categoryIndex == 0);   // (todos), o atual
    CHECK(s.categories.size() == 3);
    std::string out = draw(s);
    for (const char *t : {" Tap ", "• (all taps)", "homebrew/cask", "homebrew/core", "someone/tap", "1/4"})
        CHECK_INFO(out, t);

    f.app.handleEvent(Event::Character('s'));   // salta para someone/tap
    CHECK(f.app.viewState().categoryIndex == 3);
    f.app.handleEvent(Event::Character('h'));   // e para homebrew/cask
    CHECK(f.app.viewState().categoryIndex == 1);
    f.app.handleEvent(Event::Character('h'));   // homebrew/core
    f.app.handleEvent(Event::Return);
    s = f.app.viewState();
    CHECK_FALSE(s.categoryChooserOpen);
    CHECK(s.category == "homebrew/core");
    CHECK(s.status == "Tap: homebrew/core");
    CHECK(s.rowCount == 2);   // curl e wget
    CHECK(draw(s).find("[# Installed packages · homebrew/core]") != std::string::npos);
    f.app.filterText() = "wg";
    CHECK(f.app.viewState().rowCount == 1);
    f.app.filterText().clear();

    // De novo, pelo t: o atual vem marcado; Home e Enter voltam a todos.
    f.app.handleEvent(Event::Character('t'));
    s = f.app.viewState();
    CHECK(s.categoryIndex == 2);
    f.app.handleEvent(Event::Home);
    f.app.handleEvent(Event::Return);
    CHECK(f.app.viewState().category.empty());
    CHECK(f.app.viewState().rowCount == 4);

    // Sem pacotes, a lista de taps fica vazia e não quebra.
    AppFixture empty;
    empty.app.applyListing(tui::Listing{});
    empty.app.handleEvent(Event::F8);
    REQUIRE(empty.app.viewState().categoryChooserOpen);
    empty.app.handleEvent(Event::Character('x'));
    empty.app.handleEvent(Event::Escape);
    CHECK_FALSE(empty.app.viewState().categoryChooserOpen);
}

TEST_CASE("F11 e Alt+Z: só a lista, só as abas, e as duas", "[app][layout]")
{
    i18n::setLocale("en_US");
    AppFixture f;
    f.app.handleEvent(Event::ArrowDown);
    f.app.handleEvent(Event::ArrowDown);   // curl
    f.app.applyDetails(f.app.currentDetailsKey(), curlDetails(), "");

    f.app.handleEvent(Event::F11);
    tui::ViewState s = f.app.viewState();
    CHECK(s.layout == tui::Layout::ListOnly);
    std::string out = draw(s);
    CHECK(has(out, "brotli"));
    CHECK_FALSE(has(out, "1 i Info"));    // sem as abas
    f.app.handleEvent(Event::ArrowDown);  // as setas movem a seleção
    CHECK(f.app.viewState().current.name == "wget");
    f.app.handleEvent(Event::ArrowUp);

    f.app.handleEvent(Event::Special("\x1b" "z"));   // Alt+Z
    s = f.app.viewState();
    CHECK(s.layout == tui::Layout::TabsOnly);
    CHECK(s.contentFocused);
    out = draw(s);
    CHECK(has(out, "https://curl.se"));
    CHECK_FALSE(has(out, "Name ^"));      // sem a lista
    f.app.handleEvent(Event::ArrowDown);
    CHECK(f.app.viewState().current.name == "curl");

    f.app.handleEvent(Event::F11);
    CHECK(f.app.viewState().layout == tui::Layout::Normal);
    CHECK_FALSE(f.app.viewState().contentFocused);
}

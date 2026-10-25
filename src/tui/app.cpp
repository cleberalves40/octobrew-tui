#include "app.h"
#include "i18n.h"
#include "theme.h"

#include <algorithm>
#include <cctype>
#include <ctime>
#include <ftxui/component/component.hpp>
#include <ftxui/component/mouse.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/screen/terminal.hpp>

using namespace ftxui;

namespace tui {

namespace {

// Teclas que o FTXUI entrega como sequência crua.
const std::string kCtrlL = "\x0c";
const std::string kCtrlP = "\x10";
const std::string kCtrlQ = "\x11";
const std::string kCtrlR = "\x12";
const std::string kAltA = "\x1b" "a";
const std::string kAltB = "\x1b" "b";
const std::string kAltD = "\x1b" "d";
const std::string kAltI = "\x1b" "i";
const std::string kAltJ = "\x1b" "j";
const std::string kAltT = "\x1b" "t";
const std::string kAltZ = "\x1b" "z";
const std::string kAltComma = "\x1b" ",";
const std::string kCtrlF = "\x06";
const std::string kShiftF3 = "\x1b[1;2R";   // xterm; o FTXUI não tem um evento próprio

enum Menus { FileMenu = 0, SearchMenu, HelpMenu };

// Linhas fixas da tela: título, separadores, cabeçalho, abas, status e teclas.
constexpr int kFixedLines = 9;

// Remove o último caractere UTF-8 (Backspace no campo de texto).
void popUtf8(std::string &s)
{
    while (!s.empty()) {
        const unsigned char c = static_cast<unsigned char>(s.back());
        s.pop_back();
        if ((c & 0xC0) != 0x80)
            break;
    }
}

std::string detailsKey(const octo::Package &p)
{
    return (p.installed ? "I:" : "R:") + p.key();
}

} // namespace

App::App(const Options &options)
    : m_settings(options.settingsPath)
    , m_mgr(options.brewPath)
    , m_iconMode(iconModeFromString(options.iconMode.empty() ? m_settings.value("icons", "nerd")
                                                             : options.iconMode))
    , m_remoteView(options.remote)
{
    const std::string lang = options.language.empty() ? m_settings.value("language", "system")
                                                      : options.language;
    if (lang == "system")
        i18n::initFromEnvironment();
    else
        i18n::setLocale(lang);
    m_sort.column = m_settings.intValue("sort.column", model::Name);
    if (m_sort.column < -1 || m_sort.column >= model::ColumnCount)
        m_sort.column = model::Name;
    m_sort.ascending = m_settings.value("sort.order", "asc") != "desc";
    m_themeId = theme(options.theme.empty() ? m_settings.value("theme", "system") : options.theme).id;
    m_searchMode = m_settings.value("search", "name") == "description" ? model::SearchMode::Description
                                                                       : model::SearchMode::Name;
    m_placeholder = filterPlaceholder();
    m_status = i18n::tr("Ready.");
}

App::~App()
{
    m_cancelSizes = true;
    joinWorkers();
}

void App::joinWorkers()
{
    if (m_worker.joinable())
        m_worker.join();
    if (m_detailWorker.joinable())
        m_detailWorker.join();
}

void App::log(const std::string &message)
{
    char ts[16];
    const std::time_t now = std::time(nullptr);
    std::tm tm {};
    ::localtime_r(&now, &tm);
    std::strftime(ts, sizeof ts, "%H:%M:%S", &tm);
    m_log.push_back(std::string("[") + ts + "] " + message);
    if (m_log.size() > 2000)
        m_log.erase(m_log.begin());
}

// Lista em segundo plano: a tela continua respondendo enquanto o brew roda.
// Depois, na mesma thread, mede o que cada instalado ocupa em disco.
void App::refresh()
{
    if (m_busy)
        return;
    m_busy = true;
    m_error = false;
    const bool withRemote = m_remoteView;
    m_status = withRemote ? i18n::tr("Listing repository packages…") : i18n::tr("Listing installed packages…");
    const auto list = [this, withRemote] {
        Listing l;
        try {
            l.installed = m_mgr.listInstalled();
            if (withRemote) {
                l.remote = m_mgr.listRemote();
                l.hasRemote = true;
            }
        } catch (const std::exception &e) {
            l.error = e.what();
        }
        return l;
    };
    const auto measure = [this](const std::vector<octo::Package> &installed) {
        try {
            return m_mgr.sizes(installed, &m_cancelSizes);
        } catch (const std::exception &) {
            return std::map<std::string, std::int64_t>();   // sem os tamanhos, a lista continua
        }
    };
    if (!m_screen) {   // testes
        const Listing l = list();
        applyListing(l);
        if (l.error.empty())
            applySizes(measure(l.installed));
        return;
    }
    // Um cálculo de tamanhos ainda em andamento para logo: a nova lista o refaz.
    m_cancelSizes = true;
    if (m_worker.joinable())
        m_worker.join();
    m_cancelSizes = false;
    const int generation = ++m_generation;
    m_worker = std::thread([this, list, measure, generation] {
        const Listing l = list();
        const bool measuring = l.error.empty() && !l.installed.empty();
        m_screen->Post([this, l, measuring, generation] {
            if (generation != m_generation)
                return;
            applyListing(l);
            m_sizing = measuring;
        });
        m_screen->PostEvent(Event::Custom);
        if (!measuring)
            return;
        const auto sizes = measure(l.installed);
        const bool cancelled = m_cancelSizes;
        m_screen->Post([this, sizes, cancelled, generation] {
            if (generation != m_generation)
                return;
            m_sizing = false;
            if (!cancelled)
                applySizes(sizes);
        });
        m_screen->PostEvent(Event::Custom);
    });
}

void App::applyListing(const Listing &l)
{
    m_busy = false;
    m_details.clear();
    m_detailErrors.clear();
    if (!l.error.empty()) {
        m_error = true;
        m_status = i18n::trf("ERROR: %1", {l.error});
        log(m_status);
        // Sem o catálogo do repositório, volta aos instalados.
        if (m_remoteView && !m_hasRemote && !l.installed.empty())
            m_remoteView = false;
        if (l.installed.empty())
            return;
    }
    m_installed = l.installed;
    // Os tamanhos da medição anterior ficam até a nova terminar.
    for (auto &p : m_installed) {
        const auto it = m_sizes.find(p.key());
        if (it != m_sizes.end())
            p.size = it->second;
    }
    std::vector<const octo::Package *> outdated;
    for (const auto &p : m_installed)
        if (!p.newVersion.empty())
            outdated.push_back(&p);
    m_outdated = static_cast<int>(outdated.size());
    if (l.hasRemote) {
        m_remote = l.remote;
        m_hasRemote = true;
    }
    rebuild();

    if (l.error.empty()) {
        m_status = i18n::tr("Ready.");
        m_error = false;
    }
    int formulae = 0, casks = 0;
    for (const auto &p : m_installed)
        ++(p.kind == octo::Kind::Cask ? casks : formulae);
    log(i18n::trf("%1 package(s) installed: %2 formula(e) and %3 cask(s).",
                  {std::to_string(m_installed.size()), std::to_string(formulae), std::to_string(casks)}));
    if (l.hasRemote)
        log(i18n::trf("%1 package(s) in the repository.", {std::to_string(m_remote.size())}));
    if (!l.warning.empty())
        log(i18n::trf("WARNING: %1", {l.warning}));
    if (!outdated.empty()) {
        log(i18n::trf("Outdated packages (%1):", {std::to_string(outdated.size())}));
        for (const auto *p : outdated)
            log("  " + p->fullName + "  " + p->installedVersion + " -> " + p->newVersion
                + (p->pinned ? "  [" + i18n::tr("Pinned") + "]" : std::string()));
    }
}

void App::applySizes(const std::map<std::string, std::int64_t> &sizes)
{
    m_sizes = sizes;
    std::int64_t total = 0;
    for (auto &p : m_installed) {
        const auto it = m_sizes.find(p.key());
        p.size = it == m_sizes.end() ? 0 : it->second;
        total += p.size;
    }
    if (!m_installed.empty())
        log(i18n::trf("Disk usage of the installed packages: %1", {octo::formatSize(total)}));
    rebuild();
}

void App::rebuild()
{
    m_all = (m_remoteView && m_hasRemote) ? octo::mergeRemote(m_remote, m_installed) : m_installed;
    m_rowsDirty = true;
}

void App::toggleView()
{
    if (m_busy) {
        m_status = i18n::tr("Wait for the current operation to finish.");
        return;
    }
    m_remoteView = !m_remoteView;
    if (m_remoteView && !m_hasRemote) {
        refresh();
        return;
    }
    rebuild();
}

const std::vector<int> &App::rows()
{
    if (m_rowsDirty || m_rowsFilter != m_filter || !(m_rowsSort == m_sort) || m_rowsMode != m_searchMode
        || m_rowsCategory != m_category) {
        m_rows = model::visibleRows(m_all, m_filter, m_sort, m_searchMode, m_category);
        m_rowsCategory = m_category;
        m_rowsFilter = m_filter;
        m_rowsSort = m_sort;
        m_rowsMode = m_searchMode;
        m_rowsDirty = false;
    }
    return m_rows;
}

void App::layout()
{
    if (!m_screen)
        return;
    const auto size = Terminal::Size();
    const int height = size.dimy;
    // Sobra espaço para o indicador de posição ("12/345") da aba com foco.
    m_contentWidth = std::max(20, size.dimx - 9);
    const int free = std::max(2, height - kFixedLines);
    switch (m_layout) {
    case Layout::ListOnly:   // sem a barra de abas, a aba e um separador
        m_tableHeight = std::max(1, free + 2);
        m_contentHeight = 3;
        break;
    case Layout::TabsOnly:   // sem o cabeçalho, a lista e um separador
        m_contentHeight = std::max(3, free + 2);
        m_tableHeight = 1;
        break;
    default:
        m_contentHeight = std::max(3, std::min(16, free * 2 / 5));
        m_tableHeight = std::max(1, free - m_contentHeight);
        break;
    }
}

const octo::Package *App::current()
{
    const auto &r = rows();
    const int sel = model::rowToSelect(m_all, r, m_selectedKey);
    return sel < 0 ? nullptr : &m_all[static_cast<size_t>(r[static_cast<size_t>(sel)])];
}

std::string App::currentDetailsKey()
{
    const octo::Package *p = current();
    return p ? detailsKey(*p) : std::string();
}

octo::Details App::fetchDetails(const octo::Package &p, std::string *error)
{
    octo::Details d;
    try {
        d = m_mgr.details(p);
    } catch (const std::exception &e) {
        *error = e.what();
    }
    return d;
}

void App::requestDetails()
{
    if (!m_screen || m_detailBusy)
        return;
    const octo::Package *p = current();
    if (!p)
        return;
    const std::string key = detailsKey(*p);
    if (m_details.count(key) || m_detailErrors.count(key))
        return;
    m_detailBusy = true;
    if (m_detailWorker.joinable())
        m_detailWorker.join();
    const octo::Package pkg = *p;
    // "Requerido por" vem da lista de instalados, que só a thread da interface lê.
    const std::vector<std::string> requiredBy = octo::dependents(m_installed, pkg.fullName);
    m_detailWorker = std::thread([this, key, pkg, requiredBy] {
        std::string error;
        octo::Details d = fetchDetails(pkg, &error);
        d.requiredBy = requiredBy;
        m_screen->Post([this, key, d, error] { applyDetails(key, d, error); });
        m_screen->PostEvent(Event::Custom);
    });
}

void App::loadDetails()
{
    const octo::Package *p = current();
    if (!p)
        return;
    std::string error;
    octo::Details d = fetchDetails(*p, &error);
    d.requiredBy = octo::dependents(m_installed, p->fullName);
    applyDetails(detailsKey(*p), d, error);
}

void App::applyDetails(const std::string &key, const octo::Details &details, const std::string &error)
{
    m_detailBusy = false;
    if (m_details.size() > 256) {
        m_details.clear();
        m_detailErrors.clear();
    }
    if (error.empty())
        m_details[key] = details;
    else
        m_detailErrors[key] = error;
}

ViewState App::viewState()
{
    layout();
    ViewState s;
    const auto &r = rows();
    // A seleção acompanha o pacote, não a linha: sobrevive a filtro, ordem e atualização.
    const int sel = model::rowToSelect(m_all, r, m_selectedKey);
    m_selectedKey = sel >= 0 ? m_all[static_cast<size_t>(r[static_cast<size_t>(sel)])].key() : std::string();
    // Outro pacote (seta, filtro, ordem ou atualização): a aba volta ao início.
    if (m_selectedKey != m_shownKey) {
        m_shownKey = m_selectedKey;
        m_contentScroll = 0;
    }
    const int count = static_cast<int>(r.size());
    m_offset = model::scrollOffset(sel, m_offset, m_tableHeight, count);
    for (int i = m_offset; i < count && i < m_offset + m_tableHeight; ++i)
        s.rows.push_back(m_all[static_cast<size_t>(r[static_cast<size_t>(i)])]);
    s.firstRow = m_offset;
    s.rowCount = count;
    s.selected = sel >= m_offset && sel < m_offset + m_tableHeight ? sel - m_offset : -1;
    if (sel >= 0) {
        s.hasCurrent = true;
        s.current = m_all[static_cast<size_t>(r[static_cast<size_t>(sel)])];
        const std::string key = detailsKey(s.current);
        const auto d = m_details.find(key);
        if (d != m_details.end()) {
            s.detailsReady = true;
            s.details = d->second;
        }
        const auto e = m_detailErrors.find(key);
        if (e != m_detailErrors.end())
            s.detailsError = e->second;
    }
    s.sort = m_sort;
    s.searchMode = m_searchMode;
    s.category = m_category;
    s.categoryChooserOpen = m_categoryChooser;
    s.categories = m_categoryList;
    s.categoryIndex = m_categoryIndex;
    s.remoteView = m_remoteView && m_hasRemote;
    s.tab = m_tab;
    s.contentFocused = m_contentFocused;
    s.contentScroll = m_contentScroll;
    s.contentHeight = m_contentHeight;
    s.contentWidth = m_contentWidth;
    s.layout = m_layout;
    // Só com as abas na tela, as setas rolam a aba.
    if (m_layout == Layout::TabsOnly)
        s.contentFocused = true;
    s.log = m_log;
    s.status = m_status;
    s.busy = m_busy;
    s.sizing = m_sizing;
    s.error = m_error;
    s.totalPackages = static_cast<int>(m_all.size());
    s.outdatedCount = m_outdated;
    s.iconMode = m_iconMode;
    s.sortChooserOpen = m_sortChooser;
    s.sortChooserIndex = m_sortChooserIndex;
    s.dialog = m_dialog;
    s.themeId = m_themeId;
    s.findOpen = m_findOpen;
    s.findText = m_findText;
    if (m_findOpen && !m_findText.empty()) {
        const std::vector<int> found = findLines(tabText(s), m_findText);
        s.findCount = static_cast<int>(found.size());
        if (!found.empty()) {
            m_findIndex = ((m_findIndex % s.findCount) + s.findCount) % s.findCount;
            s.findMatchIndex = m_findIndex;
            s.findMatch = found[static_cast<size_t>(m_findIndex)];
        }
    }
    return s;
}

void App::setSort(model::SortSpec sort)
{
    m_sort = sort;
    m_settings.setValue("sort.column", m_sort.column);
    m_settings.setValue("sort.order", m_sort.ascending ? "asc" : "desc");
    m_settings.save();
}

void App::move(int delta)
{
    if (m_contentFocused || m_layout == Layout::TabsOnly) {
        scrollContent(delta);
        return;
    }
    const auto &r = rows();
    if (r.empty())
        return;
    int i = model::rowToSelect(m_all, r, m_selectedKey) + delta;
    i = std::max(0, std::min(i, static_cast<int>(r.size()) - 1));
    m_selectedKey = m_all[static_cast<size_t>(r[static_cast<size_t>(i)])].key();
}

void App::scrollContent(int delta)
{
    const int last = std::max(0, m_boxes.contentLines - m_contentHeight);
    m_contentScroll = std::max(0, std::min(m_contentScroll + delta, last));
}

void App::setTab(int tab)
{
    m_tab = tab;
    m_contentScroll = 0;
    if (tab == OutputTab && m_contentFocused)
        m_contentScroll = std::max(0, static_cast<int>(m_log.size()) - m_contentHeight);
}

void App::quit()
{
    // Nesta versão só há leitura: a saída nunca interrompe uma alteração.
    m_cancelSizes = true;
    if (m_screen)
        m_screen->Exit();
}

void App::actAbout()
{
    std::string brewVersion;
    try {
        brewVersion = m_mgr.version();
    } catch (const std::exception &) {
        brewVersion = "Homebrew ?";
    }
    showMessage(i18n::tr("About"), std::string("octobrew-tui ") + OCTOBREW_TUI_VERSION
                                        + "\n" + brewVersion + " · FTXUI"
                                        + "\n" + i18n::tr("Based on OctoPkg, © Alexandre Albuquerque Arnt")
                                        + "\n" + i18n::tr("and on octopkg-tui")
                                        + "\nGPL-2.0-or-later"
                                        + "\nhttps://github.com/cleberalves40/octobrew-tui");
}

// ---------------------------------------------------------------------------
// Preferências, temas e busca nas abas
// ---------------------------------------------------------------------------

void App::setSearchMode(model::SearchMode mode)
{
    // Como o menu Busca do OctoPkg; a escolha fica salva.
    m_searchMode = mode;
    m_placeholder = filterPlaceholder();
    m_settings.setValue("search", mode == model::SearchMode::Description ? "description" : "name");
    m_settings.save();
    m_status = mode == model::SearchMode::Description ? i18n::tr("Search by description (name and description).")
                                                      : i18n::tr("Search by name.");
}

void App::nextLayout()
{
    // Como o F10 (lista) e o F11 (abas) do OctoPkg, numa tecla só.
    switch (m_layout) {
    case Layout::Normal:
        m_layout = Layout::ListOnly;
        m_status = i18n::tr("Package list maximized (F11: tabs).");
        break;
    case Layout::ListOnly:
        m_layout = Layout::TabsOnly;
        m_status = i18n::tr("Tabs maximized (F11: back to both).");
        break;
    default:
        m_layout = Layout::Normal;
        m_status = i18n::tr("Package list and tabs.");
        break;
    }
}

void App::openCategoryChooser()
{
    // Os taps da visão atual (Instalados ou Repositório), com o atual em foco.
    m_categoryList = model::categories(m_all);
    m_categoryIndex = 0;
    for (size_t i = 0; i < m_categoryList.size(); ++i)
        if (m_categoryList[i].first == m_category)
            m_categoryIndex = static_cast<int>(i) + 1;
    m_categoryChooser = true;
}

void App::setCategory(const std::string &category)
{
    m_category = category;
    m_status = category.empty() ? i18n::tr("All taps.") : i18n::trf("Tap: %1", {category});
}

bool App::handleCategoryEvent(const Event &event)
{
    const int last = static_cast<int>(m_categoryList.size());
    if (event == Event::Escape || event == Event::F8) {
        m_categoryChooser = false;
    } else if (event == Event::ArrowUp) {
        m_categoryIndex = std::max(0, m_categoryIndex - 1);
    } else if (event == Event::ArrowDown) {
        m_categoryIndex = std::min(last, m_categoryIndex + 1);
    } else if (event == Event::PageUp) {
        m_categoryIndex = std::max(0, m_categoryIndex - 15);
    } else if (event == Event::PageDown) {
        m_categoryIndex = std::min(last, m_categoryIndex + 15);
    } else if (event == Event::Home) {
        m_categoryIndex = 0;
    } else if (event == Event::End) {
        m_categoryIndex = last;
    } else if (event == Event::Return) {
        setCategory(m_categoryIndex == 0 ? std::string()
                                         : m_categoryList[static_cast<size_t>(m_categoryIndex - 1)].first);
        m_categoryChooser = false;
    } else if (last > 0 && event.is_character() && event.character().size() == 1) {
        // Uma letra pula para o próximo tap que começa com ela.
        const char c = static_cast<char>(std::tolower(static_cast<unsigned char>(event.character()[0])));
        for (int step = 1; step <= last; ++step) {
            const int i = (m_categoryIndex - 1 + step) % last;
            if (!m_categoryList[static_cast<size_t>(i)].first.empty()
                && m_categoryList[static_cast<size_t>(i)].first[0] == c) {
                m_categoryIndex = i + 1;
                break;
            }
        }
    }
    return true;
}

std::string App::filterPlaceholder() const
{
    return m_searchMode == model::SearchMode::Description ? i18n::tr("Filter by description…")
                                                          : i18n::tr("Filter by name…");
}

void App::nextThemeKey()
{
    m_themeId = nextTheme(m_themeId).id;
    m_settings.setValue("theme", m_themeId);
    m_settings.save();
    const Theme t = theme(m_themeId);
    m_status = i18n::trf("Theme: %1", {t.system ? i18n::tr("System") : std::string(t.label)});
}

void App::actPreferences()
{
    // As mesmas preferências do octopkg-tui: tema, idioma e ícones.
    DialogField themeField;
    themeField.kind = DialogField::Choice;
    themeField.label = i18n::tr("Theme:");
    const auto &list = themes();
    for (size_t i = 0; i < list.size(); ++i) {
        themeField.choices.push_back(list[i].system ? i18n::tr("System (terminal colors)") : list[i].label);
        if (m_themeId == list[i].id)
            themeField.choice = static_cast<int>(i);
    }
    DialogField lang;
    lang.kind = DialogField::Choice;
    lang.label = i18n::tr("Language:");
    const std::vector<std::string> langIds{"system", "en_US", "pt_BR", "es_ES"};
    lang.choices = {i18n::tr("System default"), "English", "Português (Brasil)", "Español"};
    const std::string savedLang = m_settings.value("language", "system");
    for (size_t i = 0; i < langIds.size(); ++i)
        if (savedLang == langIds[i])
            lang.choice = static_cast<int>(i);
    DialogField iconsField;
    iconsField.kind = DialogField::Choice;
    iconsField.label = i18n::tr("Icons:");
    const std::vector<IconMode> modes{IconMode::Nerd, IconMode::Unicode, IconMode::Ascii};
    iconsField.choices = {"Nerd Fonts", "Unicode", "ASCII"};
    for (size_t i = 0; i < modes.size(); ++i)
        if (m_iconMode == modes[i])
            iconsField.choice = static_cast<int>(i);
    iconsField.hint = i18n::tr("Nerd Fonts need a Nerd Font in the terminal.");

    showForm(i18n::tr("Preferences"), std::string(), {themeField, lang, iconsField}, false,
             [this, langIds, modes](const std::vector<DialogField> &f) {
                 m_themeId = themes()[static_cast<size_t>(f[0].choice)].id;
                 const std::string language = langIds[static_cast<size_t>(f[1].choice)];
                 m_iconMode = modes[static_cast<size_t>(f[2].choice)];
                 if (language == "system")
                     i18n::initFromEnvironment();
                 else
                     i18n::setLocale(language);
                 m_placeholder = filterPlaceholder();   // o texto do campo segue o idioma
                 m_settings.setValue("theme", m_themeId);
                 m_settings.setValue("language", language);
                 m_settings.setValue("icons", iconModeName(m_iconMode));
                 m_status = m_settings.save() ? i18n::tr("Preferences saved.")
                                              : i18n::trf("Could not save %1", {m_settings.path()});
             });
    // O foco começa no primeiro campo, não no OK.
    m_dialog.focus = 0;
}

void App::openFind()
{
    // Como o Ctrl+F do OctoPkg: nas abas Arquivos, Saída e Ajuda.
    if (m_tab != FilesTab && m_tab != OutputTab && m_tab != HelpTab) {
        m_status = i18n::tr("Find works in the Files, Output and Help tabs.");
        return;
    }
    m_findOpen = true;
    m_findText.clear();
    m_findIndex = 0;
    m_contentFocused = false;
}

void App::findStep(int delta)
{
    m_findIndex += delta;
    // Rola até o resultado, no meio da aba se der.
    const ViewState s = viewState();
    if (s.findMatch >= 0)
        m_contentScroll = std::max(0, s.findMatch - std::max(1, m_contentHeight - 1) / 2);
}

bool App::handleFindEvent(const Event &event)
{
    const std::string &raw = event.input();
    if (event == Event::Escape || raw == kCtrlF) {
        m_findOpen = false;
        m_findText.clear();
        return true;
    }
    if (event == Event::Return || event == Event::F3 || event == Event::ArrowDown) {
        findStep(1);
        return true;
    }
    if (raw == kShiftF3 || event == Event::ArrowUp) {
        findStep(-1);
        return true;
    }
    if (event == Event::Backspace) {
        popUtf8(m_findText);
        m_findIndex = 0;
        findStep(0);
        return true;
    }
    if (event.is_character()) {
        m_findText += event.character();
        m_findIndex = 0;
        findStep(0);
        return true;
    }
    if (event.is_mouse())
        return handleMouse(event);
    return true;   // as demais teclas não saem da busca por engano
}

// ---------------------------------------------------------------------------
// Diálogos e menus
// ---------------------------------------------------------------------------

void App::showMessage(const std::string &title, const std::string &message)
{
    m_dialog = Dialog{};
    m_dialog.kind = Dialog::Message;
    m_dialog.title = title;
    m_dialog.message = message;
}

void App::showForm(const std::string &title, const std::string &message, std::vector<DialogField> fields,
                   bool danger, FormOp onAccept)
{
    m_dialog = Dialog{};
    m_dialog.kind = Dialog::Form;
    m_dialog.title = title;
    m_dialog.message = message;
    m_dialog.fields = std::move(fields);
    m_dialog.danger = danger;
    // O foco começa no botão: OK, ou Cancelar se for destrutivo.
    m_dialog.focus = static_cast<int>(m_dialog.fields.size()) + (danger ? 1 : 0);
    m_onForm = std::move(onAccept);
}

void App::openMenu(int menu)
{
    m_dialog = Dialog{};
    m_dialog.kind = Dialog::Menu;
    m_dialog.menus = {i18n::tr("File"), i18n::tr("Search"), i18n::tr("Help")};
    const bool byName = m_searchMode == model::SearchMode::Name;
    const auto sep = std::make_pair(std::string(), std::string());
    m_dialog.items = {
        {{i18n::tr("Refresh the package list"), "F5"},
         {i18n::tr("Switch between installed and repository packages"), "i"},
         sep,
         {i18n::tr("Preferences…"), "Alt+,"},
         {i18n::tr("Next theme"), "Alt+T"},
         sep,
         {i18n::tr("Exit"), "Ctrl+Q"}},
        {{std::string(byName ? "• " : "  ") + i18n::tr("By name"), "F7"},
         {std::string(byName ? "  " : "• ") + i18n::tr("By description"), "F7"},
         sep,
         {i18n::tr("Filter by tap…"), "F8"},
         {i18n::tr("All taps"), ""},
         sep,
         {i18n::tr("Find a package in the package list"), "Ctrl+L"},
         {i18n::tr("Find text in the tab"), "Ctrl+F"}},
        {{i18n::tr("Keyboard shortcuts"), "F1"}, {i18n::tr("About"), ""}},
    };
    m_menuActions = {
        {[this] { refresh(); }, [this] { toggleView(); }, nullptr, [this] { actPreferences(); },
         [this] { nextThemeKey(); }, nullptr, [this] { quit(); }},
        {[this] { setSearchMode(model::SearchMode::Name); },
         [this] { setSearchMode(model::SearchMode::Description); }, nullptr,
         [this] { openCategoryChooser(); }, [this] { setCategory(std::string()); }, nullptr,
         [this] { m_filterMode = true; m_contentFocused = false; }, [this] { openFind(); }},
        {[this] { setTab(HelpTab); }, [this] { actAbout(); }},
    };
    m_dialog.menu = std::max(0, std::min(menu, static_cast<int>(m_dialog.menus.size()) - 1));
}

void App::openSortChooser()
{
    m_sortChooser = true;
    m_sortChooserIndex = m_sort.column + 1;
}

bool App::handleDialogEvent(const Event &event)
{
    Dialog &d = m_dialog;
    switch (d.kind) {
    case Dialog::Message:
        if (event == Event::Return || event == Event::Escape || event == Event::Character(' '))
            d = Dialog{};
        return true;

    case Dialog::Confirm:   // sem confirmações nesta versão (só leitura)
        if (event == Event::Return || event == Event::Escape)
            d = Dialog{};
        return true;

    case Dialog::Form: {
        const int total = static_cast<int>(d.fields.size()) + 2;   // campos + OK + Cancelar
        if (event == Event::Escape) {
            d = Dialog{};
            return true;
        }
        if (event == Event::Tab || event == Event::ArrowDown) {
            d.focus = (d.focus + 1) % total;
            return true;
        }
        if (event == Event::TabReverse || event == Event::ArrowUp) {
            d.focus = (d.focus + total - 1) % total;
            return true;
        }
        if (event == Event::Return) {
            if (d.focus == total - 1) {   // Cancelar
                d = Dialog{};
                return true;
            }
            const std::vector<DialogField> fields = d.fields;
            const FormOp cb = m_onForm;
            d = Dialog{};
            if (cb)
                cb(fields);
            return true;
        }
        if (d.focus >= static_cast<int>(d.fields.size()))
            return true;
        DialogField &f = d.fields[static_cast<size_t>(d.focus)];
        switch (f.kind) {
        case DialogField::Text:
            if (event == Event::Backspace)
                popUtf8(f.text);
            else if (event.is_character())
                f.text += event.character();
            break;
        case DialogField::Check:
            if (event == Event::Character(' '))
                f.checked = !f.checked;
            break;
        case DialogField::Choice:
            if (!f.choices.empty() && (event == Event::ArrowRight || event == Event::Character(' ')))
                f.choice = (f.choice + 1) % static_cast<int>(f.choices.size());
            else if (!f.choices.empty() && event == Event::ArrowLeft)
                f.choice = (f.choice + static_cast<int>(f.choices.size()) - 1) % static_cast<int>(f.choices.size());
            break;
        }
        return true;
    }

    case Dialog::Menu: {
        const int menus = static_cast<int>(d.menus.size());
        const auto &items = d.items[static_cast<size_t>(d.menu)];
        const auto step = [&](int delta) {
            const int n = static_cast<int>(items.size());
            for (int i = 0; i < n; ++i) {
                d.item = (d.item + delta + n) % n;
                if (!items[static_cast<size_t>(d.item)].first.empty())
                    break;   // separadores não são selecionáveis
            }
        };
        if (event == Event::Escape || event == Event::F9 || event == Event::F10)
            d = Dialog{};
        else if (event == Event::ArrowLeft) {
            d.menu = (d.menu + menus - 1) % menus;
            d.item = 0;
        } else if (event == Event::ArrowRight) {
            d.menu = (d.menu + 1) % menus;
            d.item = 0;
        } else if (event == Event::ArrowUp)
            step(-1);
        else if (event == Event::ArrowDown)
            step(1);
        else if (event == Event::Return) {
            const Op action = m_menuActions[static_cast<size_t>(d.menu)][static_cast<size_t>(d.item)];
            d = Dialog{};
            if (action)
                action();
        }
        return true;
    }

    case Dialog::None:
        break;
    }
    return false;
}

bool App::handleMouse(const Event &event)
{
    Event copy = event;   // mouse() não é const em todas as versões do FTXUI
    Mouse mouse = copy.mouse();
    if (mouse.button == Mouse::WheelUp) {
        move(-1);
        return true;
    }
    if (mouse.button == Mouse::WheelDown) {
        move(1);
        return true;
    }
    if (mouse.button != Mouse::Left || mouse.motion != Mouse::Pressed)
        return true;
    for (int c = 0; c < model::ColumnCount; ++c)
        if (m_boxes.header[c].Contain(mouse.x, mouse.y)) {
            setSort(model::nextOnHeaderClick(m_sort, c));
            return true;
        }
    for (int t = 0; t < TabCount; ++t)
        if (m_boxes.tabs[t].Contain(mouse.x, mouse.y)) {
            setTab(t);
            return true;
        }
    const auto &r = rows();
    for (size_t i = 0; i < m_boxes.rows.size(); ++i) {
        const size_t index = static_cast<size_t>(m_offset) + i;
        if (index < r.size() && m_boxes.rows[i].Contain(mouse.x, mouse.y)) {
            m_selectedKey = m_all[static_cast<size_t>(r[index])].key();
            m_contentFocused = false;
            return true;
        }
    }
    return true;
}

bool App::handleEvent(const Event &event)
{
    // Diálogos e menus têm o teclado só para eles.
    if (m_dialog.kind != Dialog::None) {
        if (event.is_mouse())
            return true;
        return handleDialogEvent(event);
    }

    if (m_findOpen)
        return handleFindEvent(event);
    if (m_categoryChooser)
        return handleCategoryEvent(event);

    // Escolha da coluna de ordenação (F6 ou s).
    if (m_sortChooser) {
        if (event == Event::ArrowUp || event == Event::Character('k'))
            m_sortChooserIndex = std::max(0, m_sortChooserIndex - 1);
        else if (event == Event::ArrowDown || event == Event::Character('j'))
            m_sortChooserIndex = std::min(static_cast<int>(model::ColumnCount), m_sortChooserIndex + 1);
        else if (event == Event::Return) {
            const int column = m_sortChooserIndex - 1;
            if (column < 0)
                setSort({-1, true});
            else if (column == m_sort.column)
                setSort({column, !m_sort.ascending});
            else
                setSort({column, true});
            m_sortChooser = false;
        } else if (event == Event::Escape || event == Event::F6 || event == Event::Character('s'))
            m_sortChooser = false;
        return true;
    }

    const std::string &raw = event.input();

    // Campo de filtro: Esc, Enter ou Ctrl+P devolvem o foco à lista; o resto vai ao Input.
    if (m_filterMode) {
        if (event == Event::Escape || event == Event::Return || raw == kCtrlP) {
            m_filterMode = false;
            return true;
        }
        if (event.is_mouse())
            return handleMouse(event);
        return false;
    }

    if (event.is_mouse())
        return handleMouse(event);

    // No teclado do Mac, as teclas F pedem o fn: cada uma tem uma alternativa.
    if (event == Event::F5 || raw == kCtrlR) {
        refresh();
        return true;
    }
    if (event == Event::F6 || event == Event::Character('s')) {
        openSortChooser();
        return true;
    }
    if (raw == kCtrlL || event == Event::Character('/')) {
        m_filterMode = true;
        m_contentFocused = false;
        return true;
    }
    if (raw == kCtrlQ || event == Event::F10 || event == Event::Character('q')) {
        quit();
        return true;
    }
    if (raw == kAltI || event == Event::Character('i')) {
        toggleView();
        return true;
    }
    if (event == Event::F9 || raw == kAltA) {
        openMenu(FileMenu);
        return true;
    }
    if (raw == kAltB) {
        openMenu(SearchMenu);
        return true;
    }
    if (raw == kAltJ) {
        openMenu(HelpMenu);
        return true;
    }
    if (event == Event::F11 || raw == kAltZ) {
        nextLayout();
        return true;
    }
    if (event == Event::F8 || event == Event::Character('t')) {
        openCategoryChooser();
        return true;
    }
    if (event == Event::F7 || raw == kAltD) {
        setSearchMode(m_searchMode == model::SearchMode::Name ? model::SearchMode::Description
                                                              : model::SearchMode::Name);
        return true;
    }
    if (raw == kCtrlF) {
        openFind();
        return true;
    }
    if (raw == kAltT) {
        nextThemeKey();
        return true;
    }
    if (raw == kAltComma) {
        actPreferences();
        return true;
    }
    if (event == Event::F1 || event == Event::Character('?')) {
        setTab(HelpTab);
        return true;
    }
    for (char digit = '1'; digit <= '6'; ++digit) {
        const std::string d(1, digit);
        // Os números das abas do OctoPkg; Alt+n chega como ESC n. O 3 e o 5
        // são das abas Ações e Notícias, que ainda não existem.
        if (raw != d && raw != "\x1b" + d)
            continue;
        for (int t = 0; t < TabCount; ++t)
            if (tabKey(t) == digit)
                setTab(t);
        return true;
    }
    if (event == Event::Tab || event == Event::TabReverse) {
        m_contentFocused = !m_contentFocused;
        setTab(m_tab);
        return true;
    }
    if (raw == kCtrlP || event == Event::Escape) {
        m_contentFocused = false;
        return true;
    }
    if (event == Event::Return) {
        setTab(InfoTab);
        m_contentFocused = false;
        return true;
    }
    const int page = (m_contentFocused || m_layout == Layout::TabsOnly) ? m_contentHeight : m_tableHeight;
    if (event == Event::ArrowUp || event == Event::Character('k'))
        move(-1);
    else if (event == Event::ArrowDown || event == Event::Character('j'))
        move(1);
    else if (event == Event::PageUp)
        move(-std::max(1, page - 1));
    else if (event == Event::PageDown)
        move(std::max(1, page - 1));
    else if (event == Event::Home)
        move(-1000000);
    else if (event == Event::End)
        move(1000000);
    return true;
}

int App::run()
{
    auto screen = ScreenInteractive::Fullscreen();
    m_screen = &screen;

    InputOption option;
    option.multiline = false;
    // O texto de ajuda do campo acompanha o modo de busca (F7).
    m_input = Input(&m_filter, &m_placeholder, option);

    auto renderer = Renderer(m_input, [this] {
        // A primeira listagem parte da primeira renderização, já dentro do
        // laço: no FTXUI 6, uma tarefa enviada com Post() antes de Loop() é
        // descartada, e a lista nunca aparecia.
        if (!m_started) {
            m_started = true;
            refresh();
        }
        const ViewState state = viewState();
        Element filter = m_filterMode ? m_input->Render()
                                      : (m_filter.empty() ? text(m_placeholder) | dim
                                                          : text(m_filter));
        Element e = render(state, filter, &m_boxes);
        requestDetails();
        return e;
    });
    auto root = CatchEvent(renderer, [this](Event event) { return handleEvent(event); });

    screen.Loop(root);

    m_cancelSizes = true;
    joinWorkers();
    m_screen = nullptr;
    return 0;
}

} // namespace tui

#include "i18n.h"

#include <cstdlib>
#include <map>

namespace i18n {

namespace {

std::string g_locale = "en_US";

using Catalog = std::map<std::string, std::string>;

// { inglês (chave, o texto do OctoPkg quando existe), pt_BR, es_ES }
struct Entry {
    const char *en;
    const char *pt;
    const char *es;
};

const Entry kEntries[] = {
    {"Failed to start '%1': %2", "Falha ao iniciar '%1': %2", "No se pudo iniciar '%1': %2"},
    {"Command timed out (%1 ms): %2", "Comando excedeu o tempo limite (%1 ms): %2", "El comando excedió el tiempo límite (%1 ms): %2"},
    {"Process crashed: %1", "Processo terminou com crash: %1", "El proceso terminó abruptamente: %1"},
    {"Command failed with exit code %1", "Comando falhou com código %1", "El comando falló con código %1"},
    {"Invalid package name: %1", "Nome de pacote inválido: %1", "Nombre de paquete no válido: %1"},
    {"Package not found: %1", "Pacote não encontrado: %1", "Paquete no encontrado: %1"},

    {"Status", "Estado", "Estado"},
    {"Name", "Nome", "Nombre"},
    {"Version", "Versão", "Versión"},
    {"Size", "Tamanho", "Tamaño"},
    {"Installed on", "Instalado em", "Instalado el"},
    {"Installed", "Instalado", "Instalado"},
    {"Installed as a dependency", "Instalado como dependência", "Instalado como dependencia"},
    {"Outdated", "Desatualizado", "Desactualizado"},
    {"(version %1 is available)", "(a versão %1 está disponível)", "(la versión %1 está disponible)"},
    {"Newer than the repository", "Mais novo que o do repositório", "Más nuevo que el del repositorio"},
    {" (newer installed version is %1)", " (a versão instalada, mais nova, é %1)", " (la versión instalada, más nueva, es %1)"},
    {"Not installed", "Não instalado", "No instalado"},
    {"Installed packages", "Instalados", "Instalados"},
    {"Repository", "Repositório", "Repositorio"},

    {"Info", "Informação", "Información"},
    {"Files", "Arquivos", "Archivos"},
    {"Output", "Saída", "Salida"},
    {"Help", "Ajuda", "Ayuda"},
    {"Installed version", "Versão instalada", "Versión instalada"},
    {"Installed size", "Tamanho instalado", "Tamaño instalado"},
    {"Depends on", "Depende de", "Depende de"},
    {"Required by", "Requerido por", "Requerido por"},
    {"Select a package to see details.", "Selecione um pacote para ver os detalhes.", "Seleccione un paquete para ver los detalles."},
    {"Loading…", "Carregando…", "Cargando…"},
    {"Files are listed for installed packages only.", "Os arquivos só são listados para pacotes instalados.", "Los archivos solo se listan para paquetes instalados."},
    {"This package has too many files to be listed.", "Este pacote tem arquivos demais para listar.", "Este paquete tiene demasiados archivos para listar."},
    {"No files.", "Nenhum arquivo.", "Ningún archivo."},
    {"No packages.", "Nenhum pacote.", "Ningún paquete."},

    {"Ready.", "Pronto.", "Listo."},
    {"Listing installed packages…", "Listando os pacotes instalados…", "Listando los paquetes instalados…"},
    {"Listing repository packages…", "Listando os pacotes dos repositórios…", "Listando los paquetes de los repositorios…"},
    {"Outdated packages (%1):", "Pacotes desatualizados (%1):", "Paquetes desactualizados (%1):"},
    {"%1 package(s)", "%1 pacote(s)", "%1 paquete(s)"},
    {"%1 outdated", "%1 desatualizado(s)", "%1 desactualizado(s)"},
    {"ERROR: %1", "ERRO: %1", "ERROR: %1"},
    {"WARNING: %1", "AVISO: %1", "AVISO: %1"},
    {"Wait for the current operation to finish.", "Aguarde o fim da operação em andamento.", "Espere a que termine la operación en curso."},
    {"Filter by name…", "Filtrar por nome…", "Filtrar por nombre…"},
    {"Sort by", "Ordenar por", "Ordenar por"},

    {"Refresh", "Atualizar", "Actualizar"},
    {"Sort", "Ordenar", "Ordenar"},
    {"Exit", "Sair", "Salir"},
    {"Move", "Mover", "Mover"},
    {"Refresh the package list", "Atualizar a lista de pacotes", "Actualizar la lista de paquetes"},
    {"Find a package in the package list", "Buscar um pacote na lista", "Buscar un paquete en la lista"},
    {"Go back to the package list", "Voltar à lista de pacotes", "Volver a la lista de paquetes"},
    {"Switch between installed and repository packages", "Alternar entre os pacotes instalados e os do repositório", "Alternar entre los paquetes instalados y los del repositorio"},
    {"Show this help page", "Mostrar esta página de ajuda", "Mostrar esta página de ayuda"},
    {"Show the Info tab of the selected package", "Mostrar a aba Informação do pacote selecionado", "Mostrar la pestaña Información del paquete seleccionado"},
    {"Scroll the tab with the arrow keys", "Rolar a aba com as setas", "Desplazar la pestaña con las flechas"},
    {"Click a column header to sort", "Clique no cabeçalho de uma coluna para ordenar", "Haga clic en el encabezado de una columna para ordenar"},

    {"OK", "OK", "Aceptar"},
    {"Cancel", "Cancelar", "Cancelar"},
    {"Yes", "Sim", "Sí"},
    {"No", "Não", "No"},
    {"Tab / ↑ ↓: field   Space: check   ← →: choose   Enter: confirm   Esc: cancel", "Tab / ↑ ↓: campo   Espaço: marcar   ← →: escolher   Enter: confirmar   Esc: cancelar", "Tab / ↑ ↓: campo   Espacio: marcar   ← →: elegir   Enter: confirmar   Esc: cancelar"},
    {"File", "Arquivo", "Archivo"},
    {"Menu", "Menu", "Menú"},
    {"Keyboard shortcuts", "Atalhos de teclado", "Atajos de teclado"},
    {"About", "Sobre", "Acerca de"},
    {"Based on OctoPkg, © Alexandre Albuquerque Arnt", "Baseado no OctoPkg, © Alexandre Albuquerque Arnt", "Basado en OctoPkg, © Alexandre Albuquerque Arnt"},

    {"Preferences", "Preferências", "Preferencias"},
    {"Preferences…", "Preferências…", "Preferencias…"},
    {"Preferences: theme, language and icons", "Preferências: tema, idioma e ícones", "Preferencias: tema, idioma e iconos"},
    {"Preferences saved.", "Preferências salvas.", "Preferencias guardadas."},
    {"Could not save %1", "Não foi possível salvar %1", "No se pudo guardar %1"},
    {"Theme:", "Tema:", "Tema:"},
    {"Language:", "Idioma:", "Idioma:"},
    {"Icons:", "Ícones:", "Iconos:"},
    {"System", "Sistema", "Sistema"},
    {"System default", "Padrão do sistema", "Predeterminado del sistema"},
    {"System (terminal colors)", "Sistema (cores do terminal)", "Sistema (colores del terminal)"},
    {"Nerd Fonts need a Nerd Font in the terminal.", "Nerd Fonts exige uma fonte Nerd no terminal.", "Nerd Fonts requiere una fuente Nerd en el terminal."},
    {"Theme: %1", "Tema: %1", "Tema: %1"},
    {"Next theme", "Próximo tema", "Siguiente tema"},
    {"Find:", "Buscar:", "Buscar:"},
    {"Not found", "Não encontrado", "No encontrado"},
    {"Enter/F3: next  Shift+F3: previous  Esc: close", "Enter/F3: próximo  Shift+F3: anterior  Esc: fechar", "Enter/F3: siguiente  Shift+F3: anterior  Esc: cerrar"},

    {"Search", "Busca", "Búsqueda"},
    {"By name", "Por nome", "Por nombre"},
    {"By description", "Por descrição", "Por descripción"},
    {"Description", "Descrição", "Descripción"},
    {"Filter by description…", "Filtrar por descrição…", "Filtrar por descripción…"},
    {"Search by name.", "Busca por nome.", "Búsqueda por nombre."},
    {"Find text in the tab", "Buscar texto na aba", "Buscar texto en la pestaña"},

    {"Enter: choose   a-z: jump   Esc: close", "Enter: escolher   a-z: saltar   Esc: fechar", "Enter: elegir   a-z: saltar   Esc: cerrar"},

    {"Package list maximized (F11: tabs).", "Lista de pacotes maximizada (F11: abas).", "Lista de paquetes maximizada (F11: pestañas)."},
    {"Tabs maximized (F11: back to both).", "Abas maximizadas (F11: volta às duas).", "Pestañas maximizadas (F11: vuelve a las dos)."},
    {"Package list and tabs.", "Lista de pacotes e abas.", "Lista de paquetes y pestañas."},
    {"Maximize the package list, then the tabs, then back to both", "Maximizar a lista de pacotes, depois as abas, depois voltar às duas", "Maximizar la lista de paquetes, luego las pestañas, luego volver a las dos"},

    // Homebrew
    {"Could not read the output of brew info: %1", "Não foi possível ler a saída do brew info: %1", "No se pudo leer la salida de brew info: %1"},
    {"The Homebrew API cache was not found in %1. Run 'brew update' to download it.", "O cache da API do Homebrew não foi encontrado em %1. Execute 'brew update' para baixá-lo.", "No se encontró la caché de la API de Homebrew en %1. Ejecute 'brew update' para descargarla."},
    {"Could not read %1", "Não foi possível ler %1", "No se pudo leer %1"},
    {"Could not read %1: %2", "Não foi possível ler %1: %2", "No se pudo leer %1: %2"},
    {"Type", "Tipo", "Tipo"},
    {"Formula", "Fórmula", "Fórmula"},
    {"Cask", "Cask", "Cask"},
    {"Pinned", "Fixado", "Fijado"},
    {"Application", "Aplicativo", "Aplicación"},
    {"Tap", "Tap", "Tap"},
    {"Homepage", "Página", "Página"},
    {"License", "Licença", "Licencia"},
    {"Keg-only", "Keg-only", "Keg-only"},
    {"Auto-updates", "Atualiza sozinho", "Se actualiza solo"},
    {"yes (brew upgrade skips it)", "sim (o brew upgrade o ignora)", "sí (brew upgrade lo omite)"},
    {"Deprecated", "Obsoleto", "Obsoleto"},
    {"Disabled", "Desativado", "Desactivado"},
    {"Build dependencies", "Dependências de compilação", "Dependencias de compilación"},
    {"Conflicts with", "Conflita com", "Entra en conflicto con"},
    {"Artifacts", "Artefatos", "Artefactos"},
    {"Caveats", "Observações (caveats)", "Advertencias (caveats)"},
    {"Calculating sizes…", "Calculando os tamanhos…", "Calculando los tamaños…"},
    {"Installed/Repository", "Instalados/Repositório", "Instalados/Repositorio"},
    {"%1 package(s) installed: %2 formula(e) and %3 cask(s).", "%1 pacote(s) instalado(s): %2 fórmula(s) e %3 cask(s).", "%1 paquete(s) instalado(s): %2 fórmula(s) y %3 cask(s)."},
    {"%1 package(s) in the repository.", "%1 pacote(s) no repositório.", "%1 paquete(s) en el repositorio."},
    {"Disk usage of the installed packages: %1", "Espaço ocupado pelos pacotes instalados: %1", "Espacio ocupado por los paquetes instalados: %1"},
    {"and on octopkg-tui", "e no octopkg-tui", "y en octopkg-tui"},
    {"(brew order)", "(ordem do brew)", "(orden de brew)"},
    {"Search by name or by description", "Buscar por nome ou por descrição", "Buscar por nombre o por descripción"},
    {"Search by description (name and description).", "Busca por descrição (nome e descrição).", "Búsqueda por descripción (nombre y descripción)."},
    {"Filter by tap (homebrew/core, homebrew/cask...)", "Filtrar por tap (homebrew/core, homebrew/cask...)", "Filtrar por tap (homebrew/core, homebrew/cask...)"},
    {"Filter by tap…", "Filtrar por tap…", "Filtrar por tap…"},
    {"All taps", "Todos os taps", "Todos los taps"},
    {"All taps.", "Todos os taps.", "Todos los taps."},
    {"(all taps)", "(todos os taps)", "(todos los taps)"},
    {"Tap: %1", "Tap: %1", "Tap: %1"},
    {"Find text in the Files, Output and Help tabs (F3: next, Shift+F3: previous)", "Buscar texto nas abas Arquivos, Saída e Ajuda (F3: próximo, Shift+F3: anterior)", "Buscar texto en las pestañas Archivos, Salida y Ayuda (F3: siguiente, Shift+F3: anterior)"},
    {"Find works in the Files, Output and Help tabs.", "A busca funciona nas abas Arquivos, Saída e Ajuda.", "La búsqueda funciona en las pestañas Archivos, Salida y Ayuda."},
    {"Menus File, Search and Help", "Menus Arquivo, Busca e Ajuda", "Menús Archivo, Búsqueda y Ayuda"},
    {"Choose the sort column (again: reverse; (brew order): original order)", "Escolher a coluna de ordenação (de novo: inverter; (ordem do brew): ordem original)", "Elegir la columna de orden (otra vez: invertir; (orden de brew): orden original)"},
    {"Tabs Info, Files, Output and Help", "Abas Informação, Arquivos, Saída e Ajuda", "Pestañas Información, Archivos, Salida y Ayuda"},
    {"On a Mac keyboard, F keys may need fn; Alt is Option (enable \"Use Option as Meta key\").", "No teclado do Mac, as teclas F podem pedir o fn; Alt é o Option (ative \"Usar Option como tecla Meta\").", "En el teclado del Mac, las teclas F pueden requerir fn; Alt es Option (active \"Usar Option como tecla Meta\")."},
};

const Catalog &catalog(const std::string &loc)
{
    static const Catalog pt = [] {
        Catalog c;
        for (const auto &e : kEntries)
            c[e.en] = e.pt;
        return c;
    }();
    static const Catalog es = [] {
        Catalog c;
        for (const auto &e : kEntries)
            c[e.en] = e.es;
        return c;
    }();
    static const Catalog none;
    if (loc == "pt_BR")
        return pt;
    if (loc == "es_ES")
        return es;
    return none;
}

} // namespace

void initFromEnvironment()
{
    for (const char *var : {"LC_ALL", "LC_MESSAGES", "LANG"}) {
        const char *v = std::getenv(var);
        if (!v || !*v)
            continue;
        const std::string value(v);
        if (value.rfind("pt", 0) == 0)
            setLocale("pt_BR");
        else if (value.rfind("es", 0) == 0)
            setLocale("es_ES");
        else
            setLocale("en_US");
        return;
    }
    setLocale("en_US");
}

void setLocale(const std::string &loc)
{
    g_locale = (loc == "pt_BR" || loc == "es_ES") ? loc : "en_US";
}

std::string locale()
{
    return g_locale;
}

std::vector<std::string> availableLocales()
{
    return {"en_US", "pt_BR", "es_ES"};
}

std::string tr(const char *key)
{
    const Catalog &c = catalog(g_locale);
    const auto it = c.find(key);
    return it == c.end() ? std::string(key) : it->second;
}

std::string trf(const char *key, std::initializer_list<std::string> args)
{
    const std::string pattern = tr(key);
    const std::vector<std::string> values(args);
    std::string out;
    out.reserve(pattern.size());
    for (size_t i = 0; i < pattern.size(); ++i) {
        if (pattern[i] == '%' && i + 1 < pattern.size() && pattern[i + 1] >= '1' && pattern[i + 1] <= '9') {
            const size_t n = static_cast<size_t>(pattern[i + 1] - '1');
            if (n < values.size()) {
                out += values[n];
                ++i;
                continue;
            }
        }
        out += pattern[i];
    }
    return out;
}

} // namespace i18n

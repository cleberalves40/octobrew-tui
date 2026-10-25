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
    {"Choose the sort column (again: reverse; (brew order): original order)", "Escolher a coluna de ordenação (de novo: inverter; (ordem do brew): ordem original)", "Elegir la columna de orden (otra vez: invertir; (orden de brew): orden original)"},
    {"On a Mac keyboard, F keys may need fn; Alt is Option (enable \"Use Option as Meta key\").", "No teclado do Mac, as teclas F podem pedir o fn; Alt é o Option (ative \"Usar Option como tecla Meta\").", "En el teclado del Mac, las teclas F pueden requerir fn; Alt es Option (active \"Usar Option como tecla Meta\")."},

    // Transações (Fase 2)
    {"No packages selected.", "Nenhum pacote selecionado.", "Ningún paquete seleccionado."},
    {"%1 cannot be removed.", "%1 não pode ser removido.", "%1 no se puede eliminar."},
    {"Actions", "Ações", "Acciones"},
    {"Mark the selected package for installation", "Marcar o pacote selecionado para instalar", "Marcar el paquete seleccionado para instalar"},
    {"Mark the selected package for removal", "Marcar o pacote selecionado para remover", "Marcar el paquete seleccionado para eliminar"},
    {"Apply: install and remove the marked packages", "Aplicar: instalar e remover os pacotes marcados", "Aplicar: instalar y eliminar los paquetes marcados"},
    {"Cancel: clear the marks", "Cancelar: desfazer as marcas", "Cancelar: deshacer las marcas"},
    {"Menus File, Search, Actions and Help", "Menus Arquivo, Busca, Ações e Ajuda", "Menús Archivo, Búsqueda, Acciones y Ayuda"},
    {"No packages marked. Use + to install and - to remove.", "Nenhum pacote marcado. Use + para instalar e - para remover.", "Ningún paquete marcado. Use + para instalar y - para eliminar."},
    {"To be installed", "A instalar", "A instalar"},
    {"To be removed", "A remover", "A eliminar"},
    {"Alt+M: apply   Ctrl+E: cancel", "Alt+M: aplicar   Ctrl+E: cancelar", "Alt+M: aplicar   Ctrl+E: cancelar"},
    {"Mark", "Marcar", "Marcar"},
    {"Apply", "Aplicar", "Aplicar"},
    {"… and %1 more (see the Output tab)", "… e mais %1 (veja a aba Saída)", "… y %1 más (vea la pestaña Salida)"},
    {"Only installed packages can be removed.", "Só pacotes instalados podem ser removidos.", "Solo se pueden eliminar paquetes instalados."},
    {"Error", "Erro", "Error"},
    {"Collecting transaction data…", "Coletando os dados da transação…", "Recopilando los datos de la transacción…"},
    {"The following package needs to be retrieved", "O seguinte pacote precisa ser obtido", "El siguiente paquete debe obtenerse"},
    {"The following %1 packages need to be retrieved", "Os seguintes %1 pacotes precisam ser obtidos", "Los siguientes %1 paquetes deben obtenerse"},
    {"The following package will be removed", "O seguinte pacote será removido", "El siguiente paquete será eliminado"},
    {"The following %1 packages will be removed", "Os seguintes %1 pacotes serão removidos", "Los siguientes %1 paquetes serán eliminados"},
    {"Nothing to do: the marked packages are already in this state.", "Nada a fazer: os pacotes marcados já estão nesse estado.", "Nada que hacer: los paquetes marcados ya están en ese estado."},
    {"Confirm?", "Confirma?", "¿Confirma?"},
    {"Transaction:", "Transação:", "Transacción:"},
    {"Warning", "Aviso", "Aviso"},
    {"Confirmation", "Confirmação", "Confirmación"},
    {"Removing selected packages...", "Removendo os pacotes selecionados...", "Eliminando los paquetes seleccionados..."},
    {"Installing selected packages...", "Instalando os pacotes selecionados...", "Instalando los paquetes seleccionados..."},
    {"Installed: %1", "Instalados: %1", "Instalados: %1"},
    {"Removed: %1", "Removidos: %1", "Eliminados: %1"},
    {"Command finished OK!", "Comando concluído com sucesso!", "¡Comando terminado correctamente!"},
    {"Details in the Output tab.", "Detalhes na aba Saída.", "Detalles en la pestaña Salida."},
    {"System upgrade", "Atualização do sistema", "Actualización del sistema"},
    {"There are no new updates available!", "Não há novas atualizações disponíveis!", "¡No hay nuevas actualizaciones disponibles!"},
    {"Installing updates...", "Instalando as atualizações...", "Instalando las actualizaciones..."},
    {"Clean the local cache", "Limpar o cache local", "Limpiar la caché local"},
    {"Cleaning the local cache...", "Limpando o cache local...", "Limpiando la caché local..."},
    {"Select an installed package.", "Selecione um pacote instalado.", "Seleccione un paquete instalado."},
    {"Running: %1", "Executando: %1", "Ejecutando: %1"},
    {"Install updates", "Instalar as atualizações", "Instalar las actualizaciones"},
    {"Mark for installation", "Marcar para instalar", "Marcar para instalar"},
    {"Mark for removal", "Marcar para remover", "Marcar para eliminar"},
    {"This cask needs sudo: run 'sudo -v' in the terminal and try again.", "Este cask precisa do sudo: execute 'sudo -v' no terminal e tente de novo.", "Este cask necesita sudo: ejecute 'sudo -v' en el terminal e inténtelo de nuevo."},
    {"Update Homebrew and the package list (brew update)", "Atualizar o Homebrew e a lista de pacotes (brew update)", "Actualizar Homebrew y la lista de paquetes (brew update)"},
    {"Install updates (brew upgrade)", "Instalar as atualizações (brew upgrade)", "Instalar las actualizaciones (brew upgrade)"},
    {"%1 is pinned.", "%1 está fixado.", "%1 está fijado."},
    {"%1 is already installed and up to date.", "%1 já está instalado e atualizado.", "%1 ya está instalado y actualizado."},
    {"Save a snapshot (Brewfile) before the transaction", "Salvar um snapshot (Brewfile) antes da transação", "Guardar un snapshot (Brewfile) antes de la transacción"},
    {"File: %1", "Arquivo: %1", "Archivo: %1"},
    {"%1 depends on a package marked for removal and cannot be removed.", "%1 depende de um pacote marcado para remover e não pode ser removido.", "%1 depende de un paquete marcado para eliminar y no se puede eliminar."},
    {"(%1 of them depend on the marked packages)", "(%1 deles dependem dos pacotes marcados)", "(%1 de ellos dependen de los paquetes marcados)"},
    {"Needs sudo: %1", "Precisa do sudo: %1", "Necesita sudo: %1"},
    {"The screen will be suspended while sudo asks for your password.", "A tela será suspensa enquanto o sudo pede a sua senha.", "La pantalla se suspenderá mientras sudo pide su contraseña."},
    {"Snapshot: %1", "Snapshot: %1", "Snapshot: %1"},
    {"Pinned packages are not upgraded.", "Pacotes fixados não são atualizados.", "Los paquetes fijados no se actualizan."},
    {"Updating Homebrew and the package list…", "Atualizando o Homebrew e a lista de pacotes…", "Actualizando Homebrew y la lista de paquetes…"},
    {"Nothing to clean.", "Nada a limpar.", "Nada que limpiar."},
    {"brew cleanup would remove %1 item(s):", "O brew cleanup removeria %1 item(ns):", "brew cleanup eliminaría %1 elemento(s):"},
    {"brew cleanup would remove %1 item(s) (old versions and downloads; see the Output tab).", "O brew cleanup removeria %1 item(ns) (versões antigas e downloads; veja a aba Saída).", "brew cleanup eliminaría %1 elemento(s) (versiones antiguas y descargas; vea la pestaña Salida)."},
    {"It would free approximately %1.", "Liberaria aproximadamente %1.", "Liberaría aproximadamente %1."},
    {"Remove unneeded dependencies", "Remover dependências desnecessárias", "Eliminar dependencias innecesarias"},
    {"There are no unneeded dependencies.", "Não há dependências desnecessárias.", "No hay dependencias innecesarias."},
    {"They were installed as dependencies and nothing uses them anymore.", "Foram instalados como dependência e nada mais os usa.", "Se instalaron como dependencia y ya nada los usa."},
    {"Removing unneeded dependencies…", "Removendo as dependências desnecessárias…", "Eliminando las dependencias innecesarias…"},
    {"%1 is not pinned.", "%1 não está fixado.", "%1 no está fijado."},
    {"Pinning…", "Fixando…", "Fijando…"},
    {"Unpinning…", "Liberando…", "Liberando…"},
    {"Restore a snapshot", "Restaurar um snapshot", "Restaurar un snapshot"},
    {"No snapshots in %1 yet.", "Ainda não há snapshots em %1.", "Todavía no hay snapshots en %1."},
    {"Snapshot:", "Snapshot:", "Snapshot:"},
    {"Installs what is missing from it; nothing is removed or upgraded.", "Instala o que falta dele; nada é removido nem atualizado.", "Instala lo que falta de él; no se elimina ni se actualiza nada."},
    {"Snapshots in %1", "Snapshots em %1", "Snapshots en %1"},
    {"Restoring the snapshot…", "Restaurando o snapshot…", "Restaurando el snapshot…"},
    {"Restored: %1", "Restaurado: %1", "Restaurado: %1"},
    {"sudo may ask for your password.", "O sudo pode pedir a sua senha.", "sudo puede pedir su contraseña."},
    {"Update Homebrew", "Atualizar o Homebrew", "Actualizar Homebrew"},
    {"Restore a snapshot…", "Restaurar um snapshot…", "Restaurar un snapshot…"},
    {"Pin", "Fixar", "Fijar"},
    {"Unpin", "Liberar", "Liberar"},

    // Notícias (Fase 3)
    {"The news feed could not be read.", "Não foi possível ler o feed de notícias.", "No se pudo leer el feed de noticias."},
    {"News", "Notícias", "Noticias"},
    {"Find text in the Files, Output, News and Help tabs (F3: next, Shift+F3: previous)", "Buscar texto nas abas Arquivos, Saída, Notícias e Ajuda (F3: próximo, Shift+F3: anterior)", "Buscar texto en las pestañas Archivos, Salida, Noticias y Ayuda (F3: siguiente, Shift+F3: anterior)"},
    {"Get the latest Homebrew news (News tab)", "Buscar as últimas notícias do Homebrew (aba Notícias)", "Buscar las últimas noticias de Homebrew (pestaña Noticias)"},
    {"Tabs Info, Files, Actions, Output, News and Help", "Abas Informação, Arquivos, Ações, Saída, Notícias e Ajuda", "Pestañas Información, Archivos, Acciones, Salida, Noticias y Ayuda"},
    {"%1 news", "Notícias do %1", "Noticias de %1"},
    {"Searching for %1 latest news...", "Buscando as últimas notícias do %1...", "Buscando las últimas noticias de %1..."},
    {"(showing the saved news)", "(mostrando as notícias salvas)", "(mostrando las noticias guardadas)"},
    {"No news yet. Ctrl+G gets the latest news.", "Nenhuma notícia ainda. Ctrl+G busca as últimas.", "Todavía no hay noticias. Ctrl+G busca las últimas."},
    {"Find works in the Files, Output, News and Help tabs.", "A busca funciona nas abas Arquivos, Saída, Notícias e Ajuda.", "La búsqueda funciona en las pestañas Archivos, Salida, Noticias y Ayuda."},
    {"%1 news: %2 item(s).", "Notícias do %1: %2 item(ns).", "Noticias de %1: %2 elemento(s)."},
    {"Get the latest news", "Buscar as últimas notícias", "Buscar las últimas noticias"},
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

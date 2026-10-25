# Atalhos: OctoPkg → octopkg-tui → octobrew-tui

O octobrew-tui mantém os atalhos do octopkg-tui, que por sua vez mantém os do
OctoPkg 0.4.2 sempre que o terminal os entrega ao programa. No macOS, há duas
diferenças:

- as teclas F1–F12 do teclado do Mac pedem o **fn** (a menos que "Usar as teclas F1, F2
  etc. como teclas de função padrão" esteja ativado). Por isso cada uma tem uma
  alternativa sem fn;
- <kbd>Alt+tecla</kbd> é <kbd>Option+tecla</kbd> e só chega ao programa com "Usar Option
  como tecla Meta" ativado (Terminal → Ajustes → Perfis → Teclado; no iTerm2, *Left
  Option key: Esc+*).

Como no octopkg-tui: o terminal envia <kbd>Ctrl+I</kbd> como <kbd>Tab</kbd> e
<kbd>Ctrl+M</kbd> como <kbd>Enter</kbd>, e <kbd>Ctrl+Shift+letra</kbd> chega igual a
<kbd>Ctrl+letra</kbd>.

| Ação | OctoPkg | octobrew-tui | Sem fn (Mac) | Disponível |
|:---|:---|:---|:---|:---|
| Abas Informação / Arquivos / Saída / Ajuda | Alt+1 / Alt+2 / Alt+4 / Alt+6 | **1 2 4 6** ou **Alt+1 Alt+2 Alt+4 Alt+6** | — | v0.1.0 |
| Ajuda | F1 | **F1** | **?** | v0.1.0 |
| Buscar um pacote na lista | Ctrl+L | **Ctrl+L** ou **/** | — | v0.1.0 |
| Buscar por nome / por descrição | menu Busca | **F7**; menu Busca (**Alt+B**) | **Alt+D** | v0.1.0 |
| Filtrar por tap | — | **F8**; menu Busca | **t** | v0.1.0 |
| Maximizar a lista / as abas | F10 / F11 | **F11**, em ciclo | **Alt+Z** | v0.1.0 |
| Voltar à lista de pacotes | Ctrl+P | **Ctrl+P** ou **Esc** | — | v0.1.0 |
| Instalados / repositório | Ctrl+I | **i** ou **Alt+I** | — | v0.1.0 |
| Aba Informação do selecionado | Enter | **Enter** | — | v0.1.0 |
| Ordenar | clique no cabeçalho | clique no cabeçalho ou **F6** | **s** | v0.1.0 |
| Atualizar a lista | — | **F5** | **Ctrl+R** | v0.1.0 |
| Rolar a aba atual | — (mouse) | **Tab**, depois as setas, PgUp, PgDn | — | v0.1.0 |
| Menus | barra de menus | **F9** ou **Alt+A** (Arquivo), **Alt+B** (Busca), **Alt+J** (Ajuda) | **Alt+A** | v0.1.0 |
| Buscar texto nas abas Arquivos, Saída, Notícias e Ajuda | Ctrl+F (F3 / Shift+F3) | **Ctrl+F**; **Enter**/**F3** e **Shift+F3** (ou ↓ ↑) | ↓ ↑ | v0.1.0 |
| Preferências / próximo tema | — | **Alt+,** / **Alt+T** | — | v0.1.0 |
| Sair | Ctrl+Q | **Ctrl+Q**, **F10** ou **q** | **q** | v0.1.0 |
| Marcar para instalar / remover | + / − | **+** / **−**, ou **Insert** / **Delete** | — | v0.2.0 |
| Aba Ações | Alt+3 | **3** ou **Alt+3** | — | v0.2.0 |
| Aplicar | Ctrl+M | **Alt+M** (Ctrl+M é Enter) | — | v0.2.0 |
| Cancelar (desfazer as marcas) | Ctrl+E | **Ctrl+E** | — | v0.2.0 |
| Procurar atualizações | Ctrl+K | **Ctrl+K** (`brew update`) | — | v0.2.0 |
| Instalar as atualizações | Ctrl+U | **Ctrl+U** (`brew upgrade`) | — | v0.2.0 |
| Limpar o cache | Ctrl+Shift+C | menu Arquivo (`brew cleanup`), como no octopkg-tui | — | v0.2.0 |
| Remover dependências desnecessárias | — | menu Arquivo (`brew autoremove`) | — | v0.2.0 |
| Bloquear / desbloquear | menu de contexto | menu Ações: Fixar / Liberar (`brew pin` / `unpin`) | — | v0.2.0 |
| Restaurar um snapshot | — (Boot Environments) | menu Arquivo | — | v0.2.0 |
| Menu Ações | barra de menus | **Alt+C** | — | v0.2.0 |
| Aba Notícias / buscar as últimas notícias | Alt+5 / Ctrl+G | **5** ou **Alt+5** / **Ctrl+G** | — | v0.3.0 |

Nos diálogos: <kbd>Tab</kbd> ou as setas mudam o campo, <kbd>Espaço</kbd> marca a
opção do snapshot, <kbd>Enter</kbd> confirma e <kbd>Esc</kbd> cancela; nas confirmações,
<kbd>y</kbd>/<kbd>s</kbd> e <kbd>n</kbd> também valem.

No seletor de coluna (F6/s), no de taps (F8/t) e nos diálogos, as teclas são as do
octopkg-tui: setas, <kbd>Enter</kbd> escolhe, <kbd>Esc</kbd> fecha; nos taps, uma letra
salta para o próximo que começa com ela.

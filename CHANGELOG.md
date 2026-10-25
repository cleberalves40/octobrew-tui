# Changelog

Todas as mudanças relevantes do octobrew-tui são registradas aqui.
O formato segue o [Keep a Changelog](https://keepachangelog.com/pt-BR/1.1.0/)
e o projeto usa [versionamento semântico](https://semver.org/lang/pt-BR/),
com tags no formato `vX.Y.Z`.

## [0.2.0] - 2026-10-25

Transações: marcar, instalar, remover, atualizar, limpar e fixar, como no OctoPkg
(Fase 2 do [roadmap](docs/ROADMAP.md)).

### Adicionado
- Marcas do OctoPkg: **+** (ou Insert) para instalar e **-** (ou Delete) para remover;
  de novo, desmarca. Um instalado só é marcado para instalar se está desatualizado (o
  `brew install` o atualiza); pacotes fixados não são marcados; o `ftxui`, de que o
  próprio programa depende, e o `octobrew-tui` não podem ser removidos.
- Aba **Ações** (3), com os pacotes a instalar e a remover; **Ctrl+E** desfaz as marcas;
  a barra de status mostra quantos estão marcados.
- **Alt+M** aplica. Antes, a prévia: `brew install --dry-run` (fórmulas e casks em
  chamadas separadas, que o brew não aceita juntos) e, para a remoção, os instalados
  que dependem dos marcados, direta ou indiretamente. O brew não remove o que outros
  usam, então eles vão juntos, como no `pkg remove -R` do OctoPkg, e o diálogo vira
  **Aviso**, em vermelho, com o foco em Cancelar. Um dependente fixado ou protegido
  impede a remoção. Depois, `brew uninstall` (os casks primeiro) e `brew install`.
- **Ctrl+U** instala as atualizações (`brew upgrade`), com a prévia do
  `brew upgrade --dry-run`; os fixados ficam de fora, e os avisos do brew (casks
  desativados) vão para a aba Saída. **Ctrl+K** atualiza o Homebrew e a lista
  (`brew update`).
- Arquivo → Limpar o cache local (`brew cleanup --prune=all`), com a prévia do que sai
  e do espaço liberado; Arquivo → Remover dependências desnecessárias
  (`brew autoremove`), com a lista; Ações → Fixar e Liberar (`brew pin`/`unpin`, para
  fórmulas e casks).
- **Snapshot em Brewfile** antes da transação, no lugar do Boot Environment do
  OctoPkg: `brew bundle dump` em `~/.config/octobrew-tui/snapshots/AAMMDD-hhmmss.Brewfile`,
  marcado ao instalar e atualizar e desmarcado ao remover. Sem shell: se o snapshot
  falhar, nada muda. Arquivo → Restaurar um snapshot… (`brew bundle install
  --no-upgrade`) instala o que falta dele.
- Casks que pedem o **sudo** (instaladores `.pkg`, passos de sistema na desinstalação)
  são avisados no diálogo. Se o sudo não tiver a senha em cache, a tela é suspensa
  enquanto ele a pede, como o doas no octopkg-tui; no resto, a operação roda em
  segundo plano, com a saída ao vivo na aba Saída.
- Menu **Ações** (**Alt+C**); a barra de teclas mostra **+/-** e **M-m**.
- Testes: a leitura das prévias do brew 7 (install, upgrade, autoremove, cleanup), os
  dependentes, os casks que pedem o sudo, os snapshots, cada operação contra o brew
  falso (argumentos e ambiente), as marcas, os diálogos e os menus dirigidos por
  teclas, e as prévias contra o brew real. Com `--transactions`
  (`tests/macos-e2e.sh`, no CI), a integração instala, fixa e remove o figlet.

### Alterado
- As operações rodam sem auto-update (fazem o que a prévia mostrou), sem a pergunta
  do brew 7 (`--no-ask`, `HOMEBREW_NO_ASK`) e sem o autoremove automático do
  `uninstall` e do `cleanup` (`HOMEBREW_NO_AUTOREMOVE`), que levaria pacotes que não
  estavam na prévia. Sem o terminal, um sudo que precise de senha falha na hora
  (`SUDO_ASKPASS=/usr/bin/false`) em vez de disputar a tela.
- Sair (q, F10, Ctrl+Q) é recusado durante uma operação.

## [0.1.0] - 2026-10-25

Primeira versão: somente leitura (Fases 0 e 1 do [roadmap](docs/ROADMAP.md)).

### Adicionado
- Repositório a partir do octopkg-tui v0.7.2. Sem mudança: a execução de processos
  sem shell, o i18n (en_US, pt_BR, es_ES), as preferências, os 15 temas, os ícones,
  os diálogos, os menus, a busca nas abas (Ctrl+F), a busca por descrição (F7) e as
  disposições do F11.
- Núcleo do Homebrew (`src/core/brew.*`), no lugar do `pkg.*`:
  - instalados por `brew info --json=v2 --installed`: fórmulas e casks numa chamada,
    com os desatualizados, os fixados (`brew pin`, que fazem as vezes do bloqueio do
    pkg), as dependências (`installed_on_request`) e quantos instalados dependem de
    cada um;
  - repositório lido direto do cache da API do Homebrew, sem rede e sem Ruby: o
    `packages.<tag>.jws.json` do brew 5 ou mais novo e o `formula.jws.json` e o
    `cask.jws.json` do brew 4. São cerca de 16 mil pacotes em 0,3 s;
  - detalhes por `brew info --json=v2 --formula|--cask`, e arquivos por `brew list`;
  - todas as consultas com `HOMEBREW_NO_AUTO_UPDATE=1`, sem cores nem emojis: uma
    consulta nunca vira um `brew update`.
- Leitor de JSON próprio (`src/core/json.*`), em C++17 puro, como o resto do núcleo.
- Coluna **Tipo** (fórmula ou cask). Fórmula e cask de mesmo nome são linhas
  diferentes, e a seleção os distingue.
- Aba Informação com o tap, a página, a licença, o keg-only, a atualização automática
  dos casks, os avisos de obsoleto ou desativado, as dependências de execução e de
  compilação, o "Requerido por", os conflitos, os artifacts dos casks e os
  **caveats**, quebrados na largura da aba.
- Tamanho em disco calculado em segundo plano depois da lista. Para as fórmulas, todas
  as versões no Cellar; para os casks, o Caskroom e o aplicativo em `/Applications`.
  Uma nova listagem ou a saída interrompem o cálculo.
- **F8** filtra por tap (homebrew/core, homebrew/cask, taps de terceiros), no lugar da
  categoria do port.
- Alternativas às teclas F, que no Mac pedem o fn: `?` (F1), `s` (F6), `Alt+D` (F7) e
  `t` (F8).
- Testes: `test_core` (JSON, brew 7, cache da API, brew falso, tamanhos),
  `test_model`, `test_view` (inclusive o caminho completo com o brew falso),
  `test_integration` (Homebrew real, só leitura) e `tests/tui-smoke.sh`.
- CI no macOS (Homebrew do runner) e no Ubuntu (FTXUI 6.1.9, ASan e UBSan).

### Ainda não
- Abas Ações (3) e Notícias (5); marcar, instalar, remover, atualizar e fixar: Fases 2
  e 3.

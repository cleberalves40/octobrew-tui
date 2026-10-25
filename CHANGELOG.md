# Changelog

Todas as mudanças relevantes do octobrew-tui são registradas aqui.
O formato segue o [Keep a Changelog](https://keepachangelog.com/pt-BR/1.1.0/)
e o projeto usa [versionamento semântico](https://semver.org/lang/pt-BR/),
com tags no formato `vX.Y.Z`.

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

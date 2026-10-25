# octobrew-tui

[![License: GPL v2+](https://img.shields.io/badge/License-GPL%20v2%2B-blue.svg)](LICENSE)

Interface de terminal (TUI) para o [Homebrew](https://brew.sh), o gerenciador de pacotes
do macOS (e do Linux). É a irmã do [octopkg-tui](https://github.com/cleberalves40/octopkg-tui),
a interface de terminal do [OctoPkg](https://github.com/aarnt/octopkg) para o `pkg(8)` do
FreeBSD: as mesmas abas, os mesmos atalhos, os mesmos temas e ícones
[Nerd Fonts](https://www.nerdfonts.com/), agora com fórmulas e casks.

```
 OctoBrew — Homebrew [# Installed packages]                 / Filter by name…
────────────────────────────────────────────────────────────────────────────────────
   Name ^                       Type     Version                Size  Installed on
────────────────────────────────────────────────────────────────────────────────────
 I alacritty                    Cask     0.17.0            13.97 MiB  2026-04-14 22:13
 d brotli                       Formula  1.2.0              1.67 MiB  2026-10-19 09:12
 U curl                         Formula  8.9.1              4.72 MiB  2026-10-19 09:12
 P wget                         Formula  1.24.5             4.01 MiB  2026-09-02 15:40
────────────────────────────────────────────────────────────────────────────────────
 1 i Info   2 f Files   4 > Output   6 ? Help
curl  Get a file from an HTTP, HTTPS or FTP server
Status:             Outdated (version 8.10.0 is available)
Type:               Formula
Installed version:  8.9.1
Version:            8.10.0
Tap:                homebrew/core
────────────────────────────────────────────────────────────────────────────────────
Ready.                                   Theme: System  4 package(s)  2 outdated
F1Help  F5Refresh  F6Sort  F8Tap  F9Menu  F10Exit  iInstalled/Repository
```

(com `--icons=ascii`: **I** instalado, **d** dependência, **U** desatualizado, **N** mais
novo que o repositório, **P** fixado com `brew pin`, **-** não instalado.)

## Estado

**v0.1.0 — somente leitura** (a Fase 1 do [roadmap](docs/ROADMAP.md)). O programa:

- lista as fórmulas e os casks instalados e os do repositório;
- mostra os detalhes do pacote selecionado, incluindo os *caveats*;
- mostra os arquivos de cada instalado e o espaço que ele ocupa em disco;
- busca por nome ou por descrição (**F7**) e filtra por tap (**F8**);
- ordena por qualquer coluna (**F6**) e maximiza a lista ou as abas (**F11**);
- tem os 15 temas e as Preferências do octopkg-tui.

Instalar, remover, atualizar e fixar pacotes é a Fase 2. Veja o [CHANGELOG](CHANGELOG.md).

## Requisitos

```sh
brew install ftxui cmake ninja
brew install --cask font-jetbrains-mono-nerd-font   # ícones; opcional
```

Para os ícones, configure o terminal com a fonte **JetBrainsMono Nerd Font Mono**. Sem
uma fonte Nerd, use `--icons=unicode` ou `--icons=ascii`.

Sem o Qt, sem X11 e sem Ruby próprio: só o FTXUI e o brew do sistema. Funciona também
por SSH.

## Compilação

```sh
cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/octobrew-tui
```

Opções: `--remote` (começa na visão Repositório), `--icons=nerd|unicode|ascii`,
`--lang=en_US|pt_BR|es_ES`, `--theme=TEMA`, `--config=ARQUIVO`, `--brew=EXECUTÁVEL`,
`--version`. As preferências ficam em `~/.config/octobrew-tui/config`: a ordem da tabela,
o modo de busca, o tema, o idioma e os ícones (pelo diálogo de Preferências, **Alt+,**).

## Como ele lê o Homebrew

| O quê | De onde | Custo |
|:---|:---|:---|
| Instalados (fórmulas e casks, com desatualizados, pins e dependências) | `brew info --json=v2 --installed`, uma chamada | ~2 s (o Ruby do brew) |
| Repositório (~16 mil fórmulas e casks) | o cache da API do Homebrew, lido direto (`$(brew --cache)/api/internal/packages.*.jws.json`; no brew 4, `formula.jws.json` e `cask.jws.json`), sem rede e sem Ruby | ~0,3 s |
| Detalhes do selecionado | `brew info --json=v2 --formula\|--cask NOME` | ~0,5–1 s, em segundo plano e em cache |
| Arquivos (aba 2) | `brew list --formula --verbose` / `brew list --cask` | idem |
| Tamanho em disco | o Cellar (todas as versões) e o Caskroom, mais o aplicativo em `/Applications`, medidos pelo próprio programa | segundo plano, depois da lista |

Todas as consultas rodam com `HOMEBREW_NO_AUTO_UPDATE=1`, `HOMEBREW_NO_ENV_HINTS=1`,
`HOMEBREW_NO_COLOR=1` e `HOMEBREW_NO_EMOJI=1`, sem shell. Uma consulta nunca vira um
`brew update` nem baixa a API. Se o cache da API não existir (por exemplo, com
`HOMEBREW_NO_INSTALL_FROM_API`), a visão Repositório avisa e pede um `brew update`.

Fórmulas que saíram do homebrew-core continuam na lista de instalados, como pacotes
que só existem localmente. O mesmo vale para as fórmulas de taps de terceiros, que a API
não tem. Fórmula e cask de mesmo nome (`docker`) são duas linhas.

O Homebrew não roda como root, e o octobrew-tui também não.

## Teclado e mouse

As teclas são as do octopkg-tui. No teclado do Mac, as teclas F podem pedir o **fn**,
então cada uma tem uma alternativa:

| Tecla | Ação |
|:---|:---|
| <kbd>F5</kbd> / <kbd>Ctrl+R</kbd> | Atualizar a lista |
| <kbd>↑</kbd> <kbd>↓</kbd> <kbd>PgUp</kbd> <kbd>PgDn</kbd> <kbd>Home</kbd> <kbd>End</kbd> (ou <kbd>j</kbd> <kbd>k</kbd>) | Mover a seleção |
| <kbd>Ctrl+L</kbd> / <kbd>/</kbd> | Buscar pelo nome, ou pela descrição (<kbd>Esc</kbd>, <kbd>Enter</kbd> ou <kbd>Ctrl+P</kbd> volta à lista) |
| <kbd>F7</kbd> / <kbd>Alt+D</kbd> | Buscar por nome ou por descrição |
| <kbd>F8</kbd> / <kbd>t</kbd> | Filtrar por tap (homebrew/core, homebrew/cask, taps de terceiros); uma letra salta na lista |
| <kbd>i</kbd> / <kbd>Alt+I</kbd> | Alternar entre os instalados e o repositório |
| <kbd>F6</kbd> / <kbd>s</kbd> | Escolher a coluna de ordenação (de novo na mesma coluna inverte) |
| Clique no cabeçalho | Ordenar: crescente, decrescente, ordem do brew |
| <kbd>1</kbd> <kbd>2</kbd> <kbd>4</kbd> <kbd>6</kbd> / <kbd>Alt+1</kbd>… | Abas Informação, Arquivos, Saída e Ajuda, com os números do OctoPkg |
| <kbd>F1</kbd> / <kbd>?</kbd> | Ajuda |
| <kbd>Enter</kbd> | Aba Informação do pacote selecionado |
| <kbd>Tab</kbd> | Rolar a aba com as setas (de novo, ou <kbd>Esc</kbd>, volta à lista) |
| <kbd>Ctrl+F</kbd> | Buscar texto nas abas Arquivos, Saída e Ajuda (<kbd>F3</kbd> / <kbd>Shift+F3</kbd>) |
| <kbd>F9</kbd> / <kbd>Alt+A</kbd> <kbd>Alt+B</kbd> <kbd>Alt+J</kbd> | Menus Arquivo, Busca e Ajuda |
| <kbd>Alt+,</kbd> / <kbd>Alt+T</kbd> | Preferências (tema, idioma, ícones) / próximo tema |
| <kbd>F11</kbd> / <kbd>Alt+Z</kbd> | Maximizar a lista, depois as abas, depois voltar às duas |
| <kbd>Ctrl+Q</kbd> / <kbd>F10</kbd> / <kbd>q</kbd> | Sair |

No Terminal do macOS, ative **"Usar Option como tecla Meta"** para os atalhos com Alt (no
iTerm2: *Left Option key: Esc+*). O mapa completo está em [docs/KEYMAP.md](docs/KEYMAP.md).

## Arquitetura

| Camada | Local | Responsabilidade |
|:---|:---|:---|
| Núcleo | `src/core/` | `brew.*`: o brew sem shell, a leitura do JSON dele e do cache da API, estados, dependentes e tamanhos em disco. `json.*`: um leitor de JSON próprio. `process`, `i18n` e `settings` vêm do octopkg-tui sem mudança. C++17 puro, sem dependências. |
| Modelo | `src/model/` | Filtro, ordenação (versão, tipo, tamanho, data, estado), filtro por tap e a seleção que acompanha o pacote (pela chave `f:nome` / `c:token`) |
| Interface | `src/tui/` | FTXUI: tela, temas, teclado, mouse, diálogos, menus e busca nas abas, do octopkg-tui; listagem, detalhes e tamanhos em segundo plano |

## Testes

```sh
cmake -B build -S . -DOCTOBREW_TUI_BUILD_TESTS=ON && cmake --build build && ctest --test-dir build
sh tests/macos-e2e.sh build    # tudo, mais a integração com o Homebrew real (só leitura)
sh tests/tui-smoke.sh build    # a interface num pseudo-terminal, com um brew falso
```

- `test_core`: o leitor de JSON, a saída do brew 7 (instalados, detalhes de fórmula e de
  cask) e o cache da API (formatos do brew 5+ e do brew 4), com um brew falso que confere
  os argumentos e o ambiente de cada chamada; tamanhos em disco, processos e preferências.
- `test_model`: filtro, ordenação, tap, seleção e rolagem, com 40 mil pacotes.
- `test_view`: desenha a tela em texto e dirige a interface com eventos de teclado,
  inclusive o caminho completo (listagem, repositório, detalhes e tamanhos) com o brew
  falso.
- `test_integration`: contra o Homebrew real, só lendo.
- `tests/tui-smoke.sh`: a interface num pseudo-terminal com um brew falso; roda também no
  Ubuntu.

O CI roda no macOS (com o Homebrew do runner) e no Ubuntu (FTXUI 6.1.9, com ASan e UBSan).

## Licença

[GPL-2.0-or-later](LICENSE). A interface e o modelo vêm do octopkg-tui, cujo núcleo é um
porte de código do OctoPkg, © Alexandre Albuquerque Arnt (GPL-2.0-or-later).

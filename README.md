# oly
A CLI for managing olympiad problem solutions.

## Installation
You will need git and cmake to clone and build the project. Alternatively, you can
grab an executable from [GitHub releases](https://github.com/anonymousgrasshopper/oly/releases).

We provide an install script which will install the executable, the Zsh
completion, the typst header, and will setup the oly scheme handler for
`oly gen`.
```sh
curl -fsSL https://raw.githubusercontent.com/anonymousgrasshopper/oly/main/install.sh | bash
```

To build the project yourself, you can
```sh
git clone https://github.com/anonymousgrasshopper/oly
cd oly
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build
sudo cp build/bin/oly /usr/local/bin/oly
```
If you have [just](https://github.com/casey/just) installed, you can also
just run `just install` in the repo to do the above.

```sh
# if you use typst
cp -r assets/typst ~/.local/share/

# if you use zsh
sudo cp assets/extras/_oly /usr/local/share/zsh/site-functions/

# if you want the 'oly gen' scheme handler
copy assets/typst/oly-scheme-handler/oly-handler ~/.local/bin/
copy assets/typst/oly-scheme-handler/oly.desktop ~/.local/share/applications/
xdg-mime default oly.desktop x-scheme-handler/oly

# if you use Neovim
cp assets/extras/tex.lua ~/.config/nvim/after/ftplugin/
cp assets/extras/typst.lua ~/.config/nvim/after/ftplugin/
```

## Usage
```
$ oly --help
usage: oly <cmd> [args [...]].

Available subcommands:
    add                          - add a problem to the database
    edit                         - edit an entry from the database
    gen                          - generate a PDF from a problem
    search                       - search problems by contest, metadata...
    show                         - print a problem statement
    alias                        - link a problem to another one
    rm                           - remove a problem and its solution file
    mv                           - rename a problem
  Run oly <cmd> --help for more information regarding a specific subcommand

Arguments:
    --help              -h       - Show this help message
    --config-file FILE  -c FILE  - Specify config file to use
    --verify-config              - Check wether the config has any errors
    --version           -v       - Print this binary's version
    --log-level LEVEL            - Set the log level
```

## Configuration
Configuration is done via a yaml file at `${XDG_CONFIG_HOME:-$HOME/.config}/oly/config.yaml`.

If it does not exist yet, it will be created and opened in your editor with an example config.

## Using the oly scheme handler
The `oly` typst header file creates an eponymous command which takes a problem name
as its first argument and an optional content as its second argument. It
creates a link that, when clicked, will automatically generate and open the
given problem through the `oly gen` command. In order to make this work,
follow these steps:
- copy `assets/typst/oly-scheme-handler/oly-handler` in your `$PATH`
- copy `assets/typst/oly-scheme-handler/oly.desktop` in `~/.local/share/applications/`
- run `xdg-mime default oly.desktop x-scheme-handler/oly`

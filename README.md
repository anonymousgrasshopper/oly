# oly
A CLI for managing olympiad problem solutions.

## Installation
```sh
git clone https://github.com/anonymousgrasshopper/oly
cd oly
cmake -DCMAKE_BUILD_TYPE=Release build
cmake --build build
sudo cp build/bin/oly /usr/local/bin/oly
```
If you use typst, you will need to copy `./assets/typst/packages/local/oly` in `~/.local/share/typst/packages/local`.
If you use Neovim, you can copy `./assets/extras/(tex|typst).lua` in your `after/ftplugin` directory.
If you use Zsh, you can copy `./assets/extras/_oly` somewhere in your `$fpath`.

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
Configuration is done via a yaml file in `~.config/oly/config.yaml`.

If it does not exist yet, it will be created and opened in your editor with an example config.

# uip-exercise-1 ![.github/workflows/main.yml](https://github.com/LinqLover/uip-exercise-1/workflows/.github/workflows/main.yml/badge.svg)

Application skeleton was provided by the chair for Computer Graphical Systems at Hasso Plattner Institute, Potsdam, in winter term 2020/21.

## Command Line Interface

```
Usage: pscom-cli <command> <filter>* [-C <directory or `-'>] [--yes]

Defaults:
  <directory or `-'> - $(pwd)

Commands:
  help - print this help
  version - print version
  supported-formats - print supported formats
  list - only show files
  copy <dest> - copy files into destination
  move <dest> - move files into destination
  rename [<scheme>] - rename images
    <scheme> defaults to UPA file name standard `yyyyMMdd_HHmmsszzz'
  group [<scheme>] - group images
    <scheme> defaults to UPA directory name standard `yyyy/yyyy-MM'
  group-static <name> - group images using static name
  rescale - rescale images
    options (at least one required):
      --width=<width>
      --height=<height>
  convert - convert images
    options (all required):
      --format=<format>
      --quality=<quality>

Filters:
  --re, --regex <pattern> - regular expression
  --mi, min-date -- minimum date
  --ma, max-date -- maximum date
```

# uip-exercise-1 [![.github/workflows/main.yml](https://github.com/LinqLover/uip-exercise-1/workflows/.github/workflows/main.yml/badge.svg)](https://github.com/LinqLover/uip-exercise-1/actions?query=workflow%3A.github%2Fworkflows%2Fmain.yml)

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

Pattern: &lt;example command&gt; \[&lt;relevant methods from `pscom.h`&gt;\]

- **Find images:** `pscom-cli list` [`re`, `dt`]
  * by time filter: `pscom-cli list --before=2017-04-17 --after=2017-02-08`
  * by regex: `pscom-cli list --regex='car[it]'`
    + TODO: Should regex be full patterns always? API says yes, but we could circumvent.
- **Copy or move images:** [`cp`, `mv`]
  * copy: `pscom-cli copy <target> <filter>`
  * move: `pscom-cli move <target> <filter>`
- **Rename images by scheme:** [`fn`/`fp`?, `mv`]
  * UPA scheme: `pscom-cli rename`
  * custom scheme: `pscom-cli rename --scheme=<scheme>`
- **Group images by scheme:** [`fp`?, `mv`]
  * UPA scheme: `pscom-cli group`
  * custom scheme: `pscom-cli group --scheme=<scheme>`
  * single group: `pscom-cli group-single --name=<name>`
- **Scale down images:** [`sw`, `sh`, `ss`]
  * `pscom-cli rescale --width=<width> --height=<height>`
- **Convert images into different format or quality:** \[`cf`\]
  * `pscom-cli convert --format=<format> --quality=<quality>`

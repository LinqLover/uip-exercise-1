# uip-exercise-1 [![.github/workflows/main.yml](https://github.com/LinqLover/uip-exercise-1/workflows/.github/workflows/main.yml/badge.svg)](https://github.com/LinqLover/uip-exercise-1/actions?query=workflow%3A.github%2Fworkflows%2Fmain.yml)

Application skeleton was provided by the chair for Computer Graphical Systems at Hasso Plattner Institute, Potsdam, in winter term 2020/21.

## Command Line Interface

```
sage: ./pscom-cli/bin/pscom-clid [options] command
Photo system command-line tool

Options:
  -h, --help, -?              Displays help on command-line options.
  -V, --version               Displays version information.
  -v, --verbose               Verbose mode. Specify up to 2 times to increase
                              the verbosity level of output messages. Opposite
                              of quiet mode.
  -q, --quiet                 Quiet mode. Specify up to 2 times to decrease the
                              verbosity level of output messages. Opposite of
                              verbose mode.
  --on-conflict <strategy>    Conflict resolution strategy to be applied when a
                              destructive operation is run. Can be one of the
                              following:
                              - overwrite: Overwrite the original file
                              irrecoverably.
                              - skip: Just forget this incident and continue
                              with the next file.
                              - backup: Create a backup of the original file (by
                              appending a squiggle (~) to its file name) and
                              then overwrite it.
  -f, --force                 Enforce possibly destructive operations
                              regardless of the consequences. Equivalent to
                              on-conflict=overwrite.
  --supported-formats         Display all supported image formats.
  -d, -C, --directory <path>  The directory to look up image files. Pass a
                              single dash (-) to enter a list of image files
                              interactively.
  -R, --recursive             Include subdirectories.
  -r, --regex <pattern>       A regular expression to filter image files. Does
                              not need to match the entire file name; use text
                              anchors (^ $) for full matches.
  --min, --min-date <date>    Reject images older than the given date and time.
  --max, --max-date <date>    Reject images newer than the given date and time.
                              NOTE: If you only specify the date, it will be
                              treated as midnight time.
  --dry-run                   Only simulate all modifications to the filesystem
                              instead of actually applying them. Can be helpful
                              to understand the consequences of your complicated
                              invocation without hazarding your entire photo
                              library.
  --width <number>            The width the images should be fit into.
  --height <number>           The height the images should be fit into.
  --format <extension>        The file format (e.g. jpg or png) the images
                              should be converted into.
  --quality <value>           The quality for image conversion. Value between 0
                              (best compression) and 100 (best quality).

Arguments:
  command                     The operation to perform.

Commands:
  pscom <symbol> <arguments>  Execute a symbol from the pscom library manually.
                              No safety checks! Intended for debugging purposes
                              only.
  list, ls                    Display image files.
  copy, cp <destination>      Copy image files into the specified destination
                              folder.
  move, mv <destination>      Move image files into the specified destination
                              folder.
  rename, rn [<schema>]       Rename image files according to the given schema,
                              or according to the UPA standard, if omitted. To
                              escape date specifiers in the schema, enclose
                              literal parts into single quotes (', or "'" from
                              the bash shell).
  group, g [<schema>]         Group image files into subdirectories according
                              to the given schema, or according to the UPA
                              standard, if omitted. To escape date specifiers in
                              the schema, enclose literal parts into single
                              quotes (', or "'" from the bash shell).
  resize                      Resize image files into the given dimensions.
  convert                     Convert image files into a different file format
                              and/or quality.
```

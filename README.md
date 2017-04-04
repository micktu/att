# NieR: Automata Text Tools

This is a tool for working with game's formats, disassembling scripts, and replacing files.

It does not handle `.cpk` archives, and works with data that has already been extracted from these.

## Commands

### List files

Lists all files in a directory, including contents of DAT archives.

`att list DATA FILTER`

`DATA` Directory containing the game data. Default is `data`.

`FILTER` Include only a subset of files.

A filter can be a file extension (e.g. `wta`), or one of the predefined filters, `text` (all text formats) and `data` (all DAT containers, regardless of file extension).

### Extract files

Collects all files from a directory, extracting DAT files, and copying the rest.

`att extract DATA OUTPUT FILTER`

`DATA` Directory containing the game data. Default is `data`.

`OUTPUT` Directory where the extracted files will be. Default is `extract`.

`FILTER` Include only a subset of files. Explained in `List files` section.

### Export text

Extracts and converts all text from the game data into an editable text format.

`att export DATA OUT`

`DATA` Directory containing the game data. Default is `data`.

`OUT` Directory where the text files will be. Default is `text`.

### Import text

This command analyzes changes in edited text files, and generates a patch that can be put directly into game data folder.

`att import DATA TEXT OUT`

`DATA` Directory containing the game data. Default is `data`.

`TEXT` Directory containing previously exported text. Default is `text`.

`OUT` Directory where patched files will be. Default is `patch`.

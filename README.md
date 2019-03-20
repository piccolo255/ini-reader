Small C library for reading config files in INI format.

# INI format rules

* Keys and values
  * Key-value pairs are one per line, in "key = value" format.
  * Key names have to start with a number or a letter, and can't include the "=" character.
  * Whitespace surrounding keys and values is ignored.
  * There are no other restrictions to keys or values.
* Sections
  * K-V pairs can be put into sections.
  * Start of a new section is indicated with a "[section name]".
  * There can be no duplicate sections.
  * Before the first named section is a global, unnamed section.

# Usage

### Types

Parsed data is stored in a `t_ini_reader_data` typedefed pointer,
which points to a linked list of sections,
each containing a linked list of key-value pairs.
This pointer is required for all further operations.

### Initialization

Definition:
    t_ini_reader_data ini_reader_parse( const char *filename );

Example usage:
    t_ini_reader_data config = ini_reader_parse( "config.ini" );

Configuration in the `config.ini` file is parsed
and stored in a structure pointer `config`.

### Read values



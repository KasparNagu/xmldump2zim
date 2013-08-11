xmldump2zim
===========

Converts MediaWiki XML dumps to the OpenZim format

This is heavily work in progress, for now conversion is somewhat possible using the MWPhpParser, but without images.
Much is hardcoded. There is no autoconfig, just a makefile (where the dependencies have to be configured).


## Compile
The xml parsing and zim generation dependes on
- libxml2
- the openzim (git clone https://gerrit.wikimedia.org/r/p/openzim.git, see http://www.openzim.org)

### Parser
There are two attempts of parser, at the moment the MWPhpParser is used only

#### MWPhpParser
The MWPhpParser uses the mediawiki php code to do the HTML conversion, to use this you need:
- php with --php-embedd, I'm using php-5.5.1
- mediawiki I'm using mediawiki-1.21.1
- A mediawiki css file, just grab it by calling Makefile all.css
- other extensions as needed:Math (including texvc), Scribunto, ...) 

Adjust the MWPhpParser.php to your needs (especially add extensions you need)

Hints:
- some articles need quite some stack, ulimit -s 16384 helps.
- this parser is not multithreaded and quite slow; the first 400000 articles of the german wiki take about 23h on my i5-2520, thus the whole wiki will take about 1 week.
- if you link against a custom build php (like prepared in the Makefile), you need to specify the LD_LIBRARY_PATH 

#### KiwiParser
The KiwiParser uses the Kiwi parser to do the HTML conversion, see https://github.com/aboutus/kiwi.

This is not really working at the moment; it hangs for some pages.

## Usage

./xmldump2zim dump.xml output.zim [maximal number of articles]


### Advanced

./xmldump2zim dump.xml index: creates an index file for the given dump.xml, containing beginning and length of each page tag

this index file is then used for two things; 
- if present, its loaded at startup and used for template lookup, you can speed this up by filtering only the necessary lines: grep Vorlage\\\|Modul index > index-small; mv index-small index (for german wiki, where Vorlage is the name of the Template namespace)
- you can use the lines do create a file of only a selection if pages, which can be supplied instead of the [maximal number of articles] argument; ex: grep Bezirk index > bezirk-pages and then ./xmldump2zim dump.xml output.zim bezirk-pages


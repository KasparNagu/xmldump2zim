xmldump2zim
===========

Converts MediaWiki XML dumps to the OpenZim format

This is heavily work in progress, for now conversion is somewhat possible using the MWPhpParser, but without images.
Much is hardcoded. There is no autoconfig, just a makefile.

There are two attempts of parser:

The MWPhpParser uses the mediawiki php code to do the HTML conversion, to use this you need:
-php with --php-embedd, I'm using php-5.5.1
-mediawiki with the math extension (including texvc), I'm using mediawiki-1.21.1

The KiwiParser uses the Kiwi parser to do the HTML conversion, see https://github.com/aboutus/kiwi.
This is not really working at the moment; it hangs for some pages.

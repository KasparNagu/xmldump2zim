KIWI=../kiwi
PHPSRC=../php-5.5.1
PHP_LDFLAGS=${PHPSRC}/.libs/libphp5.so
PHP_CXXFLAGS=-I${PHPSRC} -I${PHPSRC}/sapi/embed -I${PHPSRC}/Zend -I${PHPSRC}/TSRM -I${PHPSRC}/main

CXXFLAGS=-I../openzim/zimlib/include/ $(shell pkg-config --cflags libxml-2.0) 
LDFLAGS=../openzim/zimlib/src/.libs/libzim.a -llzma $(shell pkg-config --libs libxml-2.0)
%: %.cpp 
	${CXX}  -g ${CXXFLAGS} -o $@ $+ ${LDFLAGS}

FILES=xmldump2zim KiwiParser.o MWPhpParser.o

all:$(FILES)

KiwiParser.o: KiwiParser.cpp
	${CXX}  -g ${CXXFLAGS} -I${KIWI}/src -c -o $@ $+

MWPhpParser.o: MWPhpParser.cpp
	${CXX}  -g ${CXXFLAGS} ${PHP_CXXFLAGS} -c -o $@ $+ 


xmldump2zim: xmldump2zim.cpp MWPhpParser.o
	${CXX}  -g ${CXXFLAGS} -o $@ $+ ${LDFLAGS} ${PHP_LDFLAGS}

all.css:
	wget -O $@ 'https://bits.wikimedia.org/de.wikipedia.org/load.php?debug=false&lang=de&modules=site%7Cext.flaggedRevs.basic%7Cext.gadget.CommonsDirekt%2CWikiMiniAtlas%7Cext.rtlcite%2Cwikihiero%7Cext.uls.nojs%7Cmediawiki.legacy.commonPrint%2Cshared%7Cmw.PopUpMediaTransform%7Cskins.vector&only=styles&skin=vector&*'
	echo '\n.editsection { display: none; }'  >> $@
	#somewhere there seems to be a bug swallowing some trailing bytes (but where), so as workaround we just fill
	yes "          " | dd conv=notrunc oflag=append of=$@ bs=1k count=1
clean:
	rm -f $(FILES)

#include "KiwiParser.h"

extern "C" {
#include "kiwi.h"
#include "io.h"
}


zim::Blob KiwiParser::generateHtml(const std::string &in, const std::string &title){
	printf("init\n");
	init();
	printf("balloc %ld\n",in.size());
	ballocmin(input_buffer,in.size());
	memcpy(input_buffer->data,in.data(),in.size());
	input_buffer->slen=in.size();
	printf("parsing\n");
	::parse();
	printf("parsing done");
//	printf("%d %s\n",blength(output_buffer),bdata(output_buffer));
//	std::string retS(bdata(output_buffer),blength(output_buffer));
	
//	zim::Blob ret(bdata(output_buffer),blength(output_buffer));	
//	zim::Blob ret(NULL,0);
//	zim::Blob ret(retS.data(),retS.size());
	size_t len=blength(output_buffer);
	printf("storing %ld\n",len);
	char *data = (char*)malloc(len);
	memcpy(data,bdata(output_buffer),len);
	cleanup();
	return zim::Blob(data,len);
}



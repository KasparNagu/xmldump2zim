#include <string>
#include <iostream>
extern "C" {
#include "php_embed.h"
}
#include "MWPhpParser.h"


char * MWPhpParser::php_argv[2] = {(char*)"MWPhpParser", NULL};


void MWPhpParser::init(){
	if (php_embed_init(1, php_argv PTSRMLS_CC) == FAILURE) {
		fprintf(stderr,"php init failed\n");
		exit(1);
	} 
	
	registerFunctions();
	
	evalString((char*)"error_reporting(E_ALL);");				
	evalString((char*)"ini_set('display_errors', True);");
	evalString((char*)"ini_set('display_startup_errors', True);");
	evalString((char*)"include('MWPhpParser.php');");
}

ArticleSupplier *MWPhpParser_global_articleSupplyer = NULL;
void MWPhpParser::setArticleSupplier(ArticleSupplier *sup){
	WikiParser::setArticleSupplier(sup);
	MWPhpParser_global_articleSupplyer = sup;
}

ZEND_FUNCTION(getArticleText){
	char *text = NULL;
        int text_len;

        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &text, &text_len) == FAILURE) {
                return;
        }
	std::string title(text,text_len);
	std::string *str = MWPhpParser_global_articleSupplyer->getArticleText(title);
	if(str==NULL){
		RETURN_FALSE;
	}else{
		RETURN_STRINGL(str->data(), str->size(), 1);
	}	
}
void MWPhpParser::registerFunctions(){
	zend_function_entry *	additional_functions = (zend_function_entry*)malloc(sizeof(zend_function_entry)*2);
	ZEND_BEGIN_ARG_INFO(arginfo_getArticleText, 0)
	    ZEND_ARG_INFO(0, title)
	ZEND_END_ARG_INFO()

	additional_functions[0].fname = "getArticleText";
	additional_functions[0].handler = ZEND_FN(getArticleText);
	additional_functions[0].arg_info = arginfo_getArticleText;
	additional_functions[0].num_args = 1;
	additional_functions[0].flags = 0;

	additional_functions[1].fname = NULL;
	additional_functions[1].handler = NULL;
	additional_functions[1].arg_info = NULL;

	zend_module_entry phpparser_module_entry = {
		STANDARD_MODULE_HEADER,
		(char*)"PhpParser", 
		additional_functions, 
		NULL, //startup
		NULL, //shutdown
		NULL, //request startup
		NULL, //request shutdown
		NULL, //info function, 
		NO_VERSION_YET,
		STANDARD_MODULE_PROPERTIES
	};
	zend_module_entry *entries[] = {&phpparser_module_entry};
	php_register_extensions(entries, 1);
}
void MWPhpParser::evalString(char *string){
	zend_first_try {
	    if (zend_eval_string(string, NULL, (char*)__func__) == FAILURE) {
	    }
	} zend_catch {
	} zend_end_try();
}

zim::Blob MWPhpParser::generateHtml(const std::string &text,const std::string &title){
	zval *args[2];
	zval ret,funcname,zstr,ztitle;
	ZVAL_STRING(&funcname,"generateHtml",0);
	ZVAL_STRINGL(&zstr,text.data(),text.size(),0);
	ZVAL_STRINGL(&ztitle,title.data(),title.size(),0);
	args[0] = &zstr;
	args[1] = &ztitle;
	call_user_function(EG(function_table),NULL,&funcname,&ret,2,args TSRMLS_CC);
	//return std::string(Z_STRVAL(ret),Z_STRLEN(ret));
	return zim::Blob(Z_STRVAL(ret),Z_STRLEN(ret));
}
void MWPhpParser::shutdown(){
	php_embed_shutdown(TSRMLS_C);
}

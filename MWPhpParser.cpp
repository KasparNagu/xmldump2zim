#include <string>
#include <iostream>
extern "C" {
#include "php_embed.h"
}
#include "MWPhpParser.h"


char * MWPhpParser::php_argv[2] = {(char*)"MWPhpParser", NULL};


void MWPhpParser::init(){
	useCounter = 0;
	php_embed_module.php_ini_path_override = (char*)"php.ini";
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
		RETVAL_STRINGL(str->data(), str->size(), 1);
		delete str; //we have to delete
		return;
	}	
}

ZEND_FUNCTION(getNamespaceName){
        long ns_id;

        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &ns_id) == FAILURE) {
                return;
        }
	std::string *str = MWPhpParser_global_articleSupplyer->getNamespace(ns_id);
	if(str==NULL){
		RETURN_FALSE;
	}else{
		RETVAL_STRINGL(str->data(), str->size(), 1);
		//we mustn't delete
		return;
	}	
}
void MWPhpParser::registerFunctions(){
//	zend_function_entry *	additional_functions = (zend_function_entry*)malloc(sizeof(zend_function_entry)*3);
	ZEND_BEGIN_ARG_INFO(arginfo_getArticleText, 0)
	    ZEND_ARG_INFO(0, title)
	ZEND_END_ARG_INFO()
	ZEND_BEGIN_ARG_INFO(arginfo_getNamespaceName, 0)
	    ZEND_ARG_INFO(0, id)
	ZEND_END_ARG_INFO()

	zend_function_entry additional_functions[] = {
		ZEND_FE(getArticleText,arginfo_getArticleText)
		ZEND_FE(getNamespaceName,arginfo_getNamespaceName)
		ZEND_FE_END
	};


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
	if(useCounter>100){
		shutdown();
		init();
		useCounter = 0;
	}else{
		useCounter++;
	}
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

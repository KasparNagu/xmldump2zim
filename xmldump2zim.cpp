/*
 * Copyright (C) 2013 Kaspar Baltzer
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * is provided AS IS, WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, and
 * NON-INFRINGEMENT.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 */

#include <iostream>
#include <zim/writer/articlesource.h>
#include <zim/writer/zimcreator.h>
#include <zim/blob.h>
#include <sstream>

#include <stack>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <map>
#include <fstream>
#include "MWPhpParser.h"


#define SSTR( x ) dynamic_cast< std::ostringstream & >( \
        ( std::ostringstream() << std::dec << x ) ).str()

class XmlDumpArticle : public zim::writer::Article
{
    friend class XmlDumpSource;
    private:
    std::string aid; //article id
    std::string redirectAid;
    long nid; //namespace id
    std::string title;
    char ns;
    std::string mimeType;
    public:
    virtual std::string getAid() const;
    virtual char getNamespace() const;
    virtual std::string getUrl() const;
    virtual std::string getTitle() const;
    virtual zim::size_type getVersion() const;
    virtual bool isRedirect() const;
    virtual std::string getMimeType() const;
    virtual std::string getRedirectAid() const;

};

std::string XmlDumpArticle::getAid() const
{
  return aid;
}

char XmlDumpArticle::getNamespace() const
{
  return ns;
}

std::string XmlDumpArticle::getUrl() const
{
  return title;
}

std::string XmlDumpArticle::getTitle() const
{
  return title;
}

zim::size_type XmlDumpArticle::getVersion() const
{
  return 0;
}

bool XmlDumpArticle::isRedirect() const
{
  return !redirectAid.empty();
}

std::string XmlDumpArticle::getMimeType() const
{
  return mimeType;
}

std::string XmlDumpArticle::getRedirectAid() const
{
  return redirectAid;
}


class XmlDumpSource : public zim::writer::ArticleSource, public ArticleSupplier {
	struct pagePosition {
		char* start;
		off_t len;
	};
	private:
		FILE *f;
		char *file;
		size_t fileSize;
		char *curPageStart;
		std::map<std::string,pagePosition> pagePositions;
		std::map<long,std::string> namespaces;
	    	int parse_siteinfo();
		long counter;
		bool fileEnded; //to track when we're at the end of the file
		void savePagePosition(const std::string &aid,char* start, off_t len);
		WikiParser *wikiParser;
		long nMaxArtices;
		const zim::writer::Article* getNextCss();
		std::stack<std::string> cssStack;
	public:
	    virtual const zim::writer::Article* getNextArticle();
	    virtual zim::Blob getData(const std::string& aid);
	    
	    const zim::writer::Article* getNextPage();
	    void saveIndex(const char *filename);
	    void loadIndex(const char *filename);
	    
	   XmlDumpSource(char *xmlFile){
		nMaxArtices = 10;
		counter = 0;
		fileEnded = false;
		wikiParser = new MWPhpParser();
		wikiParser->init();
		wikiParser->setArticleSupplier(this);
		f = fopen(xmlFile,"r");
		if(!f){
			fprintf(stderr, "could not open file %s\n",xmlFile);
			exit(1);
		}
		struct stat st;
		fstat(fileno(f),&st);
		fileSize = st.st_size;
		printf("size: %ld\n", fileSize); 
		file = (char*)mmap(NULL,fileSize,PROT_READ,MAP_PRIVATE,fileno(f),0);
		curPageStart = file;

		parse_siteinfo();

		if(namespaces.empty()){
			fprintf(stderr,"xml read error\n");
			exit(1);
		}

		cssStack.push("all.css");
	    };
	   virtual std::string *getArticleText(const std::string &aid);

	virtual ~XmlDumpSource(){
		fclose(f);
		munmap(file,fileSize);
		wikiParser->shutdown();

	}

	void setMaxArticles(long n){
		nMaxArtices = n;
	}

	

	
};




int main(int argc, char* argv[]){
    if(argc < 3){
	printf("usage: %s xmlFile [zimFile|index]\n", argv[0]);
	exit(1);
    }
    char *xmlFile = argv[1];
    XmlDumpSource *source = new XmlDumpSource(xmlFile);
    if(argc > 3){
	source->setMaxArticles(strtol(argv[3],NULL,10));
    }

    if(strcmp(argv[2],"index")==0){
    	while(source->getNextPage()!=NULL);
	source->saveIndex("index");
	exit(0);
    }
    source->loadIndex("index");

    char *zimFile = argv[2];
    std::ifstream file(zimFile);
    if(file){
	std::cout << zimFile << " already exists" << std::endl;
	exit(1);
    }
    zim::writer::ZimCreator creator(argc, argv);
    std::string fname = std::string(zimFile);
    creator.create(fname, *source);
    free(source);
}





int XmlDumpSource::parse_siteinfo() {
	char *siteinfoStart = strstr(file,"<siteinfo>");
	if(siteinfoStart == NULL){
		fprintf(stderr,"error parsing siteinfo\n");
		return -1;
	}
	char *siteinfoEnd = strstr(siteinfoStart+10,"</siteinfo>");
	if(siteinfoEnd == NULL){
		fprintf(stderr,"error parsing siteinfo end\n");
		return -1;
	}
	xmlDocPtr doc; 
	doc = xmlReadMemory(siteinfoStart, siteinfoEnd-siteinfoStart+11, NULL, NULL, 0);
	xmlNodePtr cur = doc->xmlChildrenNode;
	if (cur != NULL && strncmp((char*)cur->name,"siteinfo",8)==0) {
		cur = cur->xmlChildrenNode;
		while(cur != NULL){
			if(strcmp((char*)cur->name,"namespaces")==0){
				cur = cur->xmlChildrenNode;
				while(cur != NULL){
					if(strcmp((char*)cur->name,"namespace")==0){
						xmlChar *keyStr = xmlGetProp(cur,(xmlChar*)"key");
						if(keyStr != NULL){
							xmlChar *nsStr = xmlNodeListGetString(doc,cur->xmlChildrenNode,1);
							if(nsStr == NULL){
								namespaces[strtol((char*)keyStr,NULL,10)]=std::string();
							}else{
								namespaces[strtol((char*)keyStr,NULL,10)]=std::string((char*)nsStr);
								xmlFree(nsStr);
							}
							xmlFree(keyStr);
						}
					}
					cur = cur->next;
				}
				break;
			}
			cur=cur->next;
		}

	}

	xmlFreeDoc(doc);
	return 0;
}

const zim::writer::Article* XmlDumpSource::getNextArticle() {
	const zim::writer::Article* art;
	art = getNextCss();
	if(art == NULL){
		art = getNextPage();
	}
	return art;
}

const zim::writer::Article* XmlDumpSource::getNextCss() {
	if(!cssStack.empty()){
		XmlDumpArticle *a = new XmlDumpArticle();
		a->ns = '-';
		a->mimeType="text/html";
		a->aid = "css/" + cssStack.top();
		a->title = cssStack.top().substr(cssStack.top().find_last_of("/\\")+1);
		cssStack.pop();
		return a;
	}
	return NULL;
}

const zim::writer::Article* XmlDumpSource::getNextPage() {
	if(nMaxArtices > 0 && counter > nMaxArtices){
		return NULL;
	}
	if(fileEnded){
		return NULL;
	}
	XmlDumpArticle *a = new XmlDumpArticle();
	a->ns = 'A';
	a->mimeType="text/html";
	if( (curPageStart = strstr(curPageStart,"<page>")) == NULL){
		return NULL;
	}
	char *curPageEnd = strstr(curPageStart,"</page>");
	if(curPageEnd == NULL){
		fprintf(stderr,"page end not found!\n");
		return NULL;
	}
	xmlDocPtr doc;
	off_t curPageLen = curPageEnd-curPageStart+7;
	doc = xmlReadMemory(curPageStart, curPageLen, NULL, NULL, 0);
	if (doc == NULL) {
		fprintf(stderr, "Failed to parse document\n");
		return NULL;
	}
	xmlNodePtr cur = doc->xmlChildrenNode;
        if (cur != NULL && strncmp((char*)cur->name,"page",5)==0) {
		cur = cur->xmlChildrenNode;
		while(cur != NULL){
			if(strcmp((char*)cur->name,"title")==0){
				xmlChar *t = xmlNodeListGetString(doc,cur->xmlChildrenNode,1);
				if(t != NULL){
					a->title = std::string((char*)t);
					xmlFree(t);
			#ifndef useId
					a->aid=a->title;
			#endif
				}
			#ifdef useId
			}else if(strcmp((char*)cur->name,"id")==0){
				xmlChar *t = xmlNodeListGetRawString(doc,cur->xmlChildrenNode,1);
				a->aid = std::string((char*)t);
				xmlFree(t);
			#endif
			}else if(strcmp((char*)cur->name,"ns")==0){
				xmlChar *t = xmlNodeListGetRawString(doc,cur->xmlChildrenNode,1);
				a->nid = strtol((char*)t,NULL,10);
				xmlFree(t);
			}else if(strcmp((char*)cur->name,"redirect")==0){
				xmlChar *redirectTitleStr = xmlGetProp(cur,(xmlChar*)"title");
				if(redirectTitleStr != NULL){
					a->redirectAid = std::string((char*)redirectTitleStr);
				}
			}
			cur=cur->next;
		}
	}
	xmlFreeDoc(doc);

	savePagePosition(a->aid,curPageStart,curPageLen);

	if(curPageEnd-file+10>fileSize){
		fileEnded=true;
	}
	curPageStart=curPageEnd+7;
	
	counter++;
	return a;
}
void XmlDumpSource::savePagePosition(const std::string& aid,char *start, off_t len){
	pagePosition *pPos = &(pagePositions[aid]);
	pPos->start=start;
	pPos->len=len;
}

zim::Blob XmlDumpSource::getData(const std::string& aid){
	if(aid.compare(0,4,"css/") == 0){
		std::cout << "handlin css:"<<aid.substr(4)<<std::endl;
		std::ifstream cssFile(aid.substr(4).c_str());
		if(cssFile){		
   		    	// get length of file:
			cssFile.seekg (0, cssFile.end);
			size_t length = cssFile.tellg();
			cssFile.seekg (0, cssFile.beg);
			char * buffer = new char [length];
			cssFile.read (buffer,length);
			cssFile.close();
			return zim::Blob(buffer,length);
		}else{
			std::cout<<"Couldn't load css file" <<std::endl;
		}
	}

	std::string *text = getArticleText(aid);
	if(text == NULL){
		return zim::Blob(NULL,0);
	}else{
		std::cout << "serving " << aid << std::endl;
		return wikiParser->generateHtml(*text,aid);
		free(text);
	}
}

void XmlDumpSource::saveIndex(const char *filename){
	std::ofstream indexFile;
	indexFile.open(filename);
	std::map<std::string, pagePosition>::iterator iter;
	for (iter = pagePositions.begin(); iter != pagePositions.end(); ++iter) {
		indexFile << (iter->second.start-file) << " " << iter->second.len << " " << iter->first << std::endl;
	}
	indexFile.close();
}

void XmlDumpSource::loadIndex(const char *filename){
	std::ifstream indexFile(filename);
	if(!indexFile){
		return;
	}
	printf("loading index from %s\n",filename);
	long pos;
	long len;
	std::string title;
	long count = 0;
	while(indexFile >> pos && indexFile >> len){
		indexFile.get();
		std::getline(indexFile,title);
//		std::cout << pos << ";" << len << ";'" << title << "'"<< std::endl;
		savePagePosition(title,file+pos,len);
		count++;
	}
	indexFile.close();
	printf("%ld loaded\n",count);
}

std::string* XmlDumpSource::getArticleText(const std::string &aid){
	if(pagePositions.count(aid) == 0){
		return NULL;
	}
	pagePosition pPos = pagePositions[aid];
//	std::cout << "Get data for " << aid << " at " << (unsigned long)(pPos.start-file) << " over " << (unsigned long)pPos.len << std::endl;
	xmlDocPtr doc; 
	doc = xmlReadMemory(pPos.start, pPos.len, NULL, NULL, 0);
	xmlNodePtr cur = doc->xmlChildrenNode;
	std::string *text = NULL;
        if (cur != NULL && strncmp((char*)cur->name,"page",5)==0) {
		cur = cur->xmlChildrenNode;
		while(cur != NULL){
			if(strcmp((char*)cur->name,"revision")==0){
				cur = cur->xmlChildrenNode;
				while(cur != NULL){
					if(cur->type == XML_ELEMENT_NODE  && strcmp((char*)cur->name,"text")==0){
						xmlChar *t = xmlNodeListGetString(doc,cur->xmlChildrenNode,1);
						if(t != NULL){
							text = new std::string((char*)t);
//							std::cout << *text << std::endl;
							xmlFree(t);
						}
					}
					cur=cur->next;
				}
				break;
			}
			cur=cur->next;
		}
	}
	xmlFreeDoc(doc);
	return text;
}

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
#ifndef WIKIPARSER_H_
#define  WIKIPARSER_H_

#include <string>
#include <zim/blob.h>

class ArticleSupplier {
	public:
		//caller is responsible of deleteing
		virtual std::string *getArticleText(const std::string &title) = 0;
		//caller must not delete
		virtual std::string *getNamespace(const long id) = 0;
};

class WikiParser {
	public:
		virtual zim::Blob generateHtml(const std::string&, const std::string &title) = 0;
		virtual void init(){};
		virtual void shutdown(){};
		virtual void setArticleSupplier(ArticleSupplier *sup){
			articleSupplier = sup;
		}
	protected:
		ArticleSupplier *articleSupplier;		
};




#endif

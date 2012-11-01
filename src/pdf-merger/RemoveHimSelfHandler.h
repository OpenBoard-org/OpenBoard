/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation, version 2,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with Open-Sankoré; if not, see
 * <http://www.gnu.org/licenses/>.
 */


#if !defined RemoveHimselfHandler_h
#define RemoveHimselfHandler_h

#include "PageElementHandler.h"

namespace merge_lib
{
	//This class remove field from Page object's content.
	class RemoveHimselfHandler: public PageElementHandler
	{
	public:
		RemoveHimselfHandler(Object * page, const std::string & handlerName): PageElementHandler(page)	
		{
			_setHandlerName(handlerName);		
		}
		virtual ~RemoveHimselfHandler()
		{
		}
	private:
		//methods
		virtual void _changeObjectContent(unsigned int startOfPageElement);
	};
}
#endif


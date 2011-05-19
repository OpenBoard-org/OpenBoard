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


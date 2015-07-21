#include <vector>

#include "base/types.h"
#include "base/licensing.h"


namespace Zephyros {

#ifdef USE_CEF
class ClientExtensionHandler;
#endif


class CustomURLManager
{
public:
	CustomURLManager();
	void AddURL(String url);
	void FireCustomURLs();
    
    inline void SetClientExtensionHandler(ClientExtensionHandlerPtr e)
    {
        m_e = e;
    }

private:
	std::vector<String> m_urls;
	ClientExtensionHandlerPtr m_e;
};

} // namespace Zephyros
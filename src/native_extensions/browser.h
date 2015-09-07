//
//  browser.h
//  Ghostlab
//
//  Created by Matthias Christen on 06.09.13.
//
//

#ifndef Ghostlab_browser_h
#define Ghostlab_browser_h


#include <vector>
#include <string>

#include "base/types.h"


namespace Zephyros {

class Browser
{
public:
    Browser(String name, String version, String identifier, String image, bool isDefaultBrowser)
      : m_name(name),
        m_version(version),
        m_identifier(identifier),
        m_image(image),
        m_isDefaultBrowser(isDefaultBrowser)
    {
    }
   
    inline String GetName()
    {
        return m_name;
    }
    
    inline String GetVersion()
    {
        return m_version;
    }
    
    inline String GetIdentifier()
    {
        return m_identifier;
    }
    
    inline String GetImage()
    {
        return m_image;
    }
    
    inline bool IsDefaultBrowser()
    {
        return m_isDefaultBrowser;
    }
    
    JavaScript::Object CreateJSRepresentation()
    {
        JavaScript::Object obj = JavaScript::CreateObject();
        
        obj->SetString("name", m_name);
        obj->SetString("version", m_version);
        obj->SetString("id", m_identifier);
        obj->SetString("image", m_image);
        obj->SetBool("isDefaultBrowser", m_isDefaultBrowser);
        
        return obj;
    }

    
private:
    String m_name;
    String m_version;
    String m_identifier;
    String m_image;
	bool m_isDefaultBrowser;
};


namespace BrowserUtil {

void FindBrowsers(std::vector<Browser*>** ppBrowsers);

Browser* GetDefaultBrowser(std::vector<Browser*>* pBrowsers);
Browser* GetBrowserForIdentifier(std::vector<Browser*>* pBrowsers, String identifier);
Browser* GetBrowserForUserAgent(std::vector<Browser*>* pBrowsers, JavaScript::Object userAgent);
Browser* GetBrowserFromJSRepresentation(std::vector<Browser*>* pBrowsers, JavaScript::Object obj);

bool OpenURLInBrowser(String url, Browser* browser);

bool IsKnownBrowser(String browserName);

void CleanUp(std::vector<Browser*>* pBrowsers);

} // namespace BrowserUtil
} // namespace Zephyros

#endif

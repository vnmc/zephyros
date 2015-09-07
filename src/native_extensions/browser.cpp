#include <algorithm>
#include "native_extensions/browser.h"
#include "zephyros.h"


namespace Zephyros {
namespace BrowserUtil {

Browser* GetDefaultBrowser(std::vector<Browser*>* pBrowsers)
{
	FindBrowsers(&pBrowsers);

    Browser* firstBrowser = NULL;
    for (std::vector<Browser*>::iterator it = pBrowsers->begin(); it != pBrowsers->end(); ++it)
    {
        if (firstBrowser == NULL)
            firstBrowser = *it;
        if ((*it)->IsDefaultBrowser())
            return *it;
    }
    
    return firstBrowser;
}

Browser* GetBrowserForIdentifier(std::vector<Browser*>* pBrowsers, String identifier)
{
	FindBrowsers(&pBrowsers);

    for (std::vector<Browser*>::iterator it = pBrowsers->begin(); it != pBrowsers->end(); ++it)
        if ((*it)->GetIdentifier() == identifier)
            return *it;
    return NULL;
}

Browser* GetBrowserForUserAgent(std::vector<Browser*>* pBrowsers, JavaScript::Object userAgent)
{
	FindBrowsers(&pBrowsers);

    String strUserAgent = userAgent->GetString("name");
    String strUserAgentLc = String(strUserAgent);
    std::transform(strUserAgentLc.begin(), strUserAgentLc.end(), strUserAgentLc.begin(), ::tolower);
    
    String strUserAgentIdLc = userAgent->GetString("id");
    std::transform(strUserAgentIdLc.begin(), strUserAgentIdLc.end(), strUserAgentIdLc.begin(), ::tolower);
    
    for (std::vector<Browser*>::iterator it = pBrowsers->begin(); it != pBrowsers->end(); ++it)
    {
		Browser* browser = *it;
        
        String name = browser->GetName();
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        
        String identifier = browser->GetIdentifier();
        std::transform(identifier.begin(), identifier.end(), identifier.begin(), ::tolower);
        
        if (name.find(strUserAgentLc) != String::npos || identifier.find(strUserAgentLc) != String::npos ||
            (strUserAgent == TEXT("IE") && (name.find(TEXT("internet explorer")) != String::npos || identifier.find(TEXT("internet explorer")) != String::npos)))
        {
#ifdef OS_MACOSX
            if (identifier.find(TEXT("parallels")) != String::npos)
            {
                // is a browser installed on Parallels
                // reject Mac OS user agents (assuming that Parallels is not running Mac OS)
                if (strUserAgentIdLc.find(TEXT("mac_os")) != String::npos)
                    continue;
            }
            else
            {
                // native browser
                // reject if the user agent is not on Mac OS
                if (strUserAgentIdLc.find(TEXT("mac_os")) == String::npos)
                    continue;
            }
#endif
            return browser;
        }
    }
    
    return NULL;
}

Browser* GetBrowserFromJSRepresentation(std::vector<Browser*>* pBrowsers, JavaScript::Object obj)
{
	FindBrowsers(&pBrowsers);

    String identifier = obj->GetString("id");
    Browser* browser = GetBrowserForIdentifier(pBrowsers, identifier);
    if (browser != NULL)
        return browser;
    
    String name = obj->GetString("name");
    for (std::vector<Browser*>::iterator it = pBrowsers->begin(); it != pBrowsers->end(); ++it)
        if ((*it)->GetName() == name)
            return *it;
    
    return NULL;
}

bool IsKnownBrowser(String browserName)
{
    // http://en.wikipedia.org/wiki/List_of_web_browsers
    std::vector<String> knownBrowsers;
    knownBrowsers.push_back(TEXT("360 secure browser"));
    knownBrowsers.push_back(TEXT("aol browser"));
    knownBrowsers.push_back(TEXT("bento browser"));
    knownBrowsers.push_back(TEXT("greenbrowser"));
    knownBrowsers.push_back(TEXT("internet explorer"));
    knownBrowsers.push_back(TEXT("mediabrowser"));
    knownBrowsers.push_back(TEXT("menubox"));
    knownBrowsers.push_back(TEXT("msn explorer"));
    knownBrowsers.push_back(TEXT("neoplanet"));
    knownBrowsers.push_back(TEXT("netcaptor"));
    knownBrowsers.push_back(TEXT("realplayer"));
    knownBrowsers.push_back(TEXT("sitekiosk"));
    knownBrowsers.push_back(TEXT("slimbrowser"));
    knownBrowsers.push_back(TEXT("tencent traveler"));
    knownBrowsers.push_back(TEXT("tomeraider"));
    knownBrowsers.push_back(TEXT("ultrabrowser"));
    knownBrowsers.push_back(TEXT("webbie"));
                               
    knownBrowsers.push_back(TEXT("firefox"));
    knownBrowsers.push_back(TEXT("netscape"));
    knownBrowsers.push_back(TEXT("epic"));
    knownBrowsers.push_back(TEXT("iceweasel"));
    knownBrowsers.push_back(TEXT("icecat"));
    knownBrowsers.push_back(TEXT("pale moon"));
    knownBrowsers.push_back(TEXT("at&t pogo"));
    knownBrowsers.push_back(TEXT("flock"));
    knownBrowsers.push_back(TEXT("swiftfox"));
    knownBrowsers.push_back(TEXT("swiftweasel"));
    knownBrowsers.push_back(TEXT("xb browser"));
    knownBrowsers.push_back(TEXT("skyfire"));
    knownBrowsers.push_back(TEXT("camino"));
    knownBrowsers.push_back(TEXT("conkeror"));
    knownBrowsers.push_back(TEXT("seamonkey"));
    knownBrowsers.push_back(TEXT("iceape"));
    knownBrowsers.push_back(TEXT("classilla"));
    knownBrowsers.push_back(TEXT("gnuzilla"));
    knownBrowsers.push_back(TEXT("beonex"));
    knownBrowsers.push_back(TEXT("yahoo!"));
    knownBrowsers.push_back(TEXT("galeon"));
    knownBrowsers.push_back(TEXT("k-meleon"));
    knownBrowsers.push_back(TEXT("k-ninja"));
    knownBrowsers.push_back(TEXT("microb"));
    knownBrowsers.push_back(TEXT("minimo"));
                               
    knownBrowsers.push_back(TEXT("baidu"));
    knownBrowsers.push_back(TEXT("maxthon"));
    knownBrowsers.push_back(TEXT("sleipnir"));
    knownBrowsers.push_back(TEXT("avant browser"));
    knownBrowsers.push_back(TEXT("lunascape"));
    knownBrowsers.push_back(TEXT("konqueror"));
    knownBrowsers.push_back(TEXT("internet channel"));
    knownBrowsers.push_back(TEXT("nintendo ds browser"));
    knownBrowsers.push_back(TEXT("opera"));
                               
    knownBrowsers.push_back(TEXT("amazon kindle"));
    knownBrowsers.push_back(TEXT("arora"));
    knownBrowsers.push_back(TEXT("bolt browser"));
    knownBrowsers.push_back(TEXT("chromium"));
    knownBrowsers.push_back(TEXT("google chrome"));
    knownBrowsers.push_back(TEXT("amigo"));
    knownBrowsers.push_back(TEXT("torch browser"));
    knownBrowsers.push_back(TEXT("comodo dragon"));
    knownBrowsers.push_back(TEXT("qip surf"));
    knownBrowsers.push_back(TEXT("nichrome"));
    knownBrowsers.push_back(TEXT("srware iron"));
    knownBrowsers.push_back(TEXT("rockmelt"));
    knownBrowsers.push_back(TEXT("uran browser"));
    knownBrowsers.push_back(TEXT("yandex browser"));
    knownBrowsers.push_back(TEXT("dolphin browser"));
    knownBrowsers.push_back(TEXT("dooble"));
    knownBrowsers.push_back(TEXT("icab"));
    knownBrowsers.push_back(TEXT("iris browser"));
    knownBrowsers.push_back(TEXT("midori"));
    knownBrowsers.push_back(TEXT("nintendo 3ds netfront"));
    knownBrowsers.push_back(TEXT("omniweb"));
    knownBrowsers.push_back(TEXT("owb"));
    knownBrowsers.push_back(TEXT("qupzilla"));
    knownBrowsers.push_back(TEXT("rekonq"));
    knownBrowsers.push_back(TEXT("safari"));
    knownBrowsers.push_back(TEXT("shiira"));
    knownBrowsers.push_back(TEXT("steel"));
    knownBrowsers.push_back(TEXT("steam"));
    knownBrowsers.push_back(TEXT("teashark"));
    knownBrowsers.push_back(TEXT("uzbl"));
    knownBrowsers.push_back(TEXT("webpositive"));
    knownBrowsers.push_back(TEXT("xombrero"));
    knownBrowsers.push_back(TEXT("thunderhawk"));
    knownBrowsers.push_back(TEXT("hotjava"));
    knownBrowsers.push_back(TEXT("uzard web"));
    knownBrowsers.push_back(TEXT("ucweb"));
    knownBrowsers.push_back(TEXT("htmlunit"));
                               
    knownBrowsers.push_back(TEXT("gollum browser"));
    knownBrowsers.push_back(TEXT("image xplorer"));
    knownBrowsers.push_back(TEXT("kirix strata"));
    knownBrowsers.push_back(TEXT("miro"));
    knownBrowsers.push_back(TEXT("songbird"));
    knownBrowsers.push_back(TEXT("spacetime"));
    knownBrowsers.push_back(TEXT("wyzo"));
    knownBrowsers.push_back(TEXT("zac browser"));
    knownBrowsers.push_back(TEXT("epic browser"));
    knownBrowsers.push_back(TEXT("ghostzilla"));
    knownBrowsers.push_back(TEXT("prodigy classic"));
                               
    knownBrowsers.push_back(TEXT("amosaic"));
    knownBrowsers.push_back(TEXT("ibm webexplorer"));
    knownBrowsers.push_back(TEXT("internet in a box"));
    knownBrowsers.push_back(TEXT("mosaic-ck"));
    knownBrowsers.push_back(TEXT("spyglass mosaic"));
    knownBrowsers.push_back(TEXT("vms mosaic"));
                               
    knownBrowsers.push_back(TEXT("abaco"));
    knownBrowsers.push_back(TEXT("amaya"));
    knownBrowsers.push_back(TEXT("arachne"));
    knownBrowsers.push_back(TEXT("arena"));
    knownBrowsers.push_back(TEXT("ariadna"));
    knownBrowsers.push_back(TEXT("aweb"));
    knownBrowsers.push_back(TEXT("charon"));
    knownBrowsers.push_back(TEXT("dillo"));
    knownBrowsers.push_back(TEXT("dr-webspyder"));
    knownBrowsers.push_back(TEXT("embrowser"));
    knownBrowsers.push_back(TEXT("gazelle"));
    knownBrowsers.push_back(TEXT("ibrowse"));
    knownBrowsers.push_back(TEXT("mothra"));
    knownBrowsers.push_back(TEXT("netpositive"));
    knownBrowsers.push_back(TEXT("netsurf"));
    knownBrowsers.push_back(TEXT("planetweb"));
    knownBrowsers.push_back(TEXT("qihoo"));
    knownBrowsers.push_back(TEXT("phoenix"));
    knownBrowsers.push_back(TEXT("tkwww"));
    knownBrowsers.push_back(TEXT("voyager"));
    
    std::vector<String> knownNoBrowsers;
    knownNoBrowsers.push_back(TEXT("installer"));
    
    String browserNameLc = browserName;
    std::transform(browserNameLc.begin(), browserNameLc.end(), browserNameLc.begin(), ::tolower);
    
    for (std::vector<String>::iterator it = knownNoBrowsers.begin(); it != knownNoBrowsers.end(); ++it)
        if (browserNameLc.find(*it) != String::npos)
            return false;

    for (std::vector<String>::iterator it = knownBrowsers.begin(); it != knownBrowsers.end(); ++it)
        if (browserNameLc.find(*it) != String::npos)
            return true;

    return false;
}

} // namespace BrowserUtil
} // namespace Zephyros

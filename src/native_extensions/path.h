//
//  path.h
//  Ghostlab
//
//  Created by Matthias Christen on 10.09.13.
//
//

#ifndef Ghostlab_path_h
#define Ghostlab_path_h

#include "base/types.h"

#ifdef OS_WIN
#define PATH_SEPARATOR TEXT('\\')
#else
#define PATH_SEPARATOR TEXT('/')
#endif


namespace Zephyros {

class Path
{
public:
    Path()
        : m_path(TEXT("")), m_isDirectory(false), m_urlWithSecurityAccessData(TEXT("")), m_hasSecurityAccessData(false)
    {
    }
    
    Path(String path);

	Path(String path, String urlWithSecurityAccessData, bool hasSecurityAccessData);
    
    Path(JavaScript::Object jsonObj);
    
    Path& operator=(const Path& path)
    {
        m_path = path.GetPath();
		m_isDirectory = path.IsDirectory();
        m_urlWithSecurityAccessData = path.GetURLWithSecurityAccessData();
        m_hasSecurityAccessData = path.HasSecurityAccessData();
        
        return *this;
    }
    
    inline String GetPath() const
    {
        return m_path;
    }

	inline bool IsDirectory() const
	{
		return m_isDirectory;
	}
    
    inline String GetURLWithSecurityAccessData() const
    {
        return m_urlWithSecurityAccessData;
    }
    
    inline bool HasSecurityAccessData() const
    {
        return m_hasSecurityAccessData;
    }
    
    JavaScript::Object CreateJSRepresentation()
    {
        JavaScript::Object obj = JavaScript::CreateObject();
        
        obj->SetString("path", m_path);
		obj->SetBool("isDirectory", m_isDirectory);
        obj->SetString("url", m_urlWithSecurityAccessData);
        obj->SetBool("hasSecurityAccessData", m_hasSecurityAccessData);
        
        return obj;
    }
    
private:
    String m_path;
	bool m_isDirectory;
    String m_urlWithSecurityAccessData;
    bool m_hasSecurityAccessData;
};

} // namespace Zephyros

#endif

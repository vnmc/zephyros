#include "native_extensions/path.h"
#include "native_extensions/file_util.h"

namespace Zephyros {

Path::Path(String path)
    : m_path(path), m_urlWithSecurityAccessData(TEXT("")), m_hasSecurityAccessData(false)
{
	m_isDirectory = FileUtil::IsDirectory(path);
}
    
Path::Path(String path, String urlWithSecurityAccessData, bool hasSecurityAccessData)
    : m_path(path), m_urlWithSecurityAccessData(urlWithSecurityAccessData), m_hasSecurityAccessData(hasSecurityAccessData)
{
	m_isDirectory = FileUtil::IsDirectory(path);
}

Path::Path(JavaScript::Object jsonObj)
{
    m_path = jsonObj->GetString("path");
	m_isDirectory = FileUtil::IsDirectory(m_path);
    m_urlWithSecurityAccessData = jsonObj->GetString("url");
    m_hasSecurityAccessData = jsonObj->GetBool("hasSecurityAccessData");
}

JavaScript::Object Path::CreateJSRepresentation()
{
    JavaScript::Object obj = JavaScript::CreateObject();
        
    obj->SetString("path", m_path);
    obj->SetBool("isDirectory", m_isDirectory);
    obj->SetString("url", m_urlWithSecurityAccessData);
    obj->SetBool("hasSecurityAccessData", m_hasSecurityAccessData);
        
    return obj;
}

} // namespace Zephyros
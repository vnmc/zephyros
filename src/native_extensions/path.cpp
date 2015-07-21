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

} // namespace Zephyros
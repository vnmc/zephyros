/*******************************************************************************
 * Copyright (c) 2015-2016 Vanamco AG, http://www.vanamco.com
 *
 * The MIT License (MIT)
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Contributors:
 * Matthias Christen, Vanamco AG
 *******************************************************************************/


#import "base/types.h"
#import "native_extensions/os_util.h"
#import "jsbridge.h"


namespace Zephyros {
namespace OSUtil {
    
String GetOSVersion()
{
    return "";
}

String GetUserName()
{
    return "";
}
    
String GetHomeDirectory()
{
    return "";
}
    
String GetComputerName()
{
    return "";
}
 
void StartProcess(CallbackId callback, String executableFileName, std::vector<String> arguments, String cwd)
{
}
 
void SetWindowSize(CallbackId callback, int width, int height, bool hasWidth, bool hasHeight, int* pNewWidth, int* pNewHeight)
{
}
    
    
void SetMinimumWindowSize(int width, int height)
{
}
    
    
void DisplayNotification(String title, String details)
{
}
    
void RequestUserAttention()
{
}
    
void CreateMenu(JavaScript::Array menuItems)
{
}
    
void RemoveMenuItem(String strCommandId)
{
}
    
MenuHandle CreateContextMenu(JavaScript::Array menuItems)
{
    return 0;
}
    
String ShowContextMenu(MenuHandle nMenuHandle, int x, int y)
{
    return "";
}
    
void CopyToClipboard(String text)
{
}

void CleanUp()
{
}

} // namespace OSUtil
} // namespace Zephyros


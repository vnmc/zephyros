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


#include <map>

#include "zephyros.h"
#include "base/types.h"


namespace Zephyros {
    
std::map<String, String> g_mimeTypes;


void InitializeMIMETypes()
{
    g_mimeTypes[TEXT(".html")] = TEXT("text/html");
    g_mimeTypes[TEXT(".htm")] = TEXT("text/html");
    g_mimeTypes[TEXT(".css")] = TEXT("text/css");
    g_mimeTypes[TEXT(".json")] = TEXT("application/json");
    g_mimeTypes[TEXT(".js")] = TEXT("text/javascript");
    g_mimeTypes[TEXT(".xml")] = TEXT("application/xml");
    g_mimeTypes[TEXT(".txt")] = TEXT("text/plain");
    g_mimeTypes[TEXT(".log")] = TEXT("text/plain");
    
    // fonts
    g_mimeTypes[TEXT(".pdf")] = TEXT("application/pdf");
    g_mimeTypes[TEXT(".otf")] = TEXT("application/x-font-otf");
    g_mimeTypes[TEXT(".ttf")] = TEXT("application/x-font-ttf");
    g_mimeTypes[TEXT(".pfa")] = TEXT("application/x-font-type1");
    g_mimeTypes[TEXT(".pfb")] = TEXT("application/x-font-type1");
    g_mimeTypes[TEXT(".pfm")] = TEXT("application/x-font-type1");
    g_mimeTypes[TEXT(".afm")] = TEXT("application/x-font-type1");
    g_mimeTypes[TEXT(".woff")] = TEXT("application/font-woff");
    
    // images
    g_mimeTypes[TEXT(".bmp'")] = TEXT("image/bmp");
    g_mimeTypes[TEXT(".cgm")] = TEXT("image/cgm");
    g_mimeTypes[TEXT(".gif")] = TEXT("image/gif");
    g_mimeTypes[TEXT(".ief")] = TEXT("image/ief");
    g_mimeTypes[TEXT(".jpeg")] = TEXT("image/jpeg");
    g_mimeTypes[TEXT(".jpg")] = TEXT("image/jpeg");
    g_mimeTypes[TEXT(".jpe")] = TEXT("image/jpeg");
    g_mimeTypes[TEXT(".png")] = TEXT("image/png");
    g_mimeTypes[TEXT(".sgi")] = TEXT("image/sgi");
    g_mimeTypes[TEXT(".svg")] = TEXT("image/svg+xml");
    g_mimeTypes[TEXT(".svgz")] = TEXT("image/svg+xml");
    g_mimeTypes[TEXT(".tiff")] = TEXT("image/tiff");
    g_mimeTypes[TEXT(".tif")] = TEXT("image/tiff");
    
    // audio
    g_mimeTypes[TEXT(".au")] = TEXT("audio/basic");
    g_mimeTypes[TEXT(".snd")] = TEXT("audio/basic");
    g_mimeTypes[TEXT(".mid")] = TEXT("audio/midi");
    g_mimeTypes[TEXT(".midi")] = TEXT("audio/midi");
    g_mimeTypes[TEXT(".rmi")] = TEXT("audio/midi");
    g_mimeTypes[TEXT(".mp4a")] = TEXT("audio/mp4");
    g_mimeTypes[TEXT(".mpga")] = TEXT("audio/mpeg");
    g_mimeTypes[TEXT(".mp2")] = TEXT("audio/mpeg");
    g_mimeTypes[TEXT(".mp2a")] = TEXT("audio/mpeg");
    g_mimeTypes[TEXT(".mp3")] = TEXT("audio/mpeg");
    g_mimeTypes[TEXT(".m2a")] = TEXT("audio/mpeg");
    g_mimeTypes[TEXT(".m3a")] = TEXT("audio/mpeg");
    g_mimeTypes[TEXT(".oga")] = TEXT("audio/ogg");
    g_mimeTypes[TEXT(".ogg")] = TEXT("audio/ogg");
    g_mimeTypes[TEXT(".spx")] = TEXT("audio/ogg");
    g_mimeTypes[TEXT(".s3m")] = TEXT("audio/s3m");
    g_mimeTypes[TEXT(".sil")] = TEXT("audio/silk");
    g_mimeTypes[TEXT(".rip")] = TEXT("audio/vnd.rip");
    g_mimeTypes[TEXT(".weba")] = TEXT("audio/webm");
    g_mimeTypes[TEXT(".aac")] = TEXT("audio/x-aac");
    g_mimeTypes[TEXT(".aif")] = TEXT("audio/x-aiff");
    g_mimeTypes[TEXT(".aiff")] = TEXT("audio/x-aiff");
    g_mimeTypes[TEXT(".aifc")] = TEXT("audio/x-aiff");
    g_mimeTypes[TEXT(".caf")] = TEXT("audio/x-caf");
    g_mimeTypes[TEXT(".flac")] = TEXT("audio/x-flac");
    g_mimeTypes[TEXT(".m3u")] = TEXT("audio/x-mpegurl");
    g_mimeTypes[TEXT(".wax")] = TEXT("audio/x-ms-wax");
    g_mimeTypes[TEXT(".wma")] = TEXT("audio/x-ms-wma");
    g_mimeTypes[TEXT(".ram")] = TEXT("audio/x-pn-realaudio");
    g_mimeTypes[TEXT(".ra")] = TEXT("audio/x-pn-realaudio");
    g_mimeTypes[TEXT(".rmp")] = TEXT("audio/x-pn-realaudio-plugin");
    g_mimeTypes[TEXT(".wav")] = TEXT("audio/x-wav");
    g_mimeTypes[TEXT(".xm")] = TEXT("audio/xm");
    
    // video
    g_mimeTypes[TEXT(".3gp")] = TEXT("video/3gpp");
    g_mimeTypes[TEXT(".3g2")] = TEXT("video/3gpp2");
    g_mimeTypes[TEXT(".h261")] = TEXT("video/h261");
    g_mimeTypes[TEXT(".h263")] = TEXT("video/h263");
    g_mimeTypes[TEXT(".h264")] = TEXT("video/h264");
    g_mimeTypes[TEXT(".jpgv")] = TEXT("video/jpeg");
    g_mimeTypes[TEXT(".jpm")] = TEXT("video/jpm");
    g_mimeTypes[TEXT(".jpgm")] = TEXT("video/jpm");
    g_mimeTypes[TEXT(".mj2")] = TEXT("video/mj2");
    g_mimeTypes[TEXT(".mjp2")] = TEXT("video/mj2");
    g_mimeTypes[TEXT(".mp4")] = TEXT("video/mp4");
    g_mimeTypes[TEXT(".mp4v")] = TEXT("video/mp4");
    g_mimeTypes[TEXT(".mpg4")] = TEXT("video/mp4");
    g_mimeTypes[TEXT(".mpeg")] = TEXT("video/mpeg");
    g_mimeTypes[TEXT(".mpg")] = TEXT("video/mpeg");
    g_mimeTypes[TEXT(".mpe")] = TEXT("video/mpeg");
    g_mimeTypes[TEXT(".m1v")] = TEXT("video/mpeg");
    g_mimeTypes[TEXT(".m2v")] = TEXT("video/mpeg");
    g_mimeTypes[TEXT(".ogv")] = TEXT("video/ogg");
    g_mimeTypes[TEXT(".qt")] = TEXT("video/quicktime");
    g_mimeTypes[TEXT(".mov")] = TEXT("video/quicktime");
    g_mimeTypes[TEXT(".mxu")] = TEXT("video/vnd.mpegurl");
    g_mimeTypes[TEXT(".m4u")] = TEXT("video/vnd.mpegurl");
    g_mimeTypes[TEXT(".pyv")] = TEXT("video/vnd.ms-playready.media.pyv");
    g_mimeTypes[TEXT(".uvu")] = TEXT("video/vnd.uvvu.mp4");
    g_mimeTypes[TEXT(".uvvu")] = TEXT("video/vnd.uvvu.mp4");
    g_mimeTypes[TEXT(".viv")] = TEXT("video/vnd.vivo");
    g_mimeTypes[TEXT(".webm")] = TEXT("video/webm");
    g_mimeTypes[TEXT(".f4v")] = TEXT("video/x-f4v");
    g_mimeTypes[TEXT(".fli")] = TEXT("video/x-fli");
    g_mimeTypes[TEXT(".flv")] = TEXT("video/x-flv");
    g_mimeTypes[TEXT(".m4v")] = TEXT("video/x-m4v");
    g_mimeTypes[TEXT(".mng")] = TEXT("video/x-mng");
    g_mimeTypes[TEXT(".asf")] = TEXT("video/x-ms-asf");
    g_mimeTypes[TEXT(".asx")] = TEXT("video/x-ms-asf");
    g_mimeTypes[TEXT(".vob")] = TEXT("video/x-ms-vob");
    g_mimeTypes[TEXT(".wm")] = TEXT("video/x-ms-wm");
    g_mimeTypes[TEXT(".wmv")] = TEXT("video/x-ms-wmv");
    g_mimeTypes[TEXT(".wmx")] = TEXT("video/x-ms-wmx");
    g_mimeTypes[TEXT(".wvx")] = TEXT("video/x-ms-wvx");
    g_mimeTypes[TEXT(".avi")] = TEXT("video/x-msvideo");
    g_mimeTypes[TEXT(".movie")] = TEXT("video/x-sgi-movie");
    g_mimeTypes[TEXT(".smv")] = TEXT("video/x-smv");
}
    
String GetMIMETypeForFilename(String filename)
{
    size_t pos = filename.rfind(TEXT('.'));
    String extension = pos == String::npos ? TEXT("") : filename.substr(pos);
    std::map<String, String>::iterator it = g_mimeTypes.find(extension);
    return it == g_mimeTypes.end() ? TEXT("application/octet-stream") : it->second;
}
    
}

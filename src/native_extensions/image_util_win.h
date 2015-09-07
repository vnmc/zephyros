//
//  FileUtilMac.h
//  Ghostlab
//
//  Created by Matthias Christen on 10.09.13.
//
//

#ifndef Ghostlab_image_util_win_h
#define Ghostlab_image_util_win_h


#include <Windows.h>
#include <minmax.h>
#include <gdiplus.h>

#include "include/cef_base.h"
#include "base/types.h"


namespace ImageUtil {

bool IconToPNG(HICON hIcon, BYTE** pData, DWORD* pLength);
bool IconToGrayscalePNG(HICON hIcon, BYTE** pData, DWORD* pLength);
bool ImageFileToPNG(String filename, BYTE** pData, DWORD* pLength);

bool BitmapToPNGData(Gdiplus::Bitmap* pBitmap, BYTE** pData, DWORD* pLength);
String Base64Encode(BYTE* data, DWORD length);
BYTE* Base64Decode(String strData, size_t* pLength);

HBITMAP Base64PNGDataToBitmap(String strData, SIZE size);

bool GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

} // namespace ImageUtil


#endif
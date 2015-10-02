/*******************************************************************************
 * Copyright (c) 2015 Vanamco AG, http://www.vanamco.com
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


#ifndef Zephyros_ImageUtilWin_h
#define Zephyros_ImageUtilWin_h


#include <Windows.h>
#include <minmax.h>
#include <objidl.h>
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


#endif // Zephyros_ImageUtilWin_h

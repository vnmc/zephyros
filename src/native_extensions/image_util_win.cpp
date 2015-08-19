#include <Shlwapi.h>

#include "util/base64.h"
#include "native_extensions/image_util_win.h"


namespace ImageUtil {

//
// Creates a GDI+ bitmap from a HICON.
// From http://majkel.wordpress.com/2008/02/29/getting-a-gdi-bitmap-from-an-hicon-with-proper-transparency/
// with some bug fixes.
//
Gdiplus::Bitmap* GetIconPixelData(HICON hIcon)
{
	ICONINFO iconInfo;
	GetIconInfo(hIcon, &iconInfo);

	BITMAP iconBmp;
	GetObject(iconInfo.hbmColor, sizeof(BITMAP), &iconBmp);
	
	Gdiplus::Bitmap* pBitmap = new Gdiplus::Bitmap(iconBmp.bmWidth, iconBmp.bmHeight, PixelFormat32bppARGB);
	bool hasAlpha = false;

	// We have to read the raw pixels of the bitmap to get proper transparency information
	// (not sure why, all we're doing is copying one bitmap into another)

	Gdiplus::Bitmap bmpColor(iconInfo.hbmColor, NULL);
	Gdiplus::BitmapData bmpData;
	Gdiplus::Rect bmBounds(0, 0, bmpColor.GetWidth(), bmpColor.GetHeight());

	bmpColor.LockBits(&bmBounds, Gdiplus::ImageLockModeRead, bmpColor.GetPixelFormat(), &bmpData);
	for (int y = 0; y < (int) bmpColor.GetHeight(); ++y)
	{
		Gdiplus::ARGB* pPixels = (Gdiplus::ARGB*) ((byte*) bmpData.Scan0 + y * bmpData.Stride);

		for (int x = 0; x < (int) bmpColor.GetWidth(); ++x, ++pPixels)
		{
			Gdiplus::Color color = Gdiplus::Color(*pPixels);
			pBitmap->SetPixel(x, y, color);
			hasAlpha = hasAlpha || (color.GetAlpha() > 0 && color.GetAlpha() < 255);
		}
	}
	bmpColor.UnlockBits(&bmpData);

	if (!hasAlpha)
	{
		// If there's no alpha transparency information, we need to use the mask to turn back on visible pixels
		Gdiplus::Bitmap bmpMask(iconInfo.hbmMask, NULL);
		Gdiplus::Color colorMask, colorBitmap;

		for (int y = 0; y < (int) bmpMask.GetHeight(); ++y)
		{
			for (int x = 0; x < (int) bmpMask.GetWidth(); ++x)
			{
				bmpMask.GetPixel(x, y, &colorMask);
				pBitmap->GetPixel(x, y, &colorBitmap);

				// make black pixels of the mask fully opaque (0xff) in the destination image,
				// and white pixels fully transparent (0x00)
				if (colorMask.GetValue() == 0)
				{
					// black pixel: make opaque (0xff)
					colorBitmap = Gdiplus::Color::MakeARGB(0xff, colorBitmap.GetRed(), colorBitmap.GetGreen(), colorBitmap.GetBlue());
				}
				else
				{
					// white pixel: make transparent (0x00)
					colorBitmap = Gdiplus::Color::MakeARGB(0x00, 0x00, 0x00, 0x00);
				}

				pBitmap->SetPixel(x, y, colorBitmap);
			}
		}
	}

	return pBitmap;
}

//
// Converts an icon to PNG data.
//
bool IconToPNG(HICON hIcon, BYTE** pData, DWORD* pLength)
{
	Gdiplus::Bitmap* pBitmap = GetIconPixelData(hIcon);
	BitmapToPNGData(pBitmap, pData, pLength);
	delete pBitmap;

	return *pData != NULL;
}

//
// Converts an icon to a grayscale image and returns the PNG data.
//
bool IconToGrayscalePNG(HICON hIcon, BYTE** pData, DWORD* pLength)
{
	Gdiplus::Bitmap* pBitmap = GetIconPixelData(hIcon);

	// convert to grayscale
	Gdiplus::ColorMatrix matrix = {
		0.299f, 0.299f, 0.299f, 0.000f, 0.000f,
		0.587f, 0.587f, 0.587f, 0.000f, 0.000f,
		0.114f, 0.114f, 0.114f, 0.000f, 0.000f,
		0.000f, 0.000f, 0.000f, 1.000f, 0.000f,
		0.000f, 0.000f, 0.000f, 0.000f, 1.000f
	};
	Gdiplus::ImageAttributes attrs;
	attrs.SetColorMatrix(&matrix);
	Gdiplus::Graphics* pGraphics = Gdiplus::Graphics::FromImage(pBitmap);
	Gdiplus::Rect rectDest(0, 0, pBitmap->GetWidth(), pBitmap->GetHeight());
	pGraphics->DrawImage(pBitmap, rectDest, 0, 0, pBitmap->GetWidth(), pBitmap->GetHeight(), Gdiplus::Unit::UnitPixel, &attrs);
	delete pGraphics;

	BitmapToPNGData(pBitmap, pData, pLength);
	delete pBitmap;

	return *pData != NULL;
}

bool ImageFileToPNG(String filename, BYTE** pData, DWORD* pLength)
{
	*pData = NULL;

	Gdiplus::Bitmap bitmap(filename.c_str());

	if (bitmap.GetLastStatus() == Gdiplus::Ok)
		BitmapToPNGData(&bitmap, pData, pLength);

	return *pData != NULL;
}

//
// Gets the encoder CLSID for a particular image mime type.
//
bool GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	// number of image encoders
	UINT num = 0;

	// size of the image encoder array in bytes
	UINT size = 0;
	
	Gdiplus::GetImageEncodersSize(&num, &size);
	if (size == 0)
		return false;

	Gdiplus::ImageCodecInfo* pImageCodecInfo = (Gdiplus::ImageCodecInfo*) (malloc(size));
	if(pImageCodecInfo == NULL)
		return false;

	Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);
	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return true;
		}
	}

	free(pImageCodecInfo);
	return false;
}

//
// Returns PNG data from a GDI+ Bitmap.
//
bool BitmapToPNGData(Gdiplus::Bitmap* pBitmap, BYTE** pData, DWORD* pLength)
{
	*pData = NULL;
	*pLength = 0;

	CLSID clsidEncoder;
	if (!GetEncoderClsid(L"image/png", &clsidEncoder))
		return false;

	// for debugging -->
	//Gdiplus::Status status = pBitmap->Save(L"C:\\Users\\ghost\\Documents\\tmp.png", &clsidEncoder);
	// <--

	// NOTE: SHCreateMemStream will only work in Win Vista or later (or import from Shlwapi.dll as ordinal 12)

	// save the image data to a memory stream
	IStream* pStream = SHCreateMemStream(NULL, 0);
	pBitmap->Save(pStream, &clsidEncoder);

	// retrieve the data
	STATSTG stat = { 0 };
	pStream->Stat(&stat, STATFLAG_NONAME);
	_ASSERT(stat.cbSize.HighPart == 0);
	*pData = new BYTE[stat.cbSize.LowPart];

	LARGE_INTEGER pos;
	pos.QuadPart = 0;
	pStream->Seek(pos, STREAM_SEEK_SET, NULL);
		
	pStream->Read(*pData, stat.cbSize.LowPart, pLength);

	return *pData != NULL;
}

HBITMAP Base64PNGDataToBitmap(String strData, SIZE size)
{
	// make sure that strData is a data URL with a base64-encoded PNG
	String mime = TEXT("data:image/png;base64,");
	if (strData.substr(0, mime.length()) != mime)
		return NULL;

	// create a memory stream to read from
	size_t len = 0;
	BYTE* pData = Base64Decode(strData.substr(mime.length()), &len);
	IStream* pStream = SHCreateMemStream(pData, (UINT) len);

	// read the image from the stream
	Gdiplus::Bitmap bitmap(pStream);
	Gdiplus::Bitmap* pBitmapResized = NULL;

	// if the size isn't 0, resize the image
	if (size.cx > 0 && size.cy > 0 && (UINT) size.cx != bitmap.GetWidth() && (UINT) size.cy != bitmap.GetHeight())
	{
		// resize the image
		pBitmapResized = new Gdiplus::Bitmap(size.cx, size.cy, bitmap.GetPixelFormat());
		Gdiplus::Graphics g(pBitmapResized);
		g.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeHighQuality);
		g.SetInterpolationMode(Gdiplus::InterpolationMode::InterpolationModeHighQualityBicubic);
		g.DrawImage(&bitmap, 0, 0, size.cx, size.cy);
	}
	else
	{
		size.cx = bitmap.GetWidth();
		size.cy = bitmap.GetHeight();
	}

	Gdiplus::Bitmap* pBitmap = pBitmapResized == NULL ? &bitmap : pBitmapResized;

	// prepare a BITMAPINFO structure to create the DIB
	HBITMAP hBmp = NULL;
	BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biWidth = size.cx;
    bmi.bmiHeader.biHeight = size.cy;
    bmi.bmiHeader.biBitCount = 32;

    HDC hdc = GetDC(NULL);
    if (hdc)
    {
		// create the DIB
		BYTE* pBits = NULL;
        hBmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, reinterpret_cast<void**>(&pBits), NULL, 0);
		ReleaseDC(NULL, hdc);

		// copy the image data into the DIB bits
		Gdiplus::BitmapData bmpData;
		Gdiplus::Rect rect(0, 0, size.cx, size.cy);
		pBitmap->LockBits(&rect, Gdiplus::ImageLockMode::ImageLockModeRead, PixelFormat32bppPARGB, &bmpData);
		int ptr = 0;
		for (int y = size.cy - 1; y >= 0; --y)
		{
			Gdiplus::ARGB* pPixels = (Gdiplus::ARGB*) ((byte*) bmpData.Scan0 + y * bmpData.Stride);
			for (int x = 0; x < size.cx; ++x, ++pPixels, ptr += 4)
				*((DWORD*) &(pBits[ptr])) = *pPixels;
		}
		pBitmap->UnlockBits(&bmpData);
    }

	// clean up
	delete[] pData;
	if (pBitmapResized != NULL)
		delete pBitmapResized;

	return hBmp;
}

//
// Base64-encodes data.
//
String Base64Encode(BYTE* data, DWORD length)
{
	if (data == NULL)
		return TEXT("");

	size_t outLength;
	char* outBuf = NewBase64Encode(data, length, false, &outLength);
	String res = String(outBuf, outBuf + outLength);
	free(outBuf);

	return res;
}

BYTE* Base64Decode(String strData, size_t* pLength)
{
	std::string strInData = std::string(strData.begin(), strData.end());
	return (BYTE*) NewBase64Decode(strInData.c_str(), strInData.length(), pLength);
}

} // namespace ImageUtil
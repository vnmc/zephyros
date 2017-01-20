#ifndef Zephyros_Dropsource_h
#define Zephyros_Dropsource_h
#pragma once


HRESULT CreateDropSource(IDropSource** ppDropSource);


class CDropSource : public IDropSource
{
public:
	//
    // IUnknown members
	//
    HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject);
    ULONG __stdcall AddRef(void);
    ULONG __stdcall Release(void);
		
    //
	// IDropSource members
	//
    HRESULT __stdcall QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState);
	HRESULT __stdcall GiveFeedback(DWORD dwEffect);
	
	//
    // Constructor / Destructor
	//
    CDropSource();
    ~CDropSource();
	
private:
    //
	// private members and functions
	//
    LONG m_lRefCount;
};

#endif // Zephyros_Dropsource_h

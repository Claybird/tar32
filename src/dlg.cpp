#include "dlg.h"
#include "tar32res.h"
#include "tar32api.h" // EXTRACTINGINFOEX
#include <winuser.h>
#include <process.h>

CTar32StatusDialog::CTar32StatusDialog()
{
	m_cancel = false;
	m_hWnd = NULL;
	m_hThread = NULL;
}
CTar32StatusDialog::~CTar32StatusDialog()
{
	Destroy();
}
HWND CTar32StatusDialog::Create(HWND hParent)
{
	m_hParentWnd = hParent;
	HANDLE  hThread = (HANDLE)_beginthread(ThreadFunc,0,this);
	m_hThread = hThread;
	while(m_hWnd == NULL){
		Sleep(1);	// XXX busy wait....mmm
	}
	return m_hWnd;
}

/*static*/ /*DWORD*/ void _cdecl CTar32StatusDialog::ThreadFunc(LPVOID param)
{
	CTar32StatusDialog *pDlg = (CTar32StatusDialog *)param;
	extern HINSTANCE dll_instance;
	HWND hWnd = CreateDialogParam(dll_instance, MAKEINTRESOURCE(IDD_DIALOG_STATUS),pDlg->m_hParentWnd, (DLGPROC)WindowFunc, (long)pDlg); // ‚Ç‚ñ‚¼F(DLGPROC)‚ð’Ç‰Á
	//HWND hWnd = CreateDialogParam(dll_instance, MAKEINTRESOURCE(IDD_DIALOG_STATUS),NULL, WindowFunc, (long)pDlg);
	int ret;
	ret = ShowWindow(hWnd, SW_SHOW);
	ret = UpdateWindow(hWnd);
	pDlg->m_hWnd = hWnd;

	{
		MSG msg;
		while(GetMessage(&msg, 0, 0, 0)){
			if(hWnd==NULL || !IsDialogMessage(hWnd,&msg)){
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
	Sleep(0);
	//return 0;
}
/*static*/ BOOL CALLBACK CTar32StatusDialog::WindowFunc(HWND hWnd, UINT mes, WPARAM wParam, LPARAM lParam)
{
	extern UINT wm_arcextract;
	CTar32StatusDialog *pDlg = (CTar32StatusDialog*)GetWindowLong(hWnd, GWL_USERDATA);
	switch(mes){
	case WM_INITDIALOG:
		SetWindowLong(hWnd,GWL_USERDATA,lParam);
		return 1;
	case WM_DESTROY:
	//	EndDialog(hWnd,0);
		PostQuitMessage(0);
		return 1;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		return 1;
	case WM_COMMAND:
		switch(LOWORD(wParam)){
		case IDCANCEL:
			if(pDlg)pDlg->m_cancel = true;
			return 1;
			break;	
		}
		break;
	default:
		if(mes == wm_arcextract){
			EXTRACTINGINFOEX *pExtractingInfoEx = (EXTRACTINGINFOEX*)lParam;
			::SetDlgItemText(hWnd, IDC_FILENAME, pExtractingInfoEx->exinfo.szDestFileName);
			::SetDlgItemInt(hWnd, IDC_FILESIZE, pExtractingInfoEx->exinfo.dwWriteSize ,FALSE);

			extern HWND g_hwndOwnerWindow;
			extern ARCHIVERPROC *g_pArcProc;
			if(g_hwndOwnerWindow){
				LRESULT wndret = ::SendMessage(g_hwndOwnerWindow,mes,wParam,lParam);
				if(wndret != 0){
					pDlg->m_cancel = true;
				}
			}
			if(g_pArcProc){
				BOOL ProcRet = (*g_pArcProc)(g_hwndOwnerWindow, mes, wParam, pExtractingInfoEx);
				if(!ProcRet){
					pDlg->m_cancel = true;
				}
			}
			if(pDlg->m_cancel){
				ReplyMessage(1);
				return 1;
			}else{
				return 0;
			}
		}
	}
	return 0;
}

void CTar32StatusDialog::Destroy()
{
	if(m_hThread == NULL){return;}
	int ret;
	ret = SendMessage(m_hWnd, WM_DESTROY, 0, 0);
	DWORD code;
	// WaitForSingleObject() must call when m_hThread is alive.
	//ret = WaitForSingleObject(m_hThread,INFINITE);
	while(GetExitCodeThread(m_hThread,&code) == STILL_ACTIVE){
		Sleep(1);
	}
	//ret = WaitForSingleObject(m_hThread,INFINITE);
	m_hWnd = NULL;
	m_hThread = NULL;
}

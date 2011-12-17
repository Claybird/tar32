#include "stdafx.h"
#include "dlg.h"
#include "tar32res.h"
#include "tar32api.h" // EXTRACTINGINFOEX
#include "util.h"

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

void _cdecl CTar32StatusDialog::ThreadFunc(LPVOID param)
{
	CTar32StatusDialog *pDlg = (CTar32StatusDialog *)param;
	extern HINSTANCE dll_instance;
	HWND hWnd = CreateDialogParam(dll_instance, MAKEINTRESOURCE(IDD_DIALOG_STATUS),pDlg->m_hParentWnd, WindowFunc, (long)pDlg);
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

INT_PTR CALLBACK CTar32StatusDialog::WindowFunc(HWND hWnd, UINT mes, WPARAM wParam, LPARAM lParam)
{
	extern UINT wm_arcextract;
	CTar32StatusDialog *pDlg = (CTar32StatusDialog*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	switch(mes){
	case WM_INITDIALOG:
		SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
		::SendDlgItemMessage(hWnd,IDC_PROGRESS_FILE,PBM_SETRANGE32,0,100);
		EnableMenuItem(GetSystemMenu(hWnd,FALSE),SC_CLOSE, MF_GRAYED);	//閉じるボタンを無効に
		return 1;
	case WM_DESTROY:
//		return 0;
//		EndDialog(hWnd,0);
//		return 0;
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
			EXTRACTINGINFOEX64 *pExtractingInfoEx64 = (EXTRACTINGINFOEX64*)lParam;
			switch(wParam){
			case ARCEXTRACT_BEGIN:		//該当ファイルの処理の開始
				::SetDlgItemText(hWnd, IDC_STATIC_FILENAME, pExtractingInfoEx64->szDestFileName);
				{
					size64 size=pExtractingInfoEx64->llFileSize;
					HWND hProg=::GetDlgItem(hWnd,IDC_PROGRESS_FILE);
					LONG_PTR style=::GetWindowLongPtr(hProg,GWL_STYLE);
					if(size==(size64)-1){	//filesize unknown
						//プログレスバーをmarqueeスタイルに
						::SetWindowLongPtr(hProg,GWL_STYLE,style|PBS_MARQUEE);
						::SendMessage(hProg,PBM_SETMARQUEE,TRUE,30);
					}else{
						//プログレスバーを通常スタイルに
						::SetWindowLongPtr(hProg,GWL_STYLE,style&(~PBS_MARQUEE));
					}
				}
				break;
			case ARCEXTRACT_INPROCESS:	//該当ファイルの展開中
				//展開状況の設定
				{
					size64 size=pExtractingInfoEx64->llFileSize;
					size64 wrote=pExtractingInfoEx64->llWriteSize;
					if(size==(size64)-1){	//filesize unknown
						std::string strwrote=fsizeToString(wrote);
						char buf[256];
						_snprintf(buf,COUNTOF(buf),"wrote %s",strwrote.c_str());
						::SetDlgItemText(hWnd, IDC_STATIC_PROGRESS, buf);
						//::SendDlgItemMessage(hWnd,IDC_PROGRESS_FILE,PBM_SETPOS,10,0);
					}else{
						DWORD percent=(DWORD)(wrote*100/size);

						char buf[256];
						std::string strwrote=fsizeToString(wrote);
						std::string strsize=fsizeToString(size);
						_snprintf(buf,COUNTOF(buf),"%d%% (%s / %s)",percent,strwrote.c_str(),strsize.c_str());
						::SetDlgItemText(hWnd, IDC_STATIC_PROGRESS, buf);
						::SendDlgItemMessage(hWnd,IDC_PROGRESS_FILE,PBM_SETPOS,percent,0);
					}
				}
				break;
			case ARCEXTRACT_END:		//処理終了、関連メモリを開放
			case ARCEXTRACT_OPEN:		//該当書庫の処理の開始
			case ARCEXTRACT_COPY:		//ワークファイルの書き戻し
				//nothing to do
				break;
			}

#if 0
			// move to SendArcMessage() by tsuneo at 2001.11.20
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
#endif
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

	// WM_DESTROYの変わりにDestroyWindowを呼び出さないといけない。
	//ret = SendMessage(m_hWnd, WM_DESTROY, 0, 0);
	// しかしDestroyWindowは別スレッドからは送れない...
	//ret = DestroyWindow(m_hWnd);
	ret = SendMessage(m_hWnd, WM_CLOSE, 0, 0);	// 2000/03/03 by tsuneo
	DWORD code;
	// WaitForSingleObject() must call when m_hThread is alive.
	//ret = WaitForSingleObject(m_hThread,INFINITE);
	while(GetExitCodeThread(m_hThread,&code) && (code == STILL_ACTIVE)){
		Sleep(1);
	}
	//ret = WaitForSingleObject(m_hThread,INFINITE);
	m_hWnd = NULL;
	m_hThread = NULL;
}


//--------------
INT_PTR CALLBACK Tar32ConfirmOverwriteDialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg){
	case WM_INITDIALOG:
		{
			LPCTSTR pMsg=(LPCTSTR)lParam;
			if(pMsg){
				SetWindowText(GetDlgItem(hWnd,IDC_EDIT_STATUS),pMsg);
			}
		}
		return FALSE;
	case WM_COMMAND:
		switch(LOWORD(wParam)){
		case IDOK:
			//nothing to do
			break;
		case IDC_BUTTON_OVERWRITE:
		case IDC_BUTTON_OVERWRITE_ALL:
		case IDCANCEL:
			EndDialog(hWnd, LOWORD(wParam));
			break;
		default:
			return FALSE;
		}
	default:
		return FALSE;
	}
	return TRUE;
}


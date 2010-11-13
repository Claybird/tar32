class CTar32StatusDialog
{
public:
	CTar32StatusDialog();
	~CTar32StatusDialog();
	HWND Create(HWND hParent);
	void Destroy();
	bool is_cancel();
private:
	bool m_cancel;
	static void _cdecl ThreadFunc(LPVOID param);
	static INT_PTR CALLBACK WindowFunc(HWND hwnd, UINT mes, WPARAM wParam, LPARAM lParam);
	volatile HWND m_hWnd;
	HWND m_hParentWnd;
	HANDLE m_hThread;
};

//-------------------

//�㏑���m�F�_�C�A���O
INT_PTR CALLBACK Tar32ConfirmOverwriteDialogProc(HWND, UINT, WPARAM, LPARAM);

/*

  ������� Dataconf.exe �������� ��������� PPPoE ���������� �
  ������������ �������� ��������� Windows  ������� � Windows XP
  ��������� ���������
  zend.karabanov@gmail.com

  ��� ��������� �������� ��������� ����������� ������������:
  �� ������ �������������� � �/��� ��������������
  � ������������ � ��������� �������� GNU General Public License ������ 3
  ���� (�� ������ ������) ����� ����� ������� ������, ��������������
  Free Software Foundation.

  ��� ��������� ���������������� � ������� �� ��, ��� ��� ����� ��������,
  �� ��� �����-���� ��������, �� ����������� � �� ���� ����� � ����.
  �������� GNU General Public License ��� ����� ��������� ����������.

  �� ������ ���� �������� ����� GNU General Public License
  ������ � ���� ����������. ���� ���, ��. <http://www.gnu.org/licenses/>.

*/

/*

������ ����� ������ ��� http://habrahabr.ru/qa/39789/

����� ����������� ������ ���:

������ -> �������� -> �������� ������������ -> ����� -> ����� ������������ ��������� = Visual Studio 2012 - Windows XP (v110_xp)
������ -> �������� -> �������� ������������ -> ����� -> ����� �������� = ������������ ����� �������� �������
������ -> �������� -> �������� ������������ -> ����������� -> ���� -> �������������� ����������� = kernel32.lib;user32.lib;comctl32.lib;rasapi32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;odbc32.lib;odbccp32.lib;%(AdditionalDependencies)
������ -> �������� -> �������� ������������ -> ����������� -> ���� ��������� -> ��������� �������� = �� (/MANIFEST)
������ -> �������� -> �������� ������������ -> ����������� -> ���� ��������� -> �������������� ����������� ��������� = "type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'"
������ -> �������� -> �������� ������������ -> C/C++ -> �������� ���� -> ������������� (/MT)

*/

#define WINVER 0x0501
#define _WIN32_WINNT 0x0501

#include <windows.h>
#include <windowsx.h>
#include <winerror.h>

// ����� ����� ���� ��������� ������
#include <shellapi.h>
#include <shlobj.h>
#include <shlwapi.h>
#pragma comment(lib,"Shlwapi.lib")

// ����� ������ ���� ������ ��������
#include <string>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <tchar.h>

// ��� �� ����� ���� ������ ������������
#include <ras.h>
#include <rasdlg.h>
#include <raserror.h>
#pragma comment(lib, "rasapi32.lib")
#pragma comment(lib, "Mpr.lib")

// ����� ������ � ���� �������� ����� ����������
#include <commdlg.h>
#include <commctrl.h>
#pragma comment(lib,"comctl32.lib")
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// ����� ������ � ���� �������� ����� ����������
// ������ ����� ��� ������ �� ����� ������
void InitializeCommonControlsEx(VOID);

using namespace std;

#define CONN_NAME "Datacom_XXI"
#define LNK_NAME "Datacom_XXI.lnk"
#define SERVICE_NAME "datacom"

// ������ �������������� ��� ���������
#define ID_LOGO_STATIC 1
#define ID_LOGIN_STATIC 2
#define ID_LOGIN_EDIT 3
#define ID_PASSWORD_STATIC 4
#define ID_PASSWORD_EDIT 5
#define ID_CREATE_BUTTON 6
#define ID_LNK_CHECK 7
//#define ID_MESSAGE_STATIC 8

// ���������� ��������� �� ��������
int len_text;
static LPTSTR login=(LPTSTR)malloc (sizeof(TCHAR)*250);
static LPTSTR password=(LPTSTR)malloc (sizeof(TCHAR)*250);
HWND hWnd;
HWND login_static;
HWND login_edit;
HWND password_static;
HWND password_edit;
HWND create_button;
HWND lnk_check;
//HWND message_static;
HWND myPicture;
HBITMAP bmp;

// ��� ������� ������ PPPoE ����������
BOOL create_pppoe()
{
  // ����������� PPPoE
  RASENTRY RS;
  memset(&RS, 0, sizeof(RASENTRY));
  RS.dwSize = sizeof(RS);
  RS.dwfOptions =
  RASEO_PreviewUserPw |           // ����������� ��� ������������ � ������
  RASEO_ModemLights |             // ������� ������ � ������� �����������
  // RASEO_PreviewDomain |        // ���������� ���� "�����" � ���� ����� ������������ � ������ (������� "���������" - ���������� ������ "�������� ����� ����� � Windows")
  // RASEO_PreviewPhoneNumber |   // ���������� ���������� ������ "�������" � ���� ����� ������������ � ������
  RASEO_ShowDialingProgress |     // ������� "���������" - ��������� ������ ������: ���������� ������ "���������� ��� �����������"
  RASEO_RemoteDefaultGateway |    // ������� "����" - "�������� ��������� (TCP/IP)" - ������ "�������������" - ���������� ������ "������������ �������� ���� � ��������� ����"
  RASEO_SwCompression |           // ������� "����" - ������ "���������" - ���������� ������ "������������ ����������� ������ ������"
  // RASEO_RequirePAP |           // ������� "������������" - "�������������� (���������� ���������)" - ���������� ������ "��������������� ������ (PAP)"
  // RASEO_RequireSPAP |          // ������� "������������" - "�������������� (���������� ���������)" - ���������� ������ "�������� �������� ������ Shiva (SPAP)"
  RASEO_RequireCHAP |             // ������� "������������" - "�������������� (���������� ���������)" - ���������� ������ "�������� �������� ������ CHAP"
  // RASEO_RequireMsCHAP |        // ������� "������������" - "�������������� (���������� ���������)" - ���������� ������ "�������� �������� ������ Microsoft (MS-CHAP)"
  // RASEO_RequireW95MSCHAP |     // ������� "������������" - "�������������� (���������� ���������)" - ���������� ������ "��������� ������ �������� MS-CHAP ��� Windows 95"
  // RASEO_RequireMsCHAP2 |       // ������� "������������" - "�������������� (���������� ���������)" - ���������� ������ "�������� �������� ������ Microsoft (MS-CHAP v2)"
  RASEO_SecureLocalFiles;         // ������� "����" - ����������, ������������ ���� ������������: ����� ������ "������������ �������� ���� � ��������� ����";
  RS.dwfOptions2 = RASEO2_Internet | RASEO2_DontNegotiateMultilink | RASEO2_ReconnectIfDropped;
  RS.dwIdleDisconnectSeconds = RASIDS_Disabled; // �� ����������� ���� ��� ����������
  RS.dwfNetProtocols = RASNP_Ip;             // ��������� ���������
  RS.dwFramingProtocol = RASFP_Ppp;          // �������� Point-to-Point Protocol
  RS.dwType = RASET_Broadband;               // ��� �����������
  RS.dwVpnStrategy = VS_Default;             // ������� "����" - �� ���������
  RS.dwEncryptionType = ET_Optional;         // ��������� ���������� (ET_None, ET_Require, ET_RequireMax, ET_Optional)
  RS.dwDialMode = RASEDM_DialAll;
  RS.dwRedialCount = 99;                     // ����� �������� ������ ������
  RS.dwRedialPause = 60;                     // �������� ����� ������������
  lstrcpy(RS.szLocalPhoneNumber, _T(SERVICE_NAME)); // ������� "�����" - "��� ������"
  lstrcpy(RS.szDeviceType, RASDT_PPPoE);     // RASDT_PPPoE - ������������� ��� ����������� (PPPoE)
  lstrcpy(RS.szDeviceName, _T("PPPoE"));

  DWORD dwRet = RasSetEntryProperties(NULL,_T(CONN_NAME), &RS, sizeof(RS), NULL, 0);
  // ���� �� ������� ������� ����������� ���������� FALSE
  if(dwRet != ERROR_SUCCESS)
  {
    return FALSE;
  }

  // ����������� ����� � ����� (���� � Windows 7 ��������)
  RASCREDENTIALS RS_cred;
  memset(&RS_cred, 0, sizeof(RASCREDENTIALS));
  RS_cred.dwSize = sizeof(RS_cred);
  RS_cred.dwMask=RASCM_UserName | RASCM_Password;
  lstrcpy(RS_cred.szUserName, login);
  lstrcpy(RS_cred.szPassword, password);
  lstrcpy(RS_cred.szDomain,_T(""));

  DWORD dwCred = RasSetCredentials(NULL,_T(CONN_NAME),&RS_cred,FALSE);

  // ���� �� ������� �������� ����� � ������ ���������� FALSE
  if(dwCred != ERROR_SUCCESS)
  {
    return FALSE;
  }

  // ����������� ����� � ������
  // (� XP ����� ����������, ���� ��������� ����� � ������) 
  RASDIALPARAMS RS_param;
  memset(&RS_param, 0, sizeof(RASDIALPARAMS));
  RS_param.dwSize = sizeof(RS_param);
  lstrcpy(RS_param.szEntryName, _T(CONN_NAME));
  lstrcpy(RS_param.szUserName, login);
  lstrcpy(RS_param.szPassword, password);
  
  DWORD dwParam = RasSetEntryDialParams(NULL, &RS_param, FALSE);

  // ���� �� ������� �������� ����� � ������ ���������� FALSE
  if(dwParam != ERROR_SUCCESS)
  {
    return FALSE;
  }
  
  return TRUE;
}

// ���� ���� ��� ������� ������� ����� �� ������� ����
bool __fastcall CreateShortCut(LPWSTR pwzShortCutFileName,  LPCITEMIDLIST pidl,
                               LPTSTR pszWorkingDirectory, WORD wHotKey, int iCmdShow);
int short_cut_startup(TCHAR *connection_name, LPWSTR link_name);
LPITEMIDLIST GetNextItemID(LPCITEMIDLIST pidl);
UINT GetSize(LPCITEMIDLIST pidl);
LPITEMIDLIST Append(LPCITEMIDLIST pidlBase, LPCITEMIDLIST pidlAdd);

LPMALLOC pMalloc;

/* Main function which creating shortcut on desktop */
int short_cut_startup(TCHAR *connection_name, LPWSTR link_name)
{
    LPITEMIDLIST pidConnections = NULL;
    LPITEMIDLIST pidlItems = NULL;
    LPITEMIDLIST pidlDesk = NULL;
    IShellFolder *psfFirstFolder = NULL;
    IShellFolder *psfDeskTop = NULL;
    IShellFolder *pConnections = NULL;
    LPENUMIDLIST ppenum = NULL;
    ULONG celtFetched;
    HRESULT hr;
    STRRET str_curr_connection_name;
    TCHAR curr_connection_name[MAX_PATH] = _T("");  /* Connection point name */
    TCHAR desktop_path[MAX_PATH]=_T("");            /* Path to desktop */
    TCHAR full_link_name[MAX_PATH]=_T("");          /* Full shortcut name */
    LPITEMIDLIST full_pid;                          /* Full shortcut pid */
   
    CoInitialize( NULL );
    /* Allocating memory for Namespace objects */
    hr = SHGetMalloc(&pMalloc);
    hr = SHGetFolderLocation(NULL, CSIDL_CONNECTIONS, NULL, NULL, &pidConnections);

    /* Get full path to desktop */
    SHGetFolderPath(NULL, CSIDL_DESKTOP, NULL, 0, desktop_path);

    hr = SHGetDesktopFolder(&psfDeskTop);
    hr = psfDeskTop->BindToObject(pidConnections, NULL, IID_IShellFolder, (LPVOID *) &pConnections);
    hr = pConnections->EnumObjects(NULL,SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, &ppenum);

    /* Loop for searching our connection */
    while(hr = ppenum->Next(1,&pidlItems, &celtFetched) == S_OK && (celtFetched) == 1)
    {
        pConnections->GetDisplayNameOf(pidlItems, SHGDN_INFOLDER, &str_curr_connection_name);
        StrRetToBuf(&str_curr_connection_name, pidlItems, curr_connection_name, MAX_PATH);
        if(!_tcscmp(curr_connection_name,connection_name))
            goto found;
    }
    printf("Connection not found in \"Network connections\" folder.\n");
    return 0;
found:
    /* Append PIDLs */
    full_pid=Append(pidConnections,pidlItems);
    SetCurrentDirectory(desktop_path);
    if(!CreateShortCut(link_name, full_pid, _T("C:\\windows"), 0, SW_SHOWNORMAL))
        return 0;

    ppenum->Release();
    pMalloc->Free(pidlItems);
    pMalloc->Free(pidConnections);
    pMalloc->Release();
    pConnections->Release();
    CoUninitialize();
    return 1;
}

bool __fastcall CreateShortCut(LPWSTR pwzShortCutFileName,  LPCITEMIDLIST pidl,
                               LPTSTR pszWorkingDirectory, WORD wHotKey, int iCmdShow)
{ 
    IShellLink * pSL; 
    IPersistFile * pPF; 
    HRESULT hRes; 
    hRes = CoCreateInstance(CLSID_ShellLink, 0,CLSCTX_INPROC_SERVER, 
                            IID_IShellLink, (LPVOID *)&pSL); 
    if( SUCCEEDED(hRes) ) 
    { 
        hRes=pSL->SetIDList(pidl);
        if(SUCCEEDED(hRes))
        { 
            hRes = pSL->SetHotkey(wHotKey); 
            if( SUCCEEDED(hRes) ) 
            { 
                hRes = pSL->SetShowCmd(iCmdShow); 
                if( SUCCEEDED(hRes) ) 
                { 
                    hRes = pSL->QueryInterface(IID_IPersistFile,(LPVOID *)&pPF); 
                    if( SUCCEEDED(hRes) ) 
                    { 
                        hRes = pPF->Save(pwzShortCutFileName,TRUE); 
                        pPF->Release(); 
                    }
                }
            }
        }
        pSL->Release(); 
    }
    return SUCCEEDED(hRes); 
}

/******************************************************************************/
/* Functions copied from http://msdn.microsoft.com */

LPITEMIDLIST GetNextItemID(LPCITEMIDLIST pidl) 
{ 
   // Check for valid pidl.
   if(pidl == NULL)
      return NULL;

   // Get the size of the specified item identifier. 
   int cb = pidl->mkid.cb; 

   // If the size is zero, it is the end of the list. 

   if (cb == 0) 
      return NULL; 

   // Add cb to pidl (casting to increment by bytes). 
   pidl = (LPITEMIDLIST) (((LPBYTE) pidl) + cb); 

   // Return NULL if it is null-terminating, or a pidl otherwise. 
   return (pidl->mkid.cb == 0) ? NULL : (LPITEMIDLIST) pidl; 
} 

/* Get size of PIDL */
UINT GetSize(LPCITEMIDLIST pidl)
{
    UINT cbTotal = 0;
    if (pidl)
    {
        cbTotal += sizeof(pidl->mkid.cb);    // Terminating null character
        while (pidl)
        {
            cbTotal += pidl->mkid.cb;
            pidl = GetNextItemID(pidl);
        }
    }
    return cbTotal;
}

/* Appending PIDLs */
LPITEMIDLIST Append(LPCITEMIDLIST pidlBase, LPCITEMIDLIST pidlAdd)
{
    if(pidlBase == NULL)
        return NULL;
    if(pidlAdd == NULL)
        return (LPITEMIDLIST)pidlBase;
    
    LPITEMIDLIST pidlNew;

    UINT cb1 = GetSize(pidlBase) - sizeof(pidlBase->mkid.cb);
    UINT cb2 = GetSize(pidlAdd);

    pidlNew = (LPITEMIDLIST)pMalloc->Alloc(cb1 + cb2);
    if (pidlNew)
    {
        CopyMemory(pidlNew, pidlBase, cb1);
        CopyMemory(((LPSTR)pidlNew) + cb1, pidlAdd, cb2);
    }
    return pidlNew;
}

// ��� ������� �������� ����������� ���� � ������ �������
BOOL CenterWindow(HWND hwndWindow)
{
    RECT rectWindow;
 
    GetWindowRect(hwndWindow, &rectWindow);
 
    int nWidth = rectWindow.right - rectWindow.left;
    int nHeight = rectWindow.bottom - rectWindow.top;
    
    int nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    int nScreenHeight = GetSystemMetrics(SM_CYSCREEN);
 
    int nX = (nScreenWidth - nWidth) / 2;
    int nY = (nScreenHeight - nHeight) / 2;
  
     // �� ������ ������ �� ������� ���� ���� ��� ������� �� ������� ������
     // ������ ���� ��� ������� ������
     if (nX < 0) nX = 0;
     if (nY < 0) nY = 0;
     if (nX + nWidth > nScreenWidth) nX = nScreenWidth - nWidth;
     if (nY + nHeight > nScreenHeight) nY = nScreenHeight - nHeight;

     // ������� ���� � ����� ������
     if (MoveWindow(hwndWindow, nX, nY, nWidth, nHeight, FALSE))
     {
       return TRUE;
     }
     return FALSE;
}

// ��� ������
static TCHAR szWindowClass[] = _T("win32app");

// ������� � ��������� ���� ����� �����
static TCHAR szTitle[] = _T("��������� ����������� � Datacom");

HINSTANCE hInst;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX wcex;

    // ����� �������� ���� ���������
    InitCommonControls();

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance,_T("icon_datacom"));
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    //wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.hbrBackground  = HBRUSH(COLOR_BTNFACE + 1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(hInstance,_T("icon_datacom"));

    if (!RegisterClassEx(&wcex))
    {
        MessageBox(NULL, _T("Call to RegisterClassEx failed!"), _T("������"), NULL);
        return 1;
    }

    hInst = hInstance; // ���� ���� �������� ��������

    // ������ ������
    hWnd = CreateWindow(
        szWindowClass,
        szTitle,
        WS_BORDER|WS_MINIMIZEBOX|WS_SYSMENU,CW_USEDEFAULT,CW_USEDEFAULT,
        378, 342,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!hWnd)
    {
        MessageBox(NULL, _T("���-�� �� �������� :-("), _T("������"), NULL);
        return 1;
    }

    if (!CenterWindow(hWnd))
    {
        MessageBox(NULL, _T("��������� ����������� ���� � ������ ������"), _T("������"), NULL);
        return 1;
    }

    // ������ ����� �������� ������� ��������
    myPicture = CreateWindow(_T("static"),_T(""),SS_BITMAP|WS_CHILD|WS_VISIBLE|WS_BORDER,0,0,370,77,hWnd,(HMENU)ID_LOGO_STATIC,hInstance,NULL);
    bmp=LoadBitmap(hInstance, _T("logo_datacom"));
    SendMessage(myPicture,STM_SETIMAGE,IMAGE_BITMAP,(LPARAM)bmp);

    // ������ ����� ������
    login_static=CreateWindow(_T("static"),_T("��� ������������:"),WS_VISIBLE|WS_CHILD|SS_LEFT,61,91,250,25,hWnd,(HMENU)ID_LOGIN_STATIC,hInstance,NULL);

    // ������ ��������� ���� ��� ����� ������
    login_edit=CreateWindow(_T("edit"),_T(""),WS_TABSTOP|ES_AUTOHSCROLL|WS_VISIBLE|WS_CHILD|WS_BORDER,62,112,250,25,hWnd,(HMENU)ID_LOGIN_EDIT,hInstance,NULL);

    // ������ ����� �� ��������� ���� ��� ����� ������
    SetFocus(login_edit);

    // ������ ����� ������
    password_static=CreateWindow(_T("static"),_T("������:"),WS_VISIBLE|WS_CHILD|SS_LEFT,61,156,250,25,hWnd,(HMENU)ID_PASSWORD_STATIC,hInstance,NULL);

    // ������ ��������� ���� ��� ����� ������
    password_edit=CreateWindow(_T("edit"),_T(""),WS_TABSTOP|ES_AUTOHSCROLL|WS_VISIBLE|WS_CHILD|WS_BORDER,62,177,250,25,hWnd,(HMENU)ID_PASSWORD_EDIT,hInstance,NULL);

    // ������ ������ "��������� �����������"
    create_button=CreateWindow(_T("button"),_T("��������� �����������"),WS_TABSTOP|WS_CHILD|BS_PUSHBUTTON|WS_VISIBLE,61,221,252,40,hWnd,(HMENU)ID_CREATE_BUTTON,NULL,NULL);

    // ������ ������� "�������� ����� �� �������� ����"
    lnk_check=CreateWindow(_T("button"), _T("�������� ����� �� ������� ����"), WS_TABSTOP|WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX,62,278,300,25,hWnd,(HMENU)ID_LNK_CHECK,hInstance,NULL);
    // ������������� �������
    SendMessage(lnk_check,BM_SETCHECK,BST_CHECKED,0);

    // ������ ����� ��� ������ ���������
    //message_static=CreateWindow(_T("static"),_T(""),WS_VISIBLE|WS_CHILD|SS_CENTER,61,300,295,25,hWnd,(HMENU)ID_MESSAGE_STATIC,hInstance,NULL);

    // ���������� ����
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // �������� ���� ����������
    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0))
    {
      if (!IsDialogMessage(hWnd, &msg)) // ����� TAB ������� � ���������� ����� ����� ���������� 
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }
    return (int) msg.wParam;
}

// ��� ������� ������������ ��������� �� ����
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;
    TCHAR greeting[] = _T("Datacom");
    
    switch (message)
    {
    case WM_COMMAND:
      if(wParam==ID_CREATE_BUTTON)
      {
        // �������� �����
        len_text=GetWindowTextLength(login_edit) + 1;
        GetWindowText(login_edit,login,len_text);

        // �������� ������
        len_text=GetWindowTextLength(password_edit) + 1;
        GetWindowText(password_edit,password,len_text);

        // ����������� ����������
        SetWindowText(create_button, _T("��� ���������..."));
        if(create_pppoe())
        {
          Sleep(900);
          SetWindowText(create_button, _T("����������� ���������"));
          Sleep(900);
        }
        else
        {
          SetWindowText(create_button, _T("��������� ���������"));
          // ��������� �� ���������
          Sleep(900);
          SetWindowText(create_button,_T("����� �� ���������"));
          Sleep(800);
          exit(0);
        }

        // �������� ���� �� ��������� �����
        LRESULT res = SendMessage(lnk_check, BM_GETCHECK, 0, 0);
        // ���� ������� �����.
        if(res == BST_CHECKED)
        {
           if(short_cut_startup(_T(CONN_NAME), _T(LNK_NAME)))
           {
             SetWindowText(create_button, _T("��������� �����..."));
             Sleep(900);
             SetWindowText(create_button, _T("����� ������"));
             Sleep(900);
           }
           else
           {
             SetWindowText(create_button, _T("��������� �����..."));
             Sleep(900);
             SetWindowText(create_button, _T("����� �� ����� ������"));
             Sleep(900);
           }
        }
        // ���� ������� �� �����.
        if(res == BST_UNCHECKED)
        {
           SetWindowText(create_button, _T("����� �� ����� ������"));
           Sleep(900);
        }

        // ��������� �� ���������
        SetWindowText(create_button,_T("�����..."));
        Sleep(900);
        exit(0);
      }
    case WM_PAINT:
        // ��������� ������� � ��������� �����
        // �� ������� ��� ���� �����������
        hdc = BeginPaint(hWnd, &ps);
        TextOut(hdc, 5, 5, greeting, _tcslen(greeting));
        EndPaint(hWnd, &ps);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }

    return 0;
}
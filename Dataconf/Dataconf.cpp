/*

  Утилита Dataconf.exe Помогает настроить PPPoE соединение в
  операционных системах семейства Windows  начиная с Windows XP
  Александр Карабанов
  zend.karabanov@gmail.com

  Эта программа является свободным программным обеспечением:
  вы можете распространять её и/или модифицировать
  в соответствии с условиями лицензии GNU General Public License версии 3
  либо (по вашему выбору) любой более поздней версии, опубликованной
  Free Software Foundation.

  Эта программа распространяется в надежде на то, что она будет полезной,
  но БЕЗ КАКИХ-ЛИБО ГАРАНТИЙ, вы используете её на свой СТРАХ и РИСК.
  Прочтите GNU General Public License для более подробной информации.

  Вы должны были получить копию GNU General Public License
  вместе с этой программой. Если нет, см. <http://www.gnu.org/licenses/>.

*/

/*

Прежде всего читаем это http://habrahabr.ru/qa/39789/

Затем настраиваем проект так:

Проект -> Свойства -> Саойства конфигурации -> Общие -> Набор инструментов платформы = Visual Studio 2012 - Windows XP (v110_xp)
Проект -> Свойства -> Саойства конфигурации -> Общие -> Набор символов = Использовать набор символов Юникода
Проект -> Свойства -> Саойства конфигурации -> Компоновщик -> Ввод -> Дополнительные зависимости = kernel32.lib;user32.lib;comctl32.lib;rasapi32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;odbc32.lib;odbccp32.lib;%(AdditionalDependencies)
Проект -> Свойства -> Саойства конфигурации -> Компоновщик -> Файл манифеста -> Создавать манифкст = Да (/MANIFEST)
Проект -> Свойства -> Саойства конфигурации -> Компоновщик -> Файл манифеста -> Дополнительные зависимости манифеста = "type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'"
Проект -> Свойства -> Саойства конфигурации -> C/C++ -> Создание кода -> Многопоточная (/MT)

*/

#define WINVER 0x0501
#define _WIN32_WINNT 0x0501

#include <windows.h>
#include <windowsx.h>
#include <winerror.h>

// Чтобы можно было создавать ярлыки
#include <shellapi.h>
#include <shlobj.h>
#include <shlwapi.h>
#pragma comment(lib,"Shlwapi.lib")

// Чтобы удобно было рулить строками
#include <string>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <tchar.h>

// Что бы можно было рулить соединениями
#include <ras.h>
#include <rasdlg.h>
#include <raserror.h>
#pragma comment(lib, "rasapi32.lib")
#pragma comment(lib, "Mpr.lib")

// Чтобы кнопки и иные контролы стали гламурными
#include <commdlg.h>
#include <commctrl.h>
#pragma comment(lib,"comctl32.lib")
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// Чтобы кнопки и иные контролы стали гламурными
// Скорее всего эта строка не иметт смысла
void InitializeCommonControlsEx(VOID);

using namespace std;

#define CONN_NAME "Datacom_XXI"
#define LNK_NAME "Datacom_XXI.lnk"
#define SERVICE_NAME "datacom"

// Всякие идентификаторы для контролов
#define ID_LOGO_STATIC 1
#define ID_LOGIN_STATIC 2
#define ID_LOGIN_EDIT 3
#define ID_PASSWORD_STATIC 4
#define ID_PASSWORD_EDIT 5
#define ID_CREATE_BUTTON 6
#define ID_LNK_CHECK 7
//#define ID_MESSAGE_STATIC 8

// Определяем указатели на контролы
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

// Эта функция создаёт PPPoE соединение
BOOL create_pppoe()
{
  // Настраиваем PPPoE
  RASENTRY RS;
  memset(&RS, 0, sizeof(RASENTRY));
  RS.dwSize = sizeof(RS);
  RS.dwfOptions =
  RASEO_PreviewUserPw |           // Запрашивать имя пользователя и пароль
  RASEO_ModemLights |             // Вывести значёк в область уведомлений
  // RASEO_PreviewDomain |        // отображать поле "Домен" в окне ввода пользователя и пароля (вкладка "Параметры" - установить флажок "Включать домен входа в Windows")
  // RASEO_PreviewPhoneNumber |   // отображать выпадающий список "Набрать" в окне ввода пользователя и пароля
  RASEO_ShowDialingProgress |     // вкладка "Параметры" - Параметры набора номера: установить флажок "Отображать ход подключения"
  RASEO_RemoteDefaultGateway |    // вкладка "Сеть" - "Протокол Интернета (TCP/IP)" - кнопка "Дополнительно" - установить флажок "Использовать основной шлюз в удаленной сети"
  RASEO_SwCompression |           // вкладка "Сеть" - кнопка "Параметры" - установить флажок "Использовать программное сжатие данных"
  // RASEO_RequirePAP |           // вкладка "Безопасность" - "Дополнительные (выборочные параметры)" - установить флажок "Незашифрованный пароль (PAP)"
  // RASEO_RequireSPAP |          // вкладка "Безопасность" - "Дополнительные (выборочные параметры)" - установить флажок "Протокол проверки пароля Shiva (SPAP)"
  RASEO_RequireCHAP |             // вкладка "Безопасность" - "Дополнительные (выборочные параметры)" - установить флажок "Протокол проверки пароля CHAP"
  // RASEO_RequireMsCHAP |        // вкладка "Безопасность" - "Дополнительные (выборочные параметры)" - установить флажок "Протокол проверки пароля Microsoft (MS-CHAP)"
  // RASEO_RequireW95MSCHAP |     // вкладка "Безопасность" - "Дополнительные (выборочные параметры)" - установить флажок "Разрешить старый протокол MS-CHAP для Windows 95"
  // RASEO_RequireMsCHAP2 |       // вкладка "Безопасность" - "Дополнительные (выборочные параметры)" - установить флажок "Протокол проверки пароля Microsoft (MS-CHAP v2)"
  RASEO_SecureLocalFiles;         // вкладка "Сеть" - Компоненты, используемые этим подключением: снять флажок "Использовать основной шлюз в удаленной сети";
  RS.dwfOptions2 = RASEO2_Internet | RASEO2_DontNegotiateMultilink | RASEO2_ReconnectIfDropped;
  RS.dwIdleDisconnectSeconds = RASIDS_Disabled; // Не отключаться если нет активности
  RS.dwfNetProtocols = RASNP_Ip;             // протоколы Интернета
  RS.dwFramingProtocol = RASFP_Ppp;          // протокол Point-to-Point Protocol
  RS.dwType = RASET_Broadband;               // тип подключения
  RS.dwVpnStrategy = VS_Default;             // вкладка "Сеть" - По умолчанию
  RS.dwEncryptionType = ET_Optional;         // параметры шифрования (ET_None, ET_Require, ET_RequireMax, ET_Optional)
  RS.dwDialMode = RASEDM_DialAll;
  RS.dwRedialCount = 99;                     // число повторов набора номера
  RS.dwRedialPause = 60;                     // интервал между повторениями
  lstrcpy(RS.szLocalPhoneNumber, _T(SERVICE_NAME)); // Вкладка "Общие" - "Имя службы"
  lstrcpy(RS.szDeviceType, RASDT_PPPoE);     // RASDT_PPPoE - обуславливает тип подключения (PPPoE)
  lstrcpy(RS.szDeviceName, _T("PPPoE"));

  DWORD dwRet = RasSetEntryProperties(NULL,_T(CONN_NAME), &RS, sizeof(RS), NULL, 0);
  // Если не удалось создать подключение возарвщвем FALSE
  if(dwRet != ERROR_SUCCESS)
  {
    return FALSE;
  }

  // Прописываем логин и парль (Чтоб в Windows 7 работало)
  RASCREDENTIALS RS_cred;
  memset(&RS_cred, 0, sizeof(RASCREDENTIALS));
  RS_cred.dwSize = sizeof(RS_cred);
  RS_cred.dwMask=RASCM_UserName | RASCM_Password;
  lstrcpy(RS_cred.szUserName, login);
  lstrcpy(RS_cred.szPassword, password);
  lstrcpy(RS_cred.szDomain,_T(""));

  DWORD dwCred = RasSetCredentials(NULL,_T(CONN_NAME),&RS_cred,FALSE);

  // Если не удалось происать логин и пароль возарвщвем FALSE
  if(dwCred != ERROR_SUCCESS)
  {
    return FALSE;
  }

  // Прописываем логин и пароль
  // (в XP этого достаточно, чтоб прописать логин и пароль) 
  RASDIALPARAMS RS_param;
  memset(&RS_param, 0, sizeof(RASDIALPARAMS));
  RS_param.dwSize = sizeof(RS_param);
  lstrcpy(RS_param.szEntryName, _T(CONN_NAME));
  lstrcpy(RS_param.szUserName, login);
  lstrcpy(RS_param.szPassword, password);
  
  DWORD dwParam = RasSetEntryDialParams(NULL, &RS_param, FALSE);

  // Если не удалось происать логин и пароль возарвщвем FALSE
  if(dwParam != ERROR_SUCCESS)
  {
    return FALSE;
  }
  
  return TRUE;
}

// Весь этот код поможет вынести ярлык на рабочий стол
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

// Эта функция помогает расположить окно в центре дисплея
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
  
     // На всякий случай не двигаем окно если оно выходит за перделы экрана
     // Видимо этот код неимеет смысла
     if (nX < 0) nX = 0;
     if (nY < 0) nY = 0;
     if (nX + nWidth > nScreenWidth) nX = nScreenWidth - nWidth;
     if (nY + nHeight > nScreenHeight) nY = nScreenHeight - nHeight;

     // Двигаем окно в центр экрана
     if (MoveWindow(hwndWindow, nX, nY, nWidth, nHeight, FALSE))
     {
       return TRUE;
     }
     return FALSE;
}

// Имя класса
static TCHAR szWindowClass[] = _T("win32app");

// Надпись в заголовке окна будет такой
static TCHAR szTitle[] = _T("Настройка подключения к Datacom");

HINSTANCE hInst;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX wcex;

    // Чтобы контролы были красивыми
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
        MessageBox(NULL, _T("Call to RegisterClassEx failed!"), _T("Ошибка"), NULL);
        return 1;
    }

    hInst = hInstance; // Чтоб было доступно отовсюду

    // Создаём окошко
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
        MessageBox(NULL, _T("Что-то не срослось :-("), _T("Ошибка"), NULL);
        return 1;
    }

    if (!CenterWindow(hWnd))
    {
        MessageBox(NULL, _T("Неудалось расположить окно в центре экрана"), _T("Ошибка"), NULL);
        return 1;
    }

    // Создаём метку вкоторую вставим картинку
    myPicture = CreateWindow(_T("static"),_T(""),SS_BITMAP|WS_CHILD|WS_VISIBLE|WS_BORDER,0,0,370,77,hWnd,(HMENU)ID_LOGO_STATIC,hInstance,NULL);
    bmp=LoadBitmap(hInstance, _T("logo_datacom"));
    SendMessage(myPicture,STM_SETIMAGE,IMAGE_BITMAP,(LPARAM)bmp);

    // Создаём метку логина
    login_static=CreateWindow(_T("static"),_T("Имя пользователя:"),WS_VISIBLE|WS_CHILD|SS_LEFT,61,91,250,25,hWnd,(HMENU)ID_LOGIN_STATIC,hInstance,NULL);

    // Создаём текстовое поле для ввода логина
    login_edit=CreateWindow(_T("edit"),_T(""),WS_TABSTOP|ES_AUTOHSCROLL|WS_VISIBLE|WS_CHILD|WS_BORDER,62,112,250,25,hWnd,(HMENU)ID_LOGIN_EDIT,hInstance,NULL);

    // Ставим фокус на текстовое поле для ввода логина
    SetFocus(login_edit);

    // Создаём метку пароля
    password_static=CreateWindow(_T("static"),_T("Пароль:"),WS_VISIBLE|WS_CHILD|SS_LEFT,61,156,250,25,hWnd,(HMENU)ID_PASSWORD_STATIC,hInstance,NULL);

    // Создаём текстовое поле для ввода пароля
    password_edit=CreateWindow(_T("edit"),_T(""),WS_TABSTOP|ES_AUTOHSCROLL|WS_VISIBLE|WS_CHILD|WS_BORDER,62,177,250,25,hWnd,(HMENU)ID_PASSWORD_EDIT,hInstance,NULL);

    // Создаём кнопку "Настроить подключение"
    create_button=CreateWindow(_T("button"),_T("Настроить подключение"),WS_TABSTOP|WS_CHILD|BS_PUSHBUTTON|WS_VISIBLE,61,221,252,40,hWnd,(HMENU)ID_CREATE_BUTTON,NULL,NULL);

    // Создаём чекбокс "Добавить ярлык на работчий стол"
    lnk_check=CreateWindow(_T("button"), _T("Добавить ярлык на рабочий стол"), WS_TABSTOP|WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX,62,278,300,25,hWnd,(HMENU)ID_LNK_CHECK,hInstance,NULL);
    // Устанавливаем чекбокс
    SendMessage(lnk_check,BM_SETCHECK,BST_CHECKED,0);

    // Создаём метку для вывода сообщений
    //message_static=CreateWindow(_T("static"),_T(""),WS_VISIBLE|WS_CHILD|SS_CENTER,61,300,295,25,hWnd,(HMENU)ID_MESSAGE_STATIC,hInstance,NULL);

    // Показываем окно
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // Основной цикл приложения
    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0))
    {
      if (!IsDialogMessage(hWnd, &msg)) // Чтобы TAB работал и переключал фокус между контролами 
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }
    return (int) msg.wParam;
}

// Эта функция обрабатывает сообщения от окна
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
        // Получаем логин
        len_text=GetWindowTextLength(login_edit) + 1;
        GetWindowText(login_edit,login,len_text);

        // Получаем пароль
        len_text=GetWindowTextLength(password_edit) + 1;
        GetWindowText(password_edit,password,len_text);

        // Настраиваем соединение
        SetWindowText(create_button, _T("Идёт настройка..."));
        if(create_pppoe())
        {
          Sleep(900);
          SetWindowText(create_button, _T("Подключение настроено"));
          Sleep(900);
        }
        else
        {
          SetWindowText(create_button, _T("Неудалось настроить"));
          // Автовыход из программы
          Sleep(900);
          SetWindowText(create_button,_T("Выход из программы"));
          Sleep(800);
          exit(0);
        }

        // Выясняем надо ли создавать ярлык
        LRESULT res = SendMessage(lnk_check, BM_GETCHECK, 0, 0);
        // Если галочка стоит.
        if(res == BST_CHECKED)
        {
           if(short_cut_startup(_T(CONN_NAME), _T(LNK_NAME)))
           {
             SetWindowText(create_button, _T("Создается ярлык..."));
             Sleep(900);
             SetWindowText(create_button, _T("Ярлык создан"));
             Sleep(900);
           }
           else
           {
             SetWindowText(create_button, _T("Создается ярлык..."));
             Sleep(900);
             SetWindowText(create_button, _T("Ярлык не будет создан"));
             Sleep(900);
           }
        }
        // Если галочка не стоит.
        if(res == BST_UNCHECKED)
        {
           SetWindowText(create_button, _T("Ярлык не будет создан"));
           Sleep(900);
        }

        // Автовыход из программы
        SetWindowText(create_button,_T("Выход..."));
        Sleep(900);
        exit(0);
      }
    case WM_PAINT:
        // Секретная надпись в секретном месте
        // не понятно для чего оставленная
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
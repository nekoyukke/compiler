#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>
#include <tchar.h>
#include <commdlg.h>
#include <shlobj.h>




HWND buttons[4];
HWND text[1];
HWND hwndStatus;
HINSTANCE hInst;//インスタンスハンドル


int make_button(HWND hWnd,HINSTANCE hInst,int number,const wchar_t* text,HWND*button_list,int x1,int y1,int W,int H,int reqest);
int make_edit(HWND hWnd,HINSTANCE hInst,int number,HWND*edit_list,int x,int y,int W,int H,int reqest);










LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpszCmdLine, int nCmdShow){
    TCHAR szAppName[] = TEXT("Editer v0.1");
    WNDCLASS wc;
    HWND hwnd;
    MSG msg;
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = szAppName;
    if (!RegisterClass(&wc)) return 0;
    hwnd = CreateWindow(
        szAppName,
        szAppName,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL,
        hInstance, NULL);
    if (!hwnd) return 0;
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    //コンソールのコードページを設定
    _setmode(_fileno(stdout), _O_U16TEXT);

    //COMライブラリを初期化
    CoInitialize(NULL);

    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    //COMライブラリをクリーンアップ
    CoUninitialize();

    return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
    HDC hdc;
    PAINTSTRUCT ps;
    static HPEN hpen, hpenPrev;
    static HBRUSH hbr, hbrPrev;

    switch (uMsg) {
    //MAKEされたとき
    case WM_CREATE:
        make_edit(hwnd, hInst, 0, text, 0, 50, 800, 600, 0);
        //テキストボックス（CPUA）
        make_button(hwnd,hInst,0,L"Undo",buttons,0,0,50,30,100);
        make_button(hwnd,hInst,1,L"Clear",buttons,50,0,50,30,101);
        make_button(hwnd,hInst,2,L"Save",buttons,100,0,50,30,102);
        make_button(hwnd,hInst,3,L"Open",buttons,150,0,50,30,103);
        // ファイルドロップを受け取れるように設定する
        DragAcceptFiles(hwnd, TRUE); // TRUEで受け取る
        return 0;

    //このcaseに2日かかったという事実
    //ケアレスミス怖いから絶対に触らないこと！！！
    case WM_DROPFILES:
        wchar_t buffer1[MAX_PATH], buffer2[MAX_PATH+39];
        int _size;
        //ドロップされたファイル数を取得する
        _size = DragQueryFileW((HDROP)wParam, -1, NULL, 0);
        //ファイルが2以上の時エラー出す
        if (_size >= 2){
            MessageBoxW(hwnd, L"error: There are two or more files.\nエラー:ファイルが2つ以上あります", L"ERROR  エラー", MB_OK | MB_ICONERROR);
            return 0;
        }
        //ドロップされたファイル名を取得する
        DragQueryFileW((HDROP)wParam,0,buffer1,MAX_PATH);
        wsprintfW(buffer2,L"dropfile path is %s.\n open this file?",buffer1);
        if (MessageBoxW(hwnd,buffer2,L"info", MB_YESNO) == IDNO){
            return 0;
        }
        //ボックスに書き込み
        FILE *fp = _wfopen(buffer1,L"r, ccs=UTF-8");
        if (fp != NULL){
            wchar_t line[512];
            SetWindowTextW(text[0], L"");
            // ファイルから一行ずつ読み込む
            while (fgetws(line,sizeof(line)/sizeof(wchar_t),fp) != NULL) {
                wprintf(L"%ls",line);//読み込んだ行を表示
                //表示準備
                //行数
                int num = (GetWindowTextLength(text[0]) + 1 + 2 + wcslen(line));
                //文字の長さW
                wchar_t* strText = (wchar_t*)calloc(num, sizeof(wchar_t));
                //ボックス取得
                GetWindowTextW(text[0], strText, num);
                //追加
                //変換して結合
                if (wcschr(line, L'\n') != NULL){
                    line[wcslen(line) - 1] = L'\0';
                    wcscat(line, L"\r\n");
                }
                wcscat(strText, line);
                //テキストボックスに設定
                SetWindowTextW(text[0], strText);
                free(strText);
                strText = NULL;
            }
            //クローズ
            fclose(fp);
            MessageBoxW(hwnd, L"File opened successfully.\nファイルが正しく読み込めました！", L"Success:完璧", MB_OK);
        } else {
            MessageBoxW(hwnd, L"Failed to open file.\nファイル読み込みに失敗しました..", L"Error:エラー", MB_OK | MB_ICONERROR);
        }
        // ファイル情報の内部データを解放する
        DragFinish((HDROP)wParam);
        return 0;

    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);
        // ペイント
        EndPaint(hwnd, &ps);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    
    //ctrl+S
    case WM_KEYDOWN:
        switch (wParam){
        
        default:
            return 0;
        }
    
    //任意実行
    case WM_COMMAND:
        switch(LOWORD(wParam)){
            case 100:
                SendMessageA(text[0], EM_UNDO, 0, 0);
                return 0;
            case 101:
                if (MessageBoxW(hwnd,L"Do you want to delete everything?\n全部消しますか？",L"Yes No", MB_YESNO) == IDNO){
                    return 0;
                }
                SetWindowTextW(text[0], L"");
                return 0;
            case 102:
                //行数
                int num = (GetWindowTextLength(text[0]) + 1);
                //文字の長さW
                wchar_t *strText = (wchar_t*)calloc(num, sizeof(wchar_t));
                if (strText == NULL) {
                    MessageBoxW(hwnd, L"Memory allocation failed", L"Error", MB_OK | MB_ICONERROR);
                    return 0;
                }
                //セット
                GetWindowTextW(text[0], strText, num);

                //ファイル保存ダイアログを表示
                OPENFILENAMEW ofn;
                wchar_t szFile[MAX_PATH] = L"";
                ZeroMemory(&ofn, sizeof(ofn));
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hwnd;
                ofn.lpstrFile = szFile;
                ofn.nMaxFile = sizeof(szFile) / sizeof(szFile[0]);
                ofn.lpstrFilter = L"All Files\0*.*\0";
                ofn.nFilterIndex = 1;
                ofn.lpstrFileTitle = NULL;
                ofn.nMaxFileTitle = 0;
                ofn.lpstrInitialDir = NULL;
                ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;

                if (GetSaveFileNameW(&ofn) == TRUE) {
                    //ファイルに書き込み
                    FILE *fp = _wfopen(ofn.lpstrFile, L"w, ccs=UTF-8");
                    if (fp != NULL) {
                        //改行コードを適切に処理する
                        wchar_t *p = strText;
                        while (*p) {
                            if (*p == L'\r' && *(p + 1) == L'\n'){
                                fputwc(L'\n', fp);
                                p++;
                            }else{
                                fputwc(*p, fp);
                            }
                            p++;
                        }
                        fclose(fp);
                        MessageBoxW(hwnd, L"File opened successfully.\nファイルが正しく読み込めました！", L"Success:完璧", MB_OK);
                    } else {
                        MessageBoxW(hwnd, L"Failed to open file.\nファイル読み込みに失敗しました..", L"Error:エラー", MB_OK | MB_ICONERROR);
                    }
                }

                free(strText);
                return 0;
            case 103:
                //openする
                //ファイルを開く
                OPENFILENAMEW ofnOpen;
                wchar_t szFileOpen[MAX_PATH] = L"";
                ZeroMemory(&ofnOpen, sizeof(ofnOpen));
                ofnOpen.lStructSize = sizeof(ofnOpen);
                ofnOpen.hwndOwner = hwnd;
                ofnOpen.lpstrFile = szFileOpen;
                ofnOpen.nMaxFile = sizeof(szFileOpen) / sizeof(szFileOpen[0]);
                ofnOpen.lpstrFilter = L"All Files\0*.*\0";
                ofnOpen.nFilterIndex = 1;
                ofnOpen.lpstrFileTitle = NULL;
                ofnOpen.nMaxFileTitle = 0;
                ofnOpen.lpstrInitialDir = NULL;
                ofnOpen.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

                if (GetOpenFileNameW(&ofnOpen) == TRUE) {
                    FILE *fp = _wfopen(ofnOpen.lpstrFile, L"r, ccs=UTF-8");
                    if (fp != NULL) {
                        wchar_t line[512];
                        SetWindowTextW(text[0], L"");
                        // ファイルから一行ずつ読み込む
                        while (fgetws(line,sizeof(line)/sizeof(wchar_t),fp) != NULL) {
                            wprintf(L"%ls",line);//読み込んだ行を表示
                            //表示準備
                            //行数
                            int num = (GetWindowTextLength(text[0]) + 1 + 2 + wcslen(line));
                            //文字の長さW
                            wchar_t* strText = (wchar_t*)calloc(num, sizeof(wchar_t));
                            //ボックス取得
                            GetWindowTextW(text[0], strText, num);
                            //追加
                            //変換して結合
                            if (wcschr(line, L'\n') != NULL){
                                line[wcslen(line) - 1] = L'\0';
                                wcscat(line, L"\r\n");
                            }
                            wcscat(strText, line);
                            //テキストボックスに設定
                            SetWindowTextW(text[0], strText);
                            free(strText);
                            strText = NULL;
                        }
                        //クローズ
                        fclose(fp);
                        MessageBoxW(hwnd, L"File opened successfully.\nファイルが正しく読み込めました！", L"Success:完璧", MB_OK);
                    } else {
                        MessageBoxW(hwnd, L"Failed to open file.\nファイル読み込みに失敗しました..", L"Error:エラー", MB_OK | MB_ICONERROR);
                    }
                }
                return 0;
        }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


//ボタン生成
int make_button(HWND hWnd,HINSTANCE hInst,int number,const wchar_t* text,HWND*button_list,int x1,int y1,int W,int H,int reqest){
    button_list[number]=CreateWindowW(
        L"BUTTON", text,
        WS_CHILD | WS_VISIBLE,
        x1, y1, W, H,
        hWnd, (HMENU)(UINT_PTR)reqest, hInst, NULL);
    return 0;
}
int make_edit(HWND hWnd,HINSTANCE hInst,int number,HWND*edit_list,int x,int y,int W,int H,int reqest){
    edit_list[number]=CreateWindowExW(
        0,L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL| ES_MULTILINE | WS_VSCROLL | WS_HSCROLL,
        x, y, W, H,
        hWnd, (HMENU)(UINT_PTR)reqest, hInst, NULL);
    return 0;
}
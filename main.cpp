#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <commctrl.h>
#include <shobjidl.h> 
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <numeric>
#include "include/BudgetManager.h"

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "ole32.lib")

BudgetManager g_BudgetManager;
std::vector<Transaction> g_CurrentTransactions;
bool g_IsMonthly = true;
int g_SelectedMonth = 6;
int g_SelectedYear = 2026;

#define ID_RADIO_MONTHLY    101
#define ID_RADIO_YEARLY     102
#define ID_LISTVIEW         103
#define ID_COMBO_MONTH      104
#define ID_COMBO_YEAR       105
#define ID_BTN_OPEN         106
#define ID_CANVAS_BREAKDOWN 107

const COLORREF K_PALETTE[] = {
    RGB(200, 50, 50),
    RGB(50, 50, 200),
    RGB(200, 150, 50),
    RGB(150, 50, 150),
    RGB(50, 150, 150),
    RGB(46, 139, 87),
    RGB(220, 20, 60),
    RGB(255, 140, 0),
    RGB(70, 130, 180),
    RGB(154, 205, 50),
    RGB(186, 85, 211),
    RGB(218, 165, 32),
    RGB(0, 128, 128),
    RGB(128, 0, 0),
    RGB(0, 0, 128),
    RGB(105, 105, 105)
};

HWND hwndListView = NULL;
HWND hwndComboMonth = NULL;
HWND hwndComboYear = NULL;
HWND hwndGroupSettings = NULL;
HWND hwndRadioMonthly = NULL;
HWND hwndRadioYearly = NULL;
HWND hwndBtnOpen = NULL;
HWND hwndGroupSummary = NULL;    
HWND hwndGroupBreakdown = NULL;
HWND hwndCanvasBreakdown = NULL; 

int g_PieScrollYOffset = 0; 

void DrawBudgetCircleAndSummary(HWND hwnd, HDC hdc);
void RefreshData(HWND hListView);

struct CategoryDlgData {
    std::wstring currentCategory;
    wchar_t outputCategory[64];
};

INT_PTR CALLBACK CategoryDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static CategoryDlgData* pData = nullptr;
    switch (uMsg) {
    case WM_INITDIALOG: {
        pData = reinterpret_cast<CategoryDlgData*>(lParam);
        SetWindowTextW(hwndDlg, L"Change Category");
        CreateWindowW(L"STATIC", L"Enter new category name:", WS_CHILD | WS_VISIBLE, 15, 15, 220, 18, hwndDlg, NULL, NULL, NULL);
        HWND hwndEdit = CreateWindowW(L"EDIT", pData->currentCategory.c_str(), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 15, 38, 220, 22, hwndDlg, (HMENU)201, NULL, NULL);
        CreateWindowW(L"BUTTON", L"OK", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 65, 75, 80, 25, hwndDlg, (HMENU)IDOK, NULL, NULL);
        CreateWindowW(L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE, 155, 75, 80, 25, hwndDlg, (HMENU)IDCANCEL, NULL, NULL);

        EnumChildWindows(hwndDlg, [](HWND child, LPARAM lp) -> BOOL {
            SendMessageW(child, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
            return TRUE;
        }, 0);

        SetFocus(hwndEdit);
        SendMessageW(hwndEdit, EM_SETSEL, 0, -1);
        return FALSE;
    }
    case WM_COMMAND: {
        if (LOWORD(wParam) == IDOK) {
            GetDlgItemTextW(hwndDlg, 201, pData->outputCategory, 64);
            EndDialog(hwndDlg, IDOK);
            return TRUE;
        }
        else if (LOWORD(wParam) == IDCANCEL) {
            EndDialog(hwndDlg, IDCANCEL);
            return TRUE;
        }
        break;
    }
    }
    return FALSE;
}

LRESULT CALLBACK BreakdownCanvasProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_VSCROLL: {
        SCROLLINFO si = { 0 };
        si.cbSize = sizeof(si);
        si.fMask = SIF_ALL;
        GetScrollInfo(hwnd, SB_VERT, &si);
        int oldPos = si.nPos;

        switch (LOWORD(wParam)) {
        case SB_TOP: si.nPos = si.nMin; break;
        case SB_BOTTOM: si.nPos = si.nMax; break;
        case SB_LINEUP: si.nPos -= 15; break;
        case SB_LINEDOWN: si.nPos += 15; break;
        case SB_PAGEUP: si.nPos -= si.nPage; break;
        case SB_PAGEDOWN: si.nPos += si.nPage; break;
        case SB_THUMBTRACK: si.nPos = si.nTrackPos; break;
        }

        if (si.nPos < 0) si.nPos = 0;
        if (si.nPos > (si.nMax - (int)si.nPage)) si.nPos = si.nMax - si.nPage;

        if (si.nPos != oldPos) {
            si.fMask = SIF_POS;
            SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
            g_PieScrollYOffset = si.nPos;
            InvalidateRect(hwnd, NULL, TRUE);
        }
        return 0;
    }
    case WM_MOUSEWHEEL: {
        short delta = GET_WHEEL_DELTA_WPARAM(wParam);
        SCROLLINFO si = { 0 };
        si.cbSize = sizeof(si);
        si.fMask = SIF_ALL;
        GetScrollInfo(hwnd, SB_VERT, &si);
        int oldPos = si.nPos;

        si.nPos -= (delta / WHEEL_DELTA) * 20;

        if (si.nPos < 0) si.nPos = 0;
        if (si.nPos > (si.nMax - (int)si.nPage)) si.nPos = si.nMax - si.nPage;

        if (si.nPos != oldPos) {
            si.fMask = SIF_POS;
            SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
            g_PieScrollYOffset = si.nPos;
            InvalidateRect(hwnd, NULL, TRUE);
        }
        return 0;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        
        HBRUSH bgBrush = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
        FillRect(hdc, &ps.rcPaint, bgBrush);
        DeleteObject(bgBrush);

        SetBkColor(hdc, GetSysColor(COLOR_3DFACE));
        SetTextColor(hdc, RGB(0, 0, 0));
        
        DrawBudgetCircleAndSummary(hwnd, hdc);
        
        EndPaint(hwnd, &ps);
        return 0;
    }
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

void RefreshData(HWND hListView) {
    if (!hListView) return;

    if (g_IsMonthly) {
        g_CurrentTransactions = g_BudgetManager.getTransactionsMonth(g_SelectedMonth, g_SelectedYear);
    } else {
        g_CurrentTransactions = g_BudgetManager.getTransactionsYear(g_SelectedYear);
    }

    ListView_DeleteAllItems(hListView);
    for (size_t i = 0; i < g_CurrentTransactions.size(); ++i) {
        const auto& t = g_CurrentTransactions[i];
        
        std::wstring dateStr = std::to_wstring(t.year) + L"-" + std::to_wstring(t.month) + L"-" + std::to_wstring(t.day);
        std::wstring descStr(t.Description.begin(), t.Description.end());
        std::wstring catStr(t.Category.begin(), t.Category.end());
        std::wstring amtStr = (t.amount < 0 ? L"$" : L"+$") + std::to_wstring(std::abs(t.amount)).substr(0, std::to_wstring(std::abs(t.amount)).find(L'.') + 3);

        LVITEM lvi = {0};
        lvi.mask = LVIF_TEXT;
        lvi.iItem = static_cast<int>(i);

        lvi.iSubItem = 0; lvi.pszText = (LPWSTR)dateStr.c_str(); ListView_InsertItem(hListView, &lvi);
        lvi.iSubItem = 1; lvi.pszText = (LPWSTR)descStr.c_str(); ListView_SetItem(hListView, &lvi);
        lvi.iSubItem = 2; lvi.pszText = (LPWSTR)catStr.c_str(); ListView_SetItem(hListView, &lvi);
        lvi.iSubItem = 3; lvi.pszText = (LPWSTR)amtStr.c_str(); ListView_SetItem(hListView, &lvi);
    }
}

void OnOpenFile(HWND hwndOwner, HWND hListView) {
    IFileOpenDialog* pFileOpen = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));
    
    if (SUCCEEDED(hr)) {
        COMDLG_FILTERSPEC fileTypes[] = { { L"CSV Files", L"*.csv" }, { L"All Files", L"*.*" } };
        pFileOpen->SetFileTypes(2, fileTypes);
        
        hr = pFileOpen->Show(hwndOwner);
        if (SUCCEEDED(hr)) {
            IShellItem* pItem = nullptr;
            hr = pFileOpen->GetResult(&pItem);
            if (SUCCEEDED(hr)) {
                PWSTR pszFilePath = nullptr;
                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                
                if (SUCCEEDED(hr)) {
                    std::wstring ws(pszFilePath);
                    std::string filePath(ws.begin(), ws.end());
                    
                    if (g_BudgetManager.loadRawDataFile(filePath)) {
                        g_BudgetManager.saveBudgetFile();
                        RefreshData(hListView);
                        MessageBoxW(hwndOwner, L"Raw data integrated!", L"Success", MB_OK | MB_ICONINFORMATION);
                        InvalidateRect(hwndOwner, NULL, TRUE);
                    }
                    CoTaskMemFree(pszFilePath);
                }
                pItem->Release();
            }
        }
        pFileOpen->Release();
    }
}

void DrawBudgetCircleAndSummary(HWND hwnd, HDC hdc) {
    double totalSpending = 0.0;
    std::map<std::string, double> categoryMap;

    for (const auto& t : g_CurrentTransactions) {
        if (t.amount < 0) {
            double absAmt = std::abs(t.amount);
            totalSpending += absAmt;
            categoryMap[t.Category] += absAmt;
        }
    }

    int left = 45, top = 20, right = 165, bottom = 140;
    int centerX = (left + right) / 2;
    int centerY = (top + bottom) / 2;
    int radius = (right - left) / 2;

    if (totalSpending == 0.0) {
        HBRUSH grayBrush = CreateSolidBrush(RGB(192, 192, 192));
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, grayBrush);
        Ellipse(hdc, left, top - g_PieScrollYOffset, right, bottom - g_PieScrollYOffset);
        SelectObject(hdc, oldBrush);
        DeleteObject(grayBrush);
        return;
    }

    double currentAngle = 0.0;
    int colorIndex = 0;
    int legendY = 160 - g_PieScrollYOffset; 

    for (const auto& pair : categoryMap) {
        double share = pair.second / totalSpending;
        double sweepAngle = share * 2.0 * 3.1415926535;

        int startX = centerX + static_cast<int>(radius * cos(currentAngle));
        int startY = centerY - static_cast<int>(radius * sin(currentAngle));

        currentAngle += sweepAngle;

        int endX = centerX + static_cast<int>(radius * cos(currentAngle));
        int endY = centerY - static_cast<int>(radius * sin(currentAngle));

        COLORREF color = K_PALETTE[colorIndex % 16];
        
        HBRUSH segmentBrush = CreateSolidBrush(color);
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, segmentBrush);
        Pie(hdc, left, top - g_PieScrollYOffset, right, bottom - g_PieScrollYOffset, 
            startX, startY - g_PieScrollYOffset, endX, endY - g_PieScrollYOffset);
        SelectObject(hdc, oldBrush);
        DeleteObject(segmentBrush);

        HBRUSH legendBoxBrush = CreateSolidBrush(color);
        RECT rect = { 15, legendY + 2, 27, legendY + 14 };
        FillRect(hdc, &rect, legendBoxBrush);
        DeleteObject(legendBoxBrush);

        std::wstring catName(pair.first.begin(), pair.first.end());
        std::wstring label = catName + L": $" + std::to_wstring(static_cast<int>(pair.second));
        TextOutW(hdc, 33, legendY, label.c_str(), static_cast<int>(label.length()));

        legendY += 18;
        colorIndex++;
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: {
        InitCommonControls();
        g_BudgetManager.loadBudgetFile("local_budget.csv");

        WNDCLASSW canvasWc = { 0 };
        canvasWc.lpfnWndProc = BreakdownCanvasProc;
        canvasWc.hInstance = (HINSTANCE)GetWindowLongPtrW(hwnd, GWLP_HINSTANCE);
        canvasWc.lpszClassName = L"ScrollableCanvasClass";
        canvasWc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
        RegisterClassW(&canvasWc);

        hwndGroupSettings = CreateWindowW(L"BUTTON", L"Report Settings", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 10, 5, 240, 150, hwnd, NULL, NULL, NULL);
        hwndRadioMonthly = CreateWindowW(L"BUTTON", L"Monthly", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP, 20, 25, 95, 20, hwnd, (HMENU)ID_RADIO_MONTHLY, NULL, NULL);
        CheckDlgButton(hwnd, ID_RADIO_MONTHLY, BST_CHECKED);
        hwndRadioYearly = CreateWindowW(L"BUTTON", L"Yearly", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 130, 25, 95, 20, hwnd, (HMENU)ID_RADIO_YEARLY, NULL, NULL);

        hwndComboMonth = CreateWindowW(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL, 20, 55, 210, 200, hwnd, (HMENU)ID_COMBO_MONTH, NULL, NULL);
        const wchar_t* months[] = { L"January", L"February", L"March", L"April", L"May", L"June", L"July", L"August", L"September", L"October", L"November", L"December" };
        for (int i = 0; i < 12; ++i) SendMessageW(hwndComboMonth, CB_ADDSTRING, 0, (LPARAM)months[i]);
        SendMessageW(hwndComboMonth, CB_SETCURSEL, g_SelectedMonth - 1, 0);

        hwndComboYear = CreateWindowW(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL, 20, 85, 210, 200, hwnd, (HMENU)ID_COMBO_YEAR, NULL, NULL);
        const wchar_t* years[] = { L"2024", L"2025", L"2026", L"2027" };
        for (int i = 0; i < 4; ++i) SendMessageW(hwndComboYear, CB_ADDSTRING, 0, (LPARAM)years[i]);
        SendMessageW(hwndComboYear, CB_SETCURSEL, 2, 0); 

        hwndBtnOpen = CreateWindowW(L"BUTTON", L"Import CSV Data", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 20, 115, 210, 28, hwnd, (HMENU)ID_BTN_OPEN, NULL, NULL);
        hwndGroupSummary = CreateWindowW(L"BUTTON", L"Financial Summary", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 10, 160, 240, 145, hwnd, NULL, NULL, NULL);
        hwndGroupBreakdown = CreateWindowW(L"BUTTON", L"Visual Breakdown", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 10, 310, 240, 220, hwnd, NULL, NULL, NULL);
        hwndCanvasBreakdown = CreateWindowW(L"ScrollableCanvasClass", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL, 18, 330, 224, 192, hwnd, (HMENU)ID_CANVAS_BREAKDOWN, NULL, NULL);

        hwndListView = CreateWindowW(WC_LISTVIEW, L"", WS_CHILD | WS_VISIBLE | LVS_REPORT | WS_BORDER | WS_HSCROLL | WS_VSCROLL | LVS_SHOWSELALWAYS, 260, 12, 510, 518, hwnd, (HMENU)ID_LISTVIEW, NULL, NULL);
        ListView_SetExtendedListViewStyle(hwndListView, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

        LVCOLUMNW lvc = {0};
        lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
        lvc.pszText = (LPWSTR)L"Date"; lvc.cx = 95; ListView_InsertColumn(hwndListView, 0, &lvc);
        lvc.pszText = (LPWSTR)L"Description"; lvc.cx = 245; ListView_InsertColumn(hwndListView, 1, &lvc);
        lvc.pszText = (LPWSTR)L"Category"; lvc.cx = 120; ListView_InsertColumn(hwndListView, 2, &lvc);
        lvc.pszText = (LPWSTR)L"Amount"; lvc.cx = 95; ListView_InsertColumn(hwndListView, 3, &lvc);

        SCROLLINFO si = { 0 };
        si.cbSize = sizeof(si);
        si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
        si.nMin = 0;
        si.nMax = 550; 
        si.nPage = 192; 
        si.nPos = 0;
        SetScrollInfo(hwndCanvasBreakdown, SB_VERT, &si, TRUE);

        EnumChildWindows(hwnd, [](HWND child, LPARAM lp) -> BOOL {
            SendMessageW(child, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
            return TRUE;
        }, 0);

        RefreshData(hwndListView);
        return 0;
    }

    case WM_SIZE: {
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);

        if (hwndListView) {
            MoveWindow(hwndListView, 260, 12, width - 275, height - 24, TRUE);
        }
        if (hwndGroupBreakdown) {
            int breakdownHeight = height - 320;
            if (breakdownHeight < 100) breakdownHeight = 100;
            MoveWindow(hwndGroupBreakdown, 10, 310, 240, breakdownHeight, TRUE);
            
            if (hwndCanvasBreakdown) {
                MoveWindow(hwndCanvasBreakdown, 18, 330, 224, breakdownHeight - 25, TRUE);
                
                SCROLLINFO si = { 0 };
                si.cbSize = sizeof(si);
                si.fMask = SIF_PAGE;
                si.nPage = breakdownHeight - 25;
                SetScrollInfo(hwndCanvasBreakdown, SB_VERT, &si, TRUE);
            }
        }
        
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;
    }

    case WM_NOTIFY: {
        LPNMHDR pnmh = (LPNMHDR)lParam;
        if (pnmh->hwndFrom == hwndListView && pnmh->code == NM_DBLCLK) {
            LPNMITEMACTIVATE pnmia = (LPNMITEMACTIVATE)lParam;
            int itemIndex = pnmia->iItem;
            
            if (itemIndex != -1 && itemIndex < static_cast<int>(g_CurrentTransactions.size())) {
                const Transaction& selectedTx = g_CurrentTransactions[itemIndex];
                
                CategoryDlgData dlgData;
                dlgData.currentCategory = std::wstring(selectedTx.Category.begin(), selectedTx.Category.end());
                ZeroMemory(dlgData.outputCategory, sizeof(dlgData.outputCategory));

                #pragma pack(push, 2)
                struct DLGTEMPLATE_MEM {
                    DLGTEMPLATE dt;
                    WORD menu;
                    WORD cls;
                    WCHAR title[1];
                } dlgTemplate = { { WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_MODALFRAME | DS_SETFONT, 0, 0, 0, 0, 260, 115 }, 0, 0, L"" };
                #pragma pack(pop)

                if (DialogBoxIndirectParamW(GetModuleHandle(NULL), &dlgTemplate.dt, hwnd, CategoryDlgProc, (LPARAM)&dlgData) == IDOK) {
                    std::wstring wideNewCat(dlgData.outputCategory);
                    std::string narrowNewCat(wideNewCat.begin(), wideNewCat.end());

                    if (g_BudgetManager.changeTransactionCategory(selectedTx.transactionHash, narrowNewCat)) {
                        g_BudgetManager.saveBudgetFile();
                        RefreshData(hwndListView);
                        InvalidateRect(hwndCanvasBreakdown, NULL, TRUE);
                        InvalidateRect(hwnd, NULL, TRUE);
                    }
                }
            }
        }
        return 0;
    }

    case WM_COMMAND: {
        if (HIWORD(wParam) == CBN_SELCHANGE) {
            if (LOWORD(wParam) == ID_COMBO_MONTH) {
                g_SelectedMonth = static_cast<int>(SendMessageW(hwndComboMonth, CB_GETCURSEL, 0, 0)) + 1;
            } else if (LOWORD(wParam) == ID_COMBO_YEAR) {
                wchar_t yearStr[8];
                SendMessageW(hwndComboYear, CB_GETLBTEXT, SendMessageW(hwndComboYear, CB_GETCURSEL, 0, 0), (LPARAM)yearStr);
                g_SelectedYear = _wtoi(yearStr);
            }
            RefreshData(hwndListView);
            InvalidateRect(hwndCanvasBreakdown, NULL, TRUE);
            InvalidateRect(hwnd, NULL, TRUE);
        }
        else if (HIWORD(wParam) == BN_CLICKED) {
            if (LOWORD(wParam) == ID_RADIO_MONTHLY) {
                g_IsMonthly = true;
                EnableWindow(hwndComboMonth, TRUE);
                RefreshData(hwndListView);
                InvalidateRect(hwndCanvasBreakdown, NULL, TRUE);
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (LOWORD(wParam) == ID_RADIO_YEARLY) {
                g_IsMonthly = false;
                EnableWindow(hwndComboMonth, FALSE); 
                RefreshData(hwndListView);
                InvalidateRect(hwndCanvasBreakdown, NULL, TRUE);
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (LOWORD(wParam) == ID_BTN_OPEN) {
                OnOpenFile(hwnd, hwndListView);
            }
        }
        return 0;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        SetBkColor(hdc, GetSysColor(COLOR_3DFACE));
        SetTextColor(hdc, RGB(0, 0, 0));
        
        std::wstring summaryOut = L"Total Out (Spending):\n$" + std::to_wstring(static_cast<int>(std::accumulate(g_CurrentTransactions.begin(), g_CurrentTransactions.end(), 0.0, [](double sum, const Transaction& t){ return t.amount < 0 ? sum + std::abs(t.amount) : sum; })));
        RECT rcOutText = { 25, 185, 235, 225 };
        DrawTextW(hdc, summaryOut.c_str(), -1, &rcOutText, DT_WORDBREAK);

        std::wstring summaryIn  = L"Total In (Payments):\n$" + std::to_wstring(static_cast<int>(std::accumulate(g_CurrentTransactions.begin(), g_CurrentTransactions.end(), 0.0, [](double sum, const Transaction& t){ return t.amount >= 0 ? sum + t.amount : sum; })));
        RECT rcInText = { 25, 245, 235, 285 };
        DrawTextW(hdc, summaryIn.c_str(), -1, &rcInText, DT_WORDBREAK);
        
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        CoUninitialize();
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    const wchar_t CLASS_NAME[] = L"Win98BudgetManagerClassV6";
    WNDCLASSW wc = {0};
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);

    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(0, CLASS_NAME, L"Personal Budget Manager", 
                               WS_OVERLAPPEDWINDOW, 
                               CW_USEDEFAULT, CW_USEDEFAULT, 800, 580, NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) return 0;
    ShowWindow(hwnd, nCmdShow);

    MSG msg = {0};
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return 0;
}
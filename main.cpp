#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <commctrl.h>
#include <commdlg.h> 
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <numeric>
#include "include/BudgetManager.h"

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "comdlg32.lib")

// Global State
BudgetManager g_BudgetManager;
std::vector<Transaction> g_CurrentTransactions;
bool g_IsMonthly = true;
int g_SelectedMonth = 6;
int g_SelectedYear = 2026;

#define ID_RADIO_MONTHLY 101
#define ID_RADIO_YEARLY  102
#define ID_LISTVIEW      103
#define ID_COMBO_MONTH   104
#define ID_COMBO_YEAR    105
#define ID_MENU_OPEN     106

const COLORREF K_PALETTE[] = {
    RGB(200, 50, 50),   // Red
    RGB(50, 50, 200),   // Blue
    RGB(200, 150, 50),  // Orange
    RGB(150, 50, 150),  // Purple
    RGB(50, 150, 150),  // Cyan
    RGB(120, 120, 120)  // Gray
};

void RefreshData(HWND hwndListView) {
    if (g_IsMonthly) {
        g_CurrentTransactions = g_BudgetManager.getTransactionsMonth(g_SelectedMonth, g_SelectedYear);
    } else {
        g_CurrentTransactions = g_BudgetManager.getTransactionsYear(g_SelectedYear);
    }

    // Populate List View
    ListView_DeleteAllItems(hwndListView);
    for (size_t i = 0; i < g_CurrentTransactions.size(); ++i) {
        const auto& t = g_CurrentTransactions[i];
        
        std::wstring dateStr = std::to_wstring(t.year) + L"-" + std::to_wstring(t.month) + L"-" + std::to_wstring(t.day);
        std::wstring descStr(t.Description.begin(), t.Description.end());
        std::wstring catStr(t.Category.begin(), t.Category.end());
        std::wstring amtStr = (t.amount < 0 ? L"$" : L"+$") + std::to_wstring(std::abs(t.amount)).substr(0, std::to_wstring(std::abs(t.amount)).find(L'.') + 3);

        LVITEM lvi = {0};
        lvi.mask = LVIF_TEXT;
        lvi.iItem = i;

        lvi.iSubItem = 0; lvi.pszText = (LPWSTR)dateStr.c_str(); ListView_InsertItem(hwndListView, &lvi);
        lvi.iSubItem = 1; lvi.pszText = (LPWSTR)descStr.c_str(); ListView_SetItem(hwndListView, &lvi);
        lvi.iSubItem = 2; lvi.pszText = (LPWSTR)catStr.c_str(); ListView_SetItem(hwndListView, &lvi);
        lvi.iSubItem = 3; lvi.pszText = (LPWSTR)amtStr.c_str(); ListView_SetItem(hwndListView, &lvi);
    }
}

void OnOpenFile(HWND hwnd, HWND hwndListView) {
    OPENFILENAME ofn;
    wchar_t szFile[260] = { 0 };

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"CSV Files (*.csv)\0*.csv\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn) == TRUE) {
        std::wstring ws(szFile);
        std::string filePath(ws.begin(), ws.end());
        
        if (g_BudgetManager.loadRawDataFile(filePath)) {
            g_BudgetManager.saveBudgetFile();
            RefreshData(hwndListView);
            MessageBox(hwnd, L"Data added and merged successfully!", L"Success", MB_OK | MB_ICONINFORMATION);
            InvalidateRect(hwnd, NULL, TRUE);
        } else {
            MessageBox(hwnd, L"Failed to parse the chosen CSV file.", L"Error", MB_OK | MB_ICONERROR);
        }
    }
}

void DrawBudgetCircleAndSummary(HWND hwnd, HDC hdc) {
    double totalSpending = 0.0;
    double totalPayments = 0.0;
    std::map<std::string, double> categoryMap;

    for (const auto& t : g_CurrentTransactions) {
        if (t.amount < 0) {
            double absAmt = std::abs(t.amount);
            totalSpending += absAmt;
            categoryMap[t.Category] += absAmt;
        } else {
            totalPayments += t.amount;
        }
    }

    std::wstring summaryOut = L"Total Out (Spending): $" + std::to_wstring(static_cast<int>(totalSpending));
    std::wstring summaryIn  = L"Total In (Payments):  $" + std::to_wstring(static_cast<int>(totalPayments));
    
    TextOut(hdc, 20, 155, summaryOut.c_str(), summaryOut.length());
    TextOut(hdc, 20, 175, summaryIn.c_str(), summaryIn.length());

    int left = 35, top = 205, right = 165, bottom = 335;
    int centerX = (left + right) / 2;
    int centerY = (top + bottom) / 2;
    int radius = (right - left) / 2;

    if (totalSpending == 0.0) {
        HBRUSH grayBrush = CreateSolidBrush(RGB(192, 192, 192));
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, grayBrush);
        Ellipse(hdc, left, top, right, bottom);
        SelectObject(hdc, oldBrush);
        DeleteObject(grayBrush);
        return;
    }

    double currentAngle = 0.0;
    int colorIndex = 0;
    int legendY = 350;

    for (const auto& pair : categoryMap) {
        double share = pair.second / totalSpending;
        double sweepAngle = share * 2.0 * 3.1415926535;

        int startX = centerX + static_cast<int>(radius * cos(currentAngle));
        int startY = centerY - static_cast<int>(radius * sin(currentAngle));

        currentAngle += sweepAngle;

        int endX = centerX + static_cast<int>(radius * cos(currentAngle));
        int endY = centerY - static_cast<int>(radius * sin(currentAngle));

        COLORREF color = K_PALETTE[colorIndex % 6];
        HBRUSH segmentBrush = CreateSolidBrush(color);
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, segmentBrush);

        Pie(hdc, left, top, right, bottom, startX, startY, endX, endY);

        SelectObject(hdc, oldBrush);
        DeleteObject(segmentBrush);

        HBRUSH legendBoxBrush = CreateSolidBrush(color);
        RECT rect = { 20, legendY + 2, 32, legendY + 14 };
        FillRect(hdc, &rect, legendBoxBrush);
        DeleteObject(legendBoxBrush);

        std::wstring catName(pair.first.begin(), pair.first.end());
        std::wstring label = catName + L": $" + std::to_wstring(static_cast<int>(pair.second));
        TextOut(hdc, 38, legendY, label.c_str(), label.length());

        legendY += 18;
        colorIndex++;
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hwndListView, hwndComboMonth, hwndComboYear;

    switch (uMsg) {
    case WM_CREATE: {
        InitCommonControls();
        g_BudgetManager.loadBudgetFile("local_budget.csv");

        HMENU hMenuBar = CreateMenu();
        HMENU hFileMenu = CreateMenu();
        AppendMenu(hFileMenu, MF_STRING, ID_MENU_OPEN, L"&Import Data From File Explorer...");
        AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hFileMenu, L"&File");
        SetMenu(hwnd, hMenuBar);

        CreateWindow(L"BUTTON", L"Report Settings", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 
                     10, 5, 190, 120, hwnd, NULL, NULL, NULL);

        CreateWindow(L"BUTTON", L"Monthly", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP, 
                     20, 25, 75, 20, hwnd, (HMENU)ID_RADIO_MONTHLY, NULL, NULL);
        CheckDlgButton(hwnd, ID_RADIO_MONTHLY, BST_CHECKED);

        CreateWindow(L"BUTTON", L"Yearly", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 
                     110, 25, 75, 20, hwnd, (HMENU)ID_RADIO_YEARLY, NULL, NULL);

        hwndComboMonth = CreateWindow(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL, 
                                      20, 55, 160, 200, hwnd, (HMENU)ID_COMBO_MONTH, NULL, NULL);
        const wchar_t* months[] = { L"January", L"February", L"March", L"April", L"May", L"June", 
                                    L"July", L"August", L"September", L"October", L"November", L"December" };
        for (int i = 0; i < 12; ++i) SendMessage(hwndComboMonth, CB_ADDSTRING, 0, (LPARAM)months[i]);
        SendMessage(hwndComboMonth, CB_SETCURSEL, g_SelectedMonth - 1, 0);

        hwndComboYear = CreateWindow(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL, 
                                     20, 85, 160, 200, hwnd, (HMENU)ID_COMBO_YEAR, NULL, NULL);
        const wchar_t* years[] = { L"2024", L"2025", L"2026", L"2027" };
        for (int i = 0; i < 4; ++i) SendMessage(hwndComboYear, CB_ADDSTRING, 0, (LPARAM)years[i]);
        SendMessage(hwndComboYear, CB_SETCURSEL, 2, 0); // Default to 2026

        CreateWindow(L"BUTTON", L"Visual Breakdown (Category Spending)", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 
                     10, 135, 190, 310, hwnd, NULL, NULL, NULL);

        hwndListView = CreateWindow(WC_LISTVIEW, L"", WS_CHILD | WS_VISIBLE | LVS_REPORT | WS_BORDER, 
                                    210, 12, 560, 433, hwnd, (HMENU)ID_LISTVIEW, NULL, NULL);

        LVCOLUMN lvc = {0};
        lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
        lvc.pszText = (LPWSTR)L"Date"; lvc.cx = 85; ListView_InsertColumn(hwndListView, 0, &lvc);
        lvc.pszText = (LPWSTR)L"Description"; lvc.cx = 245; ListView_InsertColumn(hwndListView, 1, &lvc);
        lvc.pszText = (LPWSTR)L"Category"; lvc.cx = 120; ListView_InsertColumn(hwndListView, 2, &lvc);
        lvc.pszText = (LPWSTR)L"Amount"; lvc.cx = 90; ListView_InsertColumn(hwndListView, 3, &lvc);

        EnumChildWindows(hwnd, [](HWND child, LPARAM lp) -> BOOL {
            SendMessage(child, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
            return TRUE;
        }, 0);

        RefreshData(hwndListView);
        return 0;
    }

    case WM_COMMAND: {
        if (HIWORD(wParam) == CBN_SELCHANGE) {
            if (LOWORD(wParam) == ID_COMBO_MONTH) {
                g_SelectedMonth = (int)SendMessage(hwndComboMonth, CB_GETCURSEL, 0, 0) + 1;
            } else if (LOWORD(wParam) == ID_COMBO_YEAR) {
                wchar_t yearStr[8];
                SendMessage(hwndComboYear, CB_GETLBTEXT, SendMessage(hwndComboYear, CB_GETCURSEL, 0, 0), (LPARAM)yearStr);
                g_SelectedYear = _wtoi(yearStr);
            }
            RefreshData(hwndListView);
            InvalidateRect(hwnd, NULL, TRUE);
        }
        else if (HIWORD(wParam) == BN_CLICKED) {
            if (LOWORD(wParam) == ID_RADIO_MONTHLY) {
                g_IsMonthly = true;
                EnableWindow(hwndComboMonth, TRUE);
            } else if (LOWORD(wParam) == ID_RADIO_YEARLY) {
                g_IsMonthly = false;
                EnableWindow(hwndComboMonth, FALSE); 
            }
            RefreshData(hwndListView);
            InvalidateRect(hwnd, NULL, TRUE);
        }
        else if (LOWORD(wParam) == ID_MENU_OPEN) {
            OnOpenFile(hwnd, hwndListView);
        }
        return 0;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        SetBkColor(hdc, GetSysColor(COLOR_3DFACE));
        SetTextColor(hdc, RGB(0, 0, 0));
        
        DrawBudgetCircleAndSummary(hwnd, hdc);
        
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"Win98BudgetManagerClassV2";
    WNDCLASS wc = {0};
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, CLASS_NAME, L"Personal Budget Manager (Win32 Extended Edition)", 
                               WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, 
                               CW_USEDEFAULT, CW_USEDEFAULT, 800, 515, NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) return 0;
    ShowWindow(hwnd, nCmdShow);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
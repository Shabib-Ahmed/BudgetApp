#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <commctrl.h>
#include <string>
#include <vector>
#include <cmath>
#include "include/BudgetManager.h"

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "comctl32.lib")

BudgetManager g_BudgetManager;
std::vector<Transaction> g_CurrentTransactions;
bool g_IsMonthly = true;
int g_SelectedMonth = 6;  // June
int g_SelectedYear = 2026;

#define ID_RADIO_MONTHLY 101
#define ID_RADIO_YEARLY  102
#define ID_LISTVIEW      103

void RefreshData() {
    if (g_IsMonthly) {
        g_CurrentTransactions = g_BudgetManager.getTransactionsMonth(g_SelectedMonth, g_SelectedYear);
    } else {
        g_CurrentTransactions = g_BudgetManager.getTransactionsYear(g_SelectedYear);
    }
}

void DrawBudgetCircle(HWND hwnd, HDC hdc) {
    double totalSpending = 0.0;
    double totalPayments = 0.0;
    for (const auto& t : g_CurrentTransactions) {
        if (t.amount < 0) totalSpending += std::abs(t.amount);
        else totalPayments += t.amount;
    }
    double total = totalSpending + totalPayments;

    int left = 30, top = 180, right = 170, bottom = 320;
    int centerX = (left + right) / 2;
    int centerY = (top + bottom) / 2;
    int radius = (right - left) / 2;

    if (total == 0.0) {
        HBRUSH grayBrush = CreateSolidBrush(RGB(128, 128, 128));
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, grayBrush);
        Ellipse(hdc, left, top, right, bottom);
        SelectObject(hdc, oldBrush);
        DeleteObject(grayBrush);
        return;
    }

    double spendingShare = totalSpending / total;
    double spendingAngle = spendingShare * 2.0 * 3.1415926535;

    int spendingEndX = centerX + static_cast<int>(radius * cos(spendingAngle));
    int spendingEndY = centerY - static_cast<int>(radius * sin(spendingAngle)); // Native GDI Y-axis goes down

    HBRUSH redBrush = CreateSolidBrush(RGB(200, 50, 50));
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, redBrush);
    // Pie parameters: bounding box, starting line coordinate (3 o'clock point), ending line coordinate
    Pie(hdc, left, top, right, bottom, right, centerY, spendingEndX, spendingEndY);

    HBRUSH greenBrush = CreateSolidBrush(RGB(50, 150, 50));
    SelectObject(hdc, greenBrush);
    Pie(hdc, left, top, right, bottom, spendingEndX, spendingEndY, right, centerY);

    SelectObject(hdc, oldBrush);
    DeleteObject(redBrush);
    DeleteObject(greenBrush);

    std::wstring spendingStr = L"Out (Spending): $" + std::to_wstring(static_cast<int>(totalSpending));
    std::wstring paymentsStr = L"In (Payments): $" + std::to_wstring(static_cast<int>(totalPayments));
    TextOut(hdc, 20, 340, spendingStr.c_str(), spendingStr.length());
    TextOut(hdc, 20, 360, paymentsStr.c_str(), paymentsStr.length());
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hwndListView;

    switch (uMsg) {
    case WM_CREATE: {
        InitCommonControls();

        g_BudgetManager.loadRawDataFile("data.csv");
        g_BudgetManager.saveBudgetFile();
        RefreshData();

        CreateWindow(L"BUTTON", L"Report Settings", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 
                     10, 10, 190, 100, hwnd, NULL, NULL, NULL);

        CreateWindow(L"BUTTON", L"Monthly Report", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP, 
                     20, 35, 150, 20, hwnd, (HMENU)ID_RADIO_MONTHLY, NULL, NULL);
        CheckDlgButton(hwnd, ID_RADIO_MONTHLY, BST_CHECKED);

        CreateWindow(L"BUTTON", L"Yearly Report", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 
                     20, 65, 150, 20, hwnd, (HMENU)ID_RADIO_YEARLY, NULL, NULL);

        CreateWindow(L"BUTTON", L"Visual Breakdown", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 
                     10, 130, 190, 270, hwnd, NULL, NULL, NULL);

        hwndListView = CreateWindow(WC_LISTVIEW, L"", WS_CHILD | WS_VISIBLE | LVS_REPORT | WS_BORDER, 
                                    210, 18, 560, 382, hwnd, (HMENU)ID_LISTVIEW, NULL, NULL);

        LVCOLUMN lvc = {0};
        lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
        
        lvc.pszText = (LPWSTR)L"Date"; lvc.cx = 90; ListView_InsertColumn(hwndListView, 0, &lvc);
        lvc.pszText = (LPWSTR)L"Description"; lvc.cx = 240; ListView_InsertColumn(hwndListView, 1, &lvc);
        lvc.pszText = (LPWSTR)L"Category"; lvc.cx = 110; ListView_InsertColumn(hwndListView, 2, &lvc);
        lvc.pszText = (LPWSTR)L"Amount"; lvc.cx = 100; ListView_InsertColumn(hwndListView, 3, &lvc);

        SendMessage(hwnd, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
        EnumChildWindows(hwnd, [](HWND child, LPARAM lp) -> BOOL {
            SendMessage(child, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
            return TRUE;
        }, 0);

        SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_RADIO_MONTHLY, BN_CLICKED), 0);
        return 0;
    }

    case WM_COMMAND: {
        if (HIWORD(wParam) == BN_CLICKED) {
            if (LOWORD(wParam) == ID_RADIO_MONTHLY) {
                g_IsMonthly = true;
                RefreshData();
            } else if (LOWORD(wParam) == ID_RADIO_YEARLY) {
                g_IsMonthly = false;
                RefreshData();
            }

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

            InvalidateRect(hwnd, NULL, TRUE);
        }
        return 0;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        
        SetBkColor(hdc, GetSysColor(COLOR_3DFACE));
        SetTextColor(hdc, RGB(0, 0, 0));

        DrawBudgetCircle(hwnd, hdc);

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
    const wchar_t CLASS_NAME[] = L"Win98BudgetManagerClass";
    
    WNDCLASS wc = {0};
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1); 

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, CLASS_NAME, L"Personal Budget Manager (Win32 Classic Edition)", 
                               WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, 
                               CW_USEDEFAULT, CW_USEDEFAULT, 800, 450, NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) return 0;

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
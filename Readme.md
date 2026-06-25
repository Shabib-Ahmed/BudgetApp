# Personal Budget Manager

A lightweight Windows desktop application for tracking and visualizing personal finances from credit card statements. Import your creditcard CSV export, categorize transactions, and get a clear monthly or yearly spending breakdown locally with no accounts or subscriptions required.

---

## Features

- **CSV Import** — Import directly from your bank's statement export
- **Supported statements:** Chase credit card (more planned)
- **Duplicate detection** — Hash-based deduplication prevents the same transaction from being imported twice across multiple files
- **Monthly / Yearly views** — Toggle between a month-by-month or full-year transaction list
- **Category editor** — Double-click any transaction to rename its category on the spot
- **Visual breakdown** — Scrollable pie chart showing spending by category with a color-coded legend
- **Financial summary** — At-a-glance totals for spending (out) and payments (in)
- **Persistent storage** — Transactions are saved to a local CSV budget file and reloaded automatically

---

## Getting Started

### Prerequisites

- Windows 10 or later
- Visual Studio 2019+ or any compiler with C++17 and Win32 support
- CMake 3.10+

### Building

```bash
git clone https://github.com/yourusername/BudgetApp.git
cd BudgetApp
mkdir build && cd build
cmake ..
cmake --build .
```

The executable will be placed in the `build` directory. A `data.csv` file is copied automatically to the output directory on build.

### Running

Launch `MyApp.exe`. On first run, click **Import CSV Data** to load a statement file.

---

## Usage

1. **Import a statement** — Click *Import CSV Data* and select a CSV exported from a supported bank. Transactions are parsed and deduplicated automatically.
2. **Browse transactions** — Use the Month/Year dropdowns and the Monthly/Yearly radio buttons to filter the transaction list.
3. **Edit a category** — Double-click any row in the transaction list to rename its category. Changes are saved immediately.
4. **View spending breakdown** — The left panel shows total spending, total payments, and a pie chart broken down by category.

---

## Supported CSV Formats

| Bank / Card | Format | Status |
|---|---|---|
| Chase Credit Card | `Date, Post Date, Description, Category, Type, Amount, Memo` | ✅ Supported |
| More | — | 🔜 Planned |

---

## Project Structure

```
BudgetApp/
├── main.cpp               # Win32 GUI, window/event logic
├── include/
│   └── BudgetManager.h    # Transaction management interface
├── src/
│   └── BudgetManager.cpp  # CSV loading, filtering, saving logic
├── libs/
│   └── csvLibrary/
│       ├── CSVReader.h
│       └── CSVReader.cpp  # CSV read/write utilities
├── CMakeLists.txt
└── data.csv               # Local budget data file (auto-generated)
```

---

## Roadmap

- [ ] Support for additional bank statement formats
- [ ] Export filtered views to CSV
- [ ] Month-over-month spending comparison
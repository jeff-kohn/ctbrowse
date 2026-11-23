# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

ctBrowse is a Windows-only desktop app (wxWidgets, C++23) for browsing, searching, and filtering wine data downloaded from CellarTracker.com.

## Build

Dependencies come from vcpkg in manifest mode (`vcpkg.json`, custom triplets/ports in `scripts/overlay_triplets` and `scripts/overlay_ports`). The `VCPKG_INSTALLATION_ROOT` environment variable must point at a vcpkg checkout. Build output goes to `build/<preset>/`.

```powershell
# MSVC (Visual Studio 18 2026 generator, what CI uses)
cmake --preset win-msvc
cmake --build --preset win-msvc-debug      # or win-msvc-release

# clang-cl + Ninja Multi-Config
cmake --preset win-clang
cmake --build --preset win-clang-debug     # or win-clang-release

# Static analysis (clang-tidy + cppcheck), as run in CI
cmake --preset ci-static-analysis
cmake --build --preset ci-static-analysis --target ctBrowse_app
cmake --build --preset ci-static-analysis --target code_analysis
```

Warnings are errors (`/W4 /WX` plus many extra MSVC warnings). Code must compile cleanly under both MSVC and clang-cl.

### Tests

The presets set `CTB_BUILD_TESTS=OFF`. To build them, configure with `-DCTB_BUILD_TESTS=ON`. Tests use Catch2 (`tests/ctb_test.cpp`, target `ctb_test`, which links `ctBrowse_lib`) and are registered with `catch_discover_tests`. The test file currently has no tests. To run them:

```powershell
ctest --test-dir build/win-msvc -C Debug                    # all tests
ctest --test-dir build/win-msvc -C Debug -R "<test name>"   # single test
```

## Architecture

There are two targets:

- **`ctBrowse_lib`** (static library). Public headers are in `include/ctb/` and sources in `ctBrowse_lib/`. It has no wxWidgets dependency.
- **`ctBrowse_app`** (WIN32 GUI exe, `ctBrowse_app/`). This is the wxWidgets UI, in namespace `ctb::app`.

New targets should use the helpers in `cmake/create_targets.cmake` (`create_lib_target`, `create_exe_target`, `create_windows_target`) so they get the shared compile/link/warning flags. **New source files must be added explicitly** to the `target_sources` list in the relevant `CMakeLists.txt`.

### Data model (library)

- **Tables are defined by traits classes** in `include/ctb/tables/*Traits.h`, one per CellarTracker CSV export (WineList, Pending, Consumed, etc.). Each traits class declares:
  - `Schema`: a `frozen::map` of `CtProp` to `FieldSchema`, giving the type and CSV column index. Calculated fields have no index (`{}`).
  - `DefaultListColumns`, `AvailableSorts`, and `MultiValueFilters`
  - `getTableId()`
  - `onRecordParse()`, which fixes up parsed values and computes calculated properties

  `CtDataTable<Traits>` is the table type, and the generic machinery lives in `include/ctb/tables/detail/`. Adding a new table means adding a traits header, a `TableId`, any new `CtProp` enum values, and wiring in the dataset manager and UI.
- **`IDataset`** (`interfaces/IDataset.h`) is the type-erased interface the UI works against. It uses a "current row" cursor model and provides filtering, sorting, and distinct-value queries. `CtDataset<DataTableT>` (`model/CtDataset.h`) is the single template implementation. Use `DatasetPtr = shared_ptr<IDataset>`. Datasets are never empty and are **not thread-safe**: use them only on the UI thread. `freezeData()`/`unfreezeData()` (or `ScopedDatasetFreeze`) batch filter/sort changes.
- **`CtDatasetMgr`** (`model/CtDatasetMgr.h`, PIMPL with `async/DatasetMgrAsyncImpl`) loads datasets from disk and performs async operations: table downloads from CellarTracker, label image retrieval, and the pro-reviews cache. Async work uses **stdexec senders on standalone asio** (`ctBrowse_lib/async/`). Results come back through callbacks (`DatasetMgrAsyncCallbacks.h`), and the UI must marshal them to the main thread.
- **Headless browser** (`ctBrowse_lib/browser/`): label images and login use a headless Edge instance driven over the Chrome DevTools websocket protocol (`DatasetMgrOptions::browser_path` / `browser_ws_port`). Image download is disabled if `browser_path` is empty.
- **Errors**: throw or return `ctb::Error` (`Error.h`), which carries a `Category`. Use `packageError()` to convert any in-flight exception into a `ctb::Error`, e.g. for `std::expected`-style returns.

### UI (app)

- UI components use an **observer pattern**: `IDatasetEventSource` (implemented by `DatasetEventSource`) holds the active `DatasetPtr` and broadcasts `DatasetEvent::Id` events to attached `IDatasetEventSink`s. Views must `detach()` before they are destroyed. When a view initiates a change, it passes itself as `event_source` to `signal()` to avoid receiving its own event.
- `MainFrame` hosts `DatasetMultiView`, a splitter with three panes: `DatasetOptionsView` (sort/filter), `DatasetListView` (the list, backed by `model/CtDataViewModel`), and a `DetailsViewBase`-derived details pane. There is one details view per table (`views/DetailsView*.h`), and each is composed of `panels/WineDetail*Panel`.
- wx window classes use a private constructor plus a static `create(parent, ...)` factory. The factory returns a non-owning pointer because wx windows are self-deleting, and it throws `ctb::Error` on failure.
- User-visible strings and format strings are kept in `include/ctb/constants.h` and `ctBrowse_app/app_constants.h` rather than inline.

## Conventions

- Format with the repo's `.clang-format`: 3-space indent, 140-column limit, Allman braces, all namespaces indented, and consecutive assignments/declarations aligned. Text files use LF line endings (see `docs/VISUAL_STUDIO_CONFIGURATION.md`).
- Naming:
  - Types: `PascalCase`
  - Methods: `camelCase`
  - Locals and parameters: `snake_case`
  - Members: `m_` prefix
  - Constants: `UPPER_SNAKE`
  - Interfaces: `I` prefix
- Prefer trailing return types (`auto f() -> T`) and `[[nodiscard]]` on getters. Headers use `#pragma once` and a Doxygen file banner (`@file`, `@brief`, `@copyright`), with `///` Doxygen comments on declarations.
- Include library headers as `"ctb/..."`.
- `std::format`-style formatting goes through `ctb::format`, which uses fmt. Enum-to-string conversion uses enchantum (`enum_to_string`).
- Suppress clang-tidy checks with an inline `// NOLINT [check-name] reason`.

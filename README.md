# Custom Text Editor

A terminal-based text editor built from scratch in C++17, inspired by the kilo editor. Supports syntax highlighting, undo/redo, incremental search, and a side by side file diff tool.

## Features

- Raw terminal I/O with RAII-based cleanup
- Text editing with undo/redo
- Syntax highlighting
- Incremental search and replace
- Side by side file diff
- Status bar with file info and document stats

## Build

Requires a C++17 compiler and CMake 3.16+. Build inside WSL on Windows.

```bash
mkdir build && cd build
cmake ..
make
```

## Run

```bash
# Open a file
./kilo filename.cpp

# New empty buffer
./kilo

# Diff two files
./kilo --diff file1.cpp file2.cpp
```

## Keybindings

| Key | Action |
|---|---|
| Ctrl-S | Save |
| Ctrl-Q | Quit (press twice with unsaved changes) |
| Ctrl-Z | Undo |
| Ctrl-Y | Redo |
| Ctrl-F | Find |
| Ctrl-R | Find and replace |
| Arrows | Move cursor |
| Home/End | Start/end of line |
| Page Up/Down | Scroll by screenful |

### Diff viewer

| Key | Action |
|---|---|
| n | Next/previous hunk |
| Arrows | Scroll |
| q | Quit |

## Tests

```bash
cd build
./kilo_tests
```

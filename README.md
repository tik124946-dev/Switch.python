# SwitchPy IDE — Android-friendly cloud build

This project is intended to be built from an Android phone using GitHub Actions (cloud build), so no PC or local devkitPro installation is needed.

## What it currently does
- line-based Python editor
- simple syntax highlighting
- on-screen keyboard for editing lines
- save/open `.py` files on the SD card
- Joy-Con/D-pad controls

## Important
The Run button is currently a placeholder. This build does **not** execute Python scripts yet. A compatible Python runtime (such as a suitable MicroPython/CPython port) must be integrated and tested separately.

## Build from an Android phone
1. Sign in to GitHub in your phone browser and create a new repository.
2. Upload all files from this project to the repository, preserving the folder structure, including `.github/workflows/build.yml`.
3. Open the repository's **Actions** tab.
4. Select **Build SwitchPy IDE** and tap **Run workflow** (or push to `main`).
5. Wait for the workflow to finish successfully.
6. Open the completed workflow run, find **Artifacts**, and download `SwitchPyIDE-nro`.
7. Extract the downloaded artifact ZIP. It should contain `SwitchPyIDE.nro`.
8. Copy `SwitchPyIDE.nro` to `sd:/switch/SwitchPyIDE/SwitchPyIDE.nro`, then launch it in Homebrew Menu.

GitHub Actions can change its interface; use the repository's Actions workflow page if the mobile layout differs.

## Controls
- D-pad Up/Down: select line
- A: edit selected line
- R: insert a blank line
- B: delete line
- X: save `sdmc:/switch/SwitchPyIDE/main.py`
- Y: open a file path using the on-screen keyboard
- L: show the runtime placeholder message
- Plus: exit

## Build prerequisites
The workflow uses the `devkitpro/devkita64` container, which provides the Nintendo Switch compiler toolchain. See the Switchbrew development setup documentation and devkitPro resources for background.

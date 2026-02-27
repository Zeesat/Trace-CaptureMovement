# Trace 2

## Project Structure

- `src/python/trace_controller.py`: main hotkey controller (record, compile, replay)
- `data/`: trace data files (`trace.json`, `trace.raw`)
- `build/`: generated C files and replay executables
- `tools/`: required recorder binary (`CTraceInputKMGPT.exe`)
- `legacy/`: old or experimental files preserved for reference
- `.vscode/`: local editor settings

## Quick Start

1. Install Python dependency:
   - `pip install pynput`
2. Make sure `gcc` is available in PATH (for compile step)
3. Run:
   - `python src/python/trace_controller.py`

## Hotkeys

- `F4`: start recording
- `F5`: stop recording
- `F6`: compile trace into C + EXE
- `F2`: start replay
- `F3`: stop replay
- `F7`: toggle mouse replay on/off
- `F8`: toggle loop on/off

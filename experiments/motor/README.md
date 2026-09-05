# Motor experiments

Reserved for isolated motor investigations. No experiment is implemented yet.

Record confirmed hardware, wiring, dependencies, procedure and observations before adding code. This directory is not selected by the current build. When an experiment is ready, keep its implementation and header here, call its setup/loop functions from `src/main.cpp`, and select its source files in the root `platformio.ini` using `build_src_filter`. Only `src/main.cpp` should define Arduino `setup()` and `loop()`. Use the existing `mega2560` environment; see the [launcher instructions](../../README.md#running-experiments).

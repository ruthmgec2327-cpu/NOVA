# Contributing to NOVA

Thanks for considering a contribution. This is a small hobbyist ESP32 voice-assistant project, so the process is intentionally lightweight.

## Before you start

Read the main [README.md](README.md) to understand the hardware, wiring, and state machine before changing anything.

## Development setup

1. Install Arduino IDE (2.x recommended) with the ESP32-S3 board package.
2. Install the libraries listed in the README's [Required Arduino Libraries](README.md#required-arduino-libraries) table.
3. Fill in your WiFi/API credentials in `config.h`.
4. Set board settings exactly as described in [Board Settings](README.md#board-settings-arduino-ide) — particularly **PSRAM: OPI PSRAM**, or audio buffer allocation will fail at runtime.

## Making changes

- Keep hardware pin assignments centralized in `config.h` — don't hardcode GPIO numbers in `.cpp` files.
- If you change a pin mapping or add new hardware, update both `config.h` and the wiring tables in `README.md`.
- If you touch the state machine in `EvoAssistant.ino`, update the state-flow diagram in the README to match.

## Testing changes

There's no automated test suite (this is firmware on real hardware), so testing is manual:

- Flash to actual hardware and check Serial Monitor output against the "Expected Serial Output" section in the README.

## Pull requests

- Keep PRs focused on one change (one new feature, one bugfix) where possible.
- Describe what hardware you tested on (board variant, OLED driver, etc.) since this project is sensitive to exact wiring and library versions.

## Reporting bugs

Open a GitHub issue with:
- What you expected to happen vs. what happened.
- Relevant Serial Monitor output.
- Board variant, library versions, and wiring (if hardware-related).

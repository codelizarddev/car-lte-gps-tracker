# Contributing

Contributions are welcome! Here's how to get started.

## Development workflow

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/my-feature`
3. Make your changes
4. Run the unit tests: `cd firmware/test && make`
5. Commit with a clear message
6. Open a Pull Request against `main`

## Running tests before submitting

All tests must pass:

```bash
cd firmware/test/
make
# Expected: Test suites: 4 total, 4 passed, 0 failed
```

The CI pipeline (GitHub Actions) runs tests automatically on every PR.

## Code style

- C code follows the ESP-IDF style: 4-space indent, snake_case
- Header guards use `#pragma once`
- All public functions have a Doxygen-style comment in the `.h` file
- No magic numbers — use `#define` constants with clear names

## What to contribute

- Bug fixes
- Support for other boards (ESP32-S3, other LTE modules)
- Additional MQTT topics (ignition state, door sensors, etc.)
- OTA firmware update support
- Better power source detection (e.g. ADC on 5V rail)
- Windows support for the AT simulator

## Questions?

Open an issue with your question — no question is too small.

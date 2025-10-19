# Contributing to NFC Laboratory

Thank you for your interest in contributing to **nfc-laboratory**!  
This project is an open source NFC signal sniffer and protocol decoder using SDR and logic analyzers.  
We welcome contributions of all kinds: bug fixes, features, documentation improvements, examples, tests, etc.

---

## Table of Contents

1. [What to Contribute](#what-to-contribute)
2. [Reporting Issues](#-reporting-issues)
3. [Suggesting Enhancements](#-suggesting-enhancements)
4. [Code Style & Guidelines](#-code-style--guidelines)
5. [How to Submit a Pull Request](#-how-to-submit-a-pull-request-pr)
6. [License & Contributor Agreement](#-license--contributor-agreement)
7. [Thank You](#-thank-you)

---

## What to Contribute

Here are some ways you can help:

- Fix bugs in decoding, demodulation, or signal capture
- Add support for new NFC standards, modulation schemes or SDR devices
- Improve performance or optimize existing code
- Add or extend tests (especially for different modulation, file formats, etc.)
- Improve documentation, tutorials, or example usage
- Provide sample captures (WAV, TRZ) for edge cases
- UI / usability enhancements to the Qt interface
- Improvements in build / CI / packaging scripts

If you're unsure where to start, you might look at open issues labeled “good first issue” (if used), or areas in the code where you see missing functionality or bugs.

---

## 🐛 Reporting Issues

If you find a bug or an unexpected behavior:
1. Check the [issue tracker](https://github.com/josevcm/nfc-laboratory/issues) to see if it’s already reported.
2. If not, open a **new issue** and include:
   - A clear title
   - Steps to reproduce (if possible)
   - The environment: OS, Qt version, SDR / logic analyzer hardware, etc.
   - Relevant logs, error messages, or screenshots
   - Sample capture file (WAV / TRZ) if possible
   - Expected vs actual behavior

---

## 🌱 Suggesting Enhancements

When proposing a new feature or enhancement:
- Explain **why** it would be useful.
- Describe **how** it should work.
- Include any relevant examples or references.

---

## 🧪 Code Style & Guidelines

To maintain consistency:

- Use C++17 (or newer) features cautiously, but prefer readability
- Follow the existing coding style (brace placement, indentations, naming, etc.)
- I use CLion, formatting config can be found in the repository (clion-c++-formatter.xml).
- Comment your code where nontrivial logic is implemented
- Avoid large monolithic commits; break your work into logical parts
- If introducing a new module or component, add documentation and examples
- For external libraries, keep track of licenses and attributions (e.g. AirSpy, nlohmann::json, microtar, etc.)

---

## 💻 How to Submit a Pull Request (PR)

1. Make sure your branch is up to date with upstream master (or default branch).
2. Rebase or merge, resolve conflicts, run tests again.
3. Push your branch to your fork.
4. Open a PR against **develop** branch in josevcm/nfc-laboratory (do not target master/main directly).
5. In the PR description, include:
   - What problem you are solving
   - How you implemented it
   - Any limitations, assumptions, or side effects
   - If applicable, instructions to test your changes
   - Any new tests or sample files you added
6. Be responsive to review feedback; maintainers may ask for changes, or do modifications themselves without consulting you.

---

## 🧾 License & Contributor Agreement

By contributing, you agree that your contributions will be licensed under the same license as the project.
You represent that your contributions are original or properly licensed for inclusion.
If your change pulls in new third-party code or libraries, ensure their license is compatible and document it in your PR.

---

## ❤️ Thank You

Your contributions help make this project better for everyone. Thank you for your time, effort, and ideas!
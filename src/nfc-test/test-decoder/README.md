# test-decoder

Regression test for `lab::NfcDecoder`. It decodes radio captures (`.wav`) and compares the decoded frames with the
expected frames in a `.json` file with the same name (trace storage format). Only the fields present in each expected
frame are compared, and carrier frames are ignored.

```
ctest --test-dir build -R decoder                      # all decoder tests
build/src/nfc-test/test-decoder/test-decoder [-v] wav/test_NFC-A_106kbps_001.wav ...
```

Tests registered with CTest:

- `decoder-test_*`: one test per reference capture in `wav/`.
- `decoder-nfca-synthetic-*`: synthetic NFC-A captures generated at test time by `scripts/nfca_synthetic.py`
  (requires Python 3). They cover protocol and robustness cases not present in the reference captures.

See `doc/nfca.md`, section 10, for the description of each scenario.

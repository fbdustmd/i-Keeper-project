# Tests
Run `.\tests\smoke.ps1` from the project root. It builds with warnings treated
as errors, then checks four CLI cases (exit codes and stdout/stderr).
Success prints `All smoke tests passed.` and exits 0; any failure exits 1.
See docs/TESTING.md for details. No parser or capture tests exist yet.

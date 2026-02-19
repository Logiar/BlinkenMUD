# BlinkenMUD Code Quality Review (Prioritized Recommendations)

## Scope and approach

This review used lightweight static signals that are already available in the repository:

- Successful baseline build/test run via `make -C src test`.
- Compiler warning output from that build.
- Quick codebase metrics (`wc -l`, simple token searches).
- CI pipeline inspection (`.github/workflows/ci.yml`).

## Key findings

- The project compiles and the included test target passes, but the build emits many warnings (including format-overflow warnings and pointer-compare warnings).
- The codebase uses many unbounded C string APIs (`sprintf`, `strcat`, `strcpy`).
- Several source files are very large (5k-6k LOC), increasing change risk and review overhead.
- CI currently validates build + integration smoke test but does not fail on warnings or run sanitizers.

## Prioritized recommendations

### P0 — Reduce memory-safety risk from unbounded string operations

**Why this is top priority:** Buffer construction is a high-risk area in C, and the current code uses unbounded APIs extensively.

**Evidence:**
- `sprintf` usage count: **1111**
- `strcat` usage count: **575**
- `strcpy` usage count: **61**

**Recommendation:**
1. Introduce safe wrappers/macros (`mud_snprintf`, bounded append helper) in one shared header.
2. Migrate hottest paths first (communication/output formatting and command handlers).
3. Add policy: no new `sprintf`/`strcat`/`strcpy` in new code.

**Expected outcome:** materially lower overflow risk and cleaner compiler diagnostics.

### P1 — Eliminate existing compiler warnings and enforce warning budget

**Why:** Existing warnings hide real regressions and reduce trust in builds.

**Evidence from current build:**
- Multiple `-Wformat-overflow` warnings in high-traffic files (for example `act_info.c`, `wizlist.c`).
- Pointer-vs-character constant comparisons in `olc_act.c` (`strstr(...) != '\0'`).

**Recommendation:**
1. Fix warning classes in batches:
   - pointer-comparison correctness fixes first,
   - then format/size correctness (`snprintf`, bounds checks).
2. After warning count is near-zero, add `-Werror` in CI (or targeted `-Werror=<class>` rollout).

**Expected outcome:** build signal becomes reliable; new defects surface immediately.

### P1 — Break up oversized translation units for maintainability

**Why:** Very large files slow review and increase merge/conflict risk.

**Evidence (largest files):**
- `src/act_wiz.c` — 6108 lines
- `src/magic.c` — 5308 lines
- `src/fight.c` — 5292 lines
- `src/olc_act.c` — 5125 lines
- `src/const.c` — 5020 lines

**Recommendation:**
1. Split by responsibility (e.g., `act_wiz_admin.c`, `act_wiz_player.c`; `magic_damage.c`, `magic_support.c`).
2. Keep public entry points stable while relocating internal helpers.
3. Enforce file-size soft limits for new modules.

**Expected outcome:** easier code ownership, faster onboarding, safer refactors.

### P2 — Strengthen CI quality gates with sanitizers and stricter matrix

**Why:** Current CI is good baseline coverage but leaves memory/UB classes under-tested.

**Current CI checks:** build with gcc+clang, run `make test`, and run integration bot smoke test.

**Recommendation:**
1. Add sanitizer jobs (ASan+UBSan) for at least one compiler target.
2. Add a stricter warning job (`-Werror` once warning backlog is reduced).
3. Optionally add a static analysis pass (clang-tidy/cppcheck) as non-blocking first.

**Expected outcome:** earlier detection of memory bugs and undefined behavior.

### P2 — Expand integration coverage into targeted regression scenarios

**Why:** Existing integration test is useful but exercises one happy-path session.

**Recommendation:**
1. Add scripted cases for historically risky behaviors (command parsing edge cases, long input lines, malformed telnet data, permission checks).
2. Keep tests deterministic with fixed seeds and isolated player artifacts.
3. Promote critical regressions to mandatory CI checks.

**Expected outcome:** better production confidence during legacy refactors.

## Suggested execution order (first 30 days)

1. **Week 1:** warning triage + fix obvious correctness warnings (pointer compare and high-confidence format fixes).
2. **Week 2:** introduce safe string helper layer and migrate top-risk callsites.
3. **Week 3:** add sanitizer CI job and begin warning-to-error rollout.
4. **Week 4:** split one large file as a template refactor and add two focused regression integration tests.


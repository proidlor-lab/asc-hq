# Documentation Organization

**Date**: 2025-11-08 21:24 UTC  
**Action**: Reorganized markdown files into active vs historical

---

## 📁 New Structure

### Root `source/ai/mcts/` - Active Documentation (8 files)

**Purpose**: Current reference, guides, and living documents

1. **README.md** - Project overview & quick start
2. **STATUS.md** - Current phase progress, next steps, known issues
3. **PROJECT_CHARTER.md** - Goals, scope, requirements
4. **QUICKSTART.md** - Build instructions & troubleshooting
5. **INTEGRATION_NOTES.md** - Legacy code integration status & bugs fixed
6. **HEADLESS_MODE_INTEGRATION.md** - Command-line AI selection guide
7. **VERIFICATION_GUIDE.md** - How to verify MCTS is working
8. **SPECIFICATION_GAPS.md** - Known limitations, TODOs, future work

**Characteristics**:
- Reference documentation
- Living documents (updated as development continues)
- Essential for new contributors
- Contains current status and next steps
- Tracks ongoing technical debt and future improvements

---

### `docs/history/` - Historical Documentation (14 files)

**Purpose**: Completed milestones, bug reports, and phase summaries

**Phase Completion Reports**:
- PHASE_0_COMPLETE.md
- PHASE_0.2_SUMMARY.md
- PHASE_0.3_SUMMARY.md
- PHASE_1.1b_INTEGRATION_SKELETON.md
- PHASE_1.1b_IMPLEMENTATION_STATUS.md
- PHASE_1.1b_COMPILATION_SUCCESS.md
- PHASE_1.1b_COMPLETE.md
- BUILD_SUCCESS_FINAL.md
- UI_INTEGRATION_COMPLETE.md
- IMPLEMENTATION_SUMMARY.md

**Bug Fixes & Issues**:
- BUGFIX_SWAPPLAYERS.md - Critical bugs fixed (2025-11-08)

**Design Documents**:
- AI_INTEGRATION_OPTIONS.md
- MANUAL_TEST_GUIDE.md

**Index**:
- INDEX.md - Complete index of historical docs

**Characteristics**:
- Snapshot of completed work
- Historical reference
- Bug fix documentation
- Not updated (frozen in time)

---

## Why This Organization?

### Benefits

1. **Cleaner Root Directory**
   - Only 7 active docs instead of 22
   - Easier to find current information
   - Less overwhelming for new contributors

2. **Preserved History**
   - All milestone reports archived
   - Easy to understand development progression
   - Bug fixes documented for future reference

3. **Clear Purpose**
   - Root = "What do I need NOW?"
   - History = "How did we get here?"

4. **Better Maintenance**
   - Active docs updated regularly
   - Historical docs remain unchanged
   - Clear separation of concerns

---

## Quick Reference

### "I want to..."

| Goal | Document |
|------|----------|
| **Get started quickly** | `README.md` |
| **Build the project** | `QUICKSTART.md` |
| **See current status** | `STATUS.md` |
| **Understand integration** | `INTEGRATION_NOTES.md` |
| **Use headless mode** | `HEADLESS_MODE_INTEGRATION.md` |
| **Verify MCTS is working** | `VERIFICATION_GUIDE.md` |
| **Understand the goals** | `PROJECT_CHARTER.md` |
| **See how we got here** | `docs/history/INDEX.md` |
| **Understand a bug fix** | `docs/history/BUGFIX_SWAPPLAYERS.md` |
| **Read phase reports** | `docs/history/PHASE_*.md` |

---

## File Counts

- **Root**: 7 active markdown files
- **History**: 15 historical markdown files
- **Technical Design**: 7 files in `docs/` (architecture, roadmap, etc.)
- **Total**: 29 markdown documentation files

---

## Migration Summary

**Moved to `docs/history/`**:
```
✅ PHASE_0_COMPLETE.md
✅ PHASE_0.2_SUMMARY.md
✅ PHASE_0.3_SUMMARY.md
✅ PHASE_1.1b_INTEGRATION_SKELETON.md
✅ PHASE_1.1b_IMPLEMENTATION_STATUS.md
✅ PHASE_1.1b_COMPILATION_SUCCESS.md
✅ PHASE_1.1b_COMPLETE.md
✅ BUILD_SUCCESS_FINAL.md
✅ UI_INTEGRATION_COMPLETE.md
✅ IMPLEMENTATION_SUMMARY.md
✅ BUGFIX_SWAPPLAYERS.md
✅ AI_INTEGRATION_OPTIONS.md
✅ SPECIFICATION_GAPS.md
✅ MANUAL_TEST_GUIDE.md
```

**Kept in root** (active/reference):
```
✅ README.md
✅ STATUS.md
✅ PROJECT_CHARTER.md
✅ QUICKSTART.md
✅ INTEGRATION_NOTES.md
✅ HEADLESS_MODE_INTEGRATION.md
✅ VERIFICATION_GUIDE.md
```

---

## Next Steps

1. **For Development**: Use root docs (STATUS.md, INTEGRATION_NOTES.md)
2. **For Bug Reference**: Check `docs/history/BUGFIX_*.md`
3. **For Phase History**: Check `docs/history/PHASE_*.md`
4. **For Complete Archive**: See `docs/history/INDEX.md`

---

## Notes

- All historical docs remain accessible via `docs/history/`
- INDEX.md provides complete navigation
- README.md updated with new documentation structure
- No documents were deleted, only reorganized

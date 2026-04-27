# Release Workflow

## Creating a Release

1. Verify all tests pass locally: `cd build && ctest --verbose`
2. Update version in `CMakeLists.txt`: `project(cpp-http-server VERSION X.Y.Z)`
3. Update Vikunja: move tasks from `[Unreleased]` to `[X.Y.Z] - YYYY-MM-DD`
4. Commit: `git commit -m "Release vX.Y.Z: description"`
5. Create annotated tag: `git tag -a vX.Y.Z -m "Release vX.Y.Z: description"`
6. Push: `git push origin feature/vX.Y.Z && git push origin vX.Y.Z`

---

## Tags — Quick Reference

```bash
# Annotated tag (recommended)
git tag -a v0.2.0 -m "Middleware и рефакторинг роутинга"

# Push single tag
git push origin v0.2.0

# Push all tags
git push origin --tags

# List tags
git tag

# Delete local tag
git tag -d v0.2.0

# Delete remote tag
git push origin --delete v0.2.0

# Tag specific commit
git tag -a v0.2.0 abc1234 -m "Сообщение"

# Show tag info
git show v0.2.0
```

---

## Version Numbering

Follow semver: `MAJOR.MINOR.PATCH`

- **MAJOR** — breaking changes (incompatible API)
- **MINOR** — new functionality (backward compatible)
- **PATCH** — bug fixes (backward compatible)

---

## After Release

1. Update consumer project (trading-platform): update `GIT_TAG` to new version
2. Verify trading-platform builds with new version
3. Run integration tests
4. Update Vikunja task status for release-related tasks
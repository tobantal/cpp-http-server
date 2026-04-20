# Release Workflow

## Создание релиза

1. Убедиться, что все тесты проходят локально: `cd build && ctest --verbose`
2. Обновить версию в `CMakeLists.txt`: `project(cpp-http-server VERSION X.Y.Z)`
3. Обновить `CHANGELOG.md`: перенести секцию `[Unreleased]` в `[X.Y.Z] - YYYY-MM-DD`
4. Закоммитить: `git commit -m "Release vX.Y.Z: описание"`
5. Создать аннотированный тег: `git tag -a vX.Y.Z -m "Release vX.Y.Z: описание"`
6. Запушить: `git push origin feature/vX.Y.Z && git push origin vX.Y.Z`

## Теги — шпаргалка

```bash
# Аннотированный тег (рекомендуется)
git tag -a v0.2.0 -m "Middleware и рефакторинг роутинга"

# Отправить один тег
git push origin v0.2.0

# Отправить все теги
git push origin --tags

# Список тегов
git tag

# Удалить локальный тег
git tag -d v0.2.0

# Удалить тег на сервере
git push origin --delete v0.2.0

# Тег для конкретного коммита
git tag -a v0.2.0 abc1234 -m "Сообщение"

# Информация о теге
git show v0.2.0
```
# Git теги

## Создать тег

### Лёгкий тег (просто метка)
```bash
git tag v0.2.0
```

### Аннотированный тег (с сообщением) — рекомендуется
```bash
git tag -a v0.2.0 -m "Middleware и рефакторинг роутинга"
```

Отправить тег на сервер
```bash
# Один тег
git push origin v0.2.0
```

### Все теги
```bash
git push origin --tags
```

Полезные команды
```bash
# Список тегов
git tag
```

### Удалить локальный тег
```bash
git tag -d v0.2.0
```

### Удалить тег на сервере
```bash
git push origin --delete v0.2.0
```

### Создать тег для конкретного коммита
```bash
git tag -a v0.2.0 abc1234 -m "Сообщение"
```

### Посмотреть информацию о теге
```bash
git show v0.2.0
```

Типичный workflow для релиза
```bash
git add .
git commit -m "Release v0.2.0: Middleware и рефакторинг роутинга"
git tag -a v0.2.0 -m "Release v0.2.0"
git push origin main
git push origin v0.2.0
```


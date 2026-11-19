# Сборка

```bash
cmake -E remove_directory './build'
cmake -S './' -B './build' -G Ninja
cmake --build './build'
```

# Запуск

```bash
./build/c166 <Файл Intel Hex>
```

# Готовые примеры

В папке `examples/` содержаться скомпилированные сценарии.
 - `source.c` - исходный код
 - `listing.m66` - листинг компиляции. Здесь можно увидеть по каким адресам расположены функции.
 - `hex.H86` - файл прошивки в формате Inltel Hex.
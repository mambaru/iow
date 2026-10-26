# examples

Минимальные примеры на iow без лишних шаблонов.

## chat_server / chat_client

TCP-чат: строки с разделителем `\n`, broadcast всем клиентам.

```bash
cmake -DWITH_SAMPLES=ON ..
make chat_server chat_client

./bin/utils/chat_server 30000
./bin/utils/chat_client 127.0.0.1 30000
```

Описание — в [docs/iow.md](../docs/iow.md).

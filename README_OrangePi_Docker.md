# Orange Pi Recorder - Dockerización

Este repositorio contiene exclusivamente la **dockerización del sistema de grabación** para Orange Pi (ARM64).

## Notas importantes

- Este proyecto solo gestiona la grabación.
- Los binarios `spectrogram` y `dht22.out` ya están compilados para ARM64.
- **No se compilan dentro de Docker.**
- El contenedor únicamente ejecuta los binarios y gestiona la persistencia de datos.

---

# Pasos de instalación en la Orange Pi

## 1️⃣ Copiar la carpeta a la Orange Pi

Desde tu ordenador:

```bash
scp -r "orangepi-docker" orangepi@IP:/home/orangepi/
```

---

## 2️⃣ Instalar Docker + Docker Compose

En la Orange Pi:

```bash
sudo apt-get update
sudo apt-get install -y docker.io docker-compose-plugin
sudo usermod -aG docker $USER
newgrp docker
```

Verificar instalación:

```bash
docker --version
docker compose version
```

---

## 3️⃣ Dar permisos de ejecución

Desde la raíz del proyecto:

```bash
chmod +x recorder/entrypoint.sh recorder/record.sh
chmod +x spectrogram/spectrogram
chmod +x DHT22/dht22.out
```

---

## 4️⃣ Construir y levantar el contenedor

```bash
docker compose build --no-cache recorder
docker compose up -d
```

---

## 5️⃣ Comprobaciones

### 🔹 Ver estado del contenedor

```bash
docker ps -a --filter name=bird-recorder
```

### 🔹 Ver logs en tiempo real

```bash
docker logs -f bird-recorder
```

### 🔹 Verificar que se están creando archivos

```bash
docker exec -it bird-recorder ls -la /data
docker exec -it bird-recorder ls -la /data/recordings | head
docker exec -it bird-recorder tail -n 20 /data/stats.txt
```

---

# ✅ Estado correcto

Si:

- El contenedor aparece como `Up`
- Existen archivos `.wav` en `/data/recordings`
- `stats.txt` contiene datos

Entonces el sistema está funcionando correctamente.

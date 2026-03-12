#include <stdio.h>
#include <unistd.h>
#include <gpiod.h>
#include <string.h>
#include <stdlib.h>

#define GPIO_CHIP "/dev/gpiochip0"
#define DHT_LINE 6  // cambia según el pin real (ej. PA6 → línea 6)

static int read_dht22(int *humidity, int *temperature)
{
    struct gpiod_chip *chip;
    struct gpiod_line *line;
    unsigned char data[5] = {0};
    int i = 0, j = 0;

    chip = gpiod_chip_open(GPIO_CHIP);
    if (!chip) return -1;

    line = gpiod_chip_get_line(chip, DHT_LINE);
    if (!line) {
        gpiod_chip_close(chip);
        return -1;
    }

    // Enviar señal de inicio
    gpiod_line_request_output(line, "dht22", 0);
    usleep(18000);
    gpiod_line_set_value(line, 1);
    usleep(40);
    gpiod_line_release(line);

    // Leer respuesta
    gpiod_line_request_input(line, "dht22");

    // Esperar respuesta del sensor
    while (gpiod_line_get_value(line) == 1) usleep(1);
    while (gpiod_line_get_value(line) == 0) usleep(1);
    while (gpiod_line_get_value(line) == 1) usleep(1);

    // Leer 40 bits (5 bytes)
    for (i = 0; i < 40; i++) {
        while (gpiod_line_get_value(line) == 0) usleep(1);
        usleep(30);
        if (gpiod_line_get_value(line) == 1)
            data[i / 8] |= (1 << (7 - (i % 8)));
        while (gpiod_line_get_value(line) == 1) usleep(1);
    }

    gpiod_line_release(line);
    gpiod_chip_close(chip);

    // Calcular checksum
    if ((unsigned char)(data[0] + data[1] + data[2] + data[3]) != data[4])
        return -2;

    *humidity = ((data[0] << 8) + data[1]);
    *temperature = ((data[2] & 0x7F) << 8) + data[3];
    if (data[2] & 0x80)
        *temperature *= -1;
    return 0;
}

int main(void)
{
    int hum = 0, temp = 0;
    int res = read_dht22(&hum, &temp);
    if (res == 0)
        printf("Temperature: %.1f°C, Humidity: %.1f%%\n", temp / 10.0, hum / 10.0);
    else
        fprintf(stderr, "Error reading DHT22 (%d)\n", res);
    return 0;
}

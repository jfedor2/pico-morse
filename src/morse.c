#include <stdlib.h>
#include <string.h>

#include "bsp/board.h"
#include "tusb.h"

#include "hardware/gpio.h"
#include "pico/stdlib.h"

#define PIN 14

typedef struct __attribute__((packed)) {
    uint8_t mods;
    uint8_t key;
} hid_report_t;

hid_report_t report_queue[8];

uint8_t head = 0;
uint8_t tail = 0;
uint8_t items = 0;

uint8_t code[][5] = {
    { 0b10, 2, 'A', 0x04, 0x00 },
    { 0b0111, 4, 'B', 0x05, 0x00 },
    { 0b0101, 4, 'C', 0x06, 0x00 },
    { 0b011, 3, 'D', 0x07, 0x00 },
    { 0b1, 1, 'E', 0x08, 0x00 },
    { 0b1101, 4, 'F', 0x09, 0x00 },
    { 0b001, 3, 'G', 0x0A, 0x00 },
    { 0b1111, 4, 'H', 0x0B, 0x00 },
    { 0b11, 2, 'I', 0x0C, 0x00 },
    { 0b1000, 4, 'J', 0x0D, 0x00 },
    { 0b010, 3, 'K', 0x0E, 0x00 },
    { 0b1011, 4, 'L', 0x0F, 0x00 },
    { 0b00, 2, 'M', 0x10, 0x00 },
    { 0b01, 2, 'N', 0x11, 0x00 },
    { 0b000, 3, 'O', 0x12, 0x00 },
    { 0b1001, 4, 'P', 0x13, 0x00 },
    { 0b0010, 4, 'Q', 0x14, 0x00 },
    { 0b101, 3, 'R', 0x15, 0x00 },
    { 0b111, 3, 'S', 0x16, 0x00 },
    { 0b0, 1, 'T', 0x17, 0x00 },
    { 0b110, 3, 'U', 0x18, 0x00 },
    { 0b1110, 4, 'V', 0x19, 0x00 },
    { 0b100, 3, 'W', 0x1A, 0x00 },
    { 0b0110, 4, 'X', 0x1B, 0x00 },
    { 0b0100, 4, 'Y', 0x1C, 0x00 },
    { 0b0011, 4, 'Z', 0x1D, 0x00 },
    { 0b10000, 5, '1', 0x1E, 0x00 },
    { 0b11000, 5, '2', 0x1F, 0x00 },
    { 0b11100, 5, '3', 0x20, 0x00 },
    { 0b11110, 5, '4', 0x21, 0x00 },
    { 0b11111, 5, '5', 0x22, 0x00 },
    { 0b01111, 5, '6', 0x23, 0x00 },
    { 0b00111, 5, '7', 0x24, 0x00 },
    { 0b00011, 5, '8', 0x25, 0x00 },
    { 0b00001, 5, '9', 0x26, 0x00 },
    { 0b00000, 5, '0', 0x27, 0x00 },
    { 0b101010, 6, '.', 0x37, 0x00 },
    { 0b001100, 6, ',', 0x36, 0x00 },
    { 0b000111, 6, ':', 0x33, 0x02 },
    { 0b110011, 6, '?', 0x38, 0x02 },
    { 0b100001, 6, '\'', 0x34, 0x00 },
    { 0b011110, 6, '-', 0x2D, 0x00 },
    { 0b01101, 5, '/', 0x38, 0x00 },
    { 0b01001, 5, '(', 0x26, 0x02 },
    { 0b010010, 6, ')', 0x27, 0x02 },
    { 0b101101, 6, '"', 0x34, 0x02 },
    { 0b01110, 5, '=', 0x2E, 0x00 },
};

void enqueue(uint8_t key, uint8_t mods) {
    if (items == sizeof(report_queue)) {
        printf("overflow!\n");
        return;
    }
    report_queue[tail].key = key;
    report_queue[tail].mods = mods;
    tail = (tail + 1) % sizeof(report_queue);
    items++;
}

void emit_letter(uint8_t dits, uint8_t len) {
    for (int i = 0; i < sizeof(code) / sizeof(code[0]); i++) {
        if (dits == code[i][0] && len == code[i][1]) {
            printf("%c", code[i][2]);
            enqueue(code[i][3], code[i][4]);
            enqueue(0x00, 0x00);
            break;
        }
    }
}

void emit_space() {
    enqueue(0x2C, 0x00);
    enqueue(0x00, 0x00);
}

int main(void) {
    board_init();
    tusb_init();
    stdio_init_all();

    gpio_init(PIN);
    gpio_pull_up(PIN);

    uint64_t unit_time = 75000;

    bool prev_state = false;
    uint64_t prev_time = time_us_64();

    uint8_t n = 0;
    uint8_t partial = 0;

    while (true) {
        tud_task();

        uint64_t now = time_us_64();
        uint64_t len = now - prev_time;
        bool state = !gpio_get(PIN);
        if (prev_state != state) {
            if (prev_state) {
                if (len > 10000) {
                    if (len < unit_time * 2) {
                        // dit
                        printf(".");
                        partial <<= 1;
                        partial |= 1;
                        n++;
                    } else {
                        // dah
                        printf("-");
                        partial <<= 1;
                        n++;
                    }
                }
            } else {
                if (len < unit_time * 2) {
                    // intra-letter gap
                } else if (len < unit_time * 5.33) {
                    // gap between letters
                    emit_letter(partial, n);
                    partial = 0;
                    n = 0;
                }
            }

            prev_state = state;
            prev_time = now;
        } else {
            if (!prev_state && (n > 0) && (len >= unit_time * 5.33)) {
                // gap between words
                emit_letter(partial, n);
                partial = 0;
                n = 0;
                emit_space();
                printf(" ");
            }
        }

        if (tud_hid_ready() && (items > 0)) {
            tud_hid_report(0, report_queue + head, sizeof(hid_report_t));
            head = (head + 1) % sizeof(report_queue);
            items--;
        }
    }

    return 0;
}

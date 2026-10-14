/* C99: inicializadores designados (structs e vetores) e literais compostos. */
#include <stdint.h>
#include <stdio.h>

enum uart_parity { UART_PARITY_NONE, UART_PARITY_EVEN, UART_PARITY_ODD };

struct uart_config {
    uint32_t baudrate;
    enum uart_parity parity;
    uint8_t stop_bits;
    uint8_t flow_control;   /* não citado no inicializador: fica zero */
};

enum { LED_STATUS, LED_ERROR, LED_COUNT };

typedef struct {
    uint8_t id;
    uint8_t len;
} message_t;

static void send_message(const message_t *msg)
{
    printf("send_message: id=%u len=%u\n", msg->id, msg->len);
}

int main(void)
{
    struct uart_config cfg = {
        .baudrate = 115200,
        .parity = UART_PARITY_NONE,
        .stop_bits = 1
    };

    static const uint8_t led_pin[LED_COUNT] = {
        [LED_STATUS] = 13,
        [LED_ERROR]  = 12,
    };

    printf("cfg: baudrate=%lu parity=%d stop_bits=%u flow_control=%u\n",
           (unsigned long)cfg.baudrate, (int)cfg.parity, cfg.stop_bits, cfg.flow_control);
    printf("led_pin: [LED_STATUS]=%u [LED_ERROR]=%u\n", led_pin[LED_STATUS], led_pin[LED_ERROR]);

    send_message(&(message_t){
        .id = 1,
        .len = 8
    });

    return 0;
}

/* O que o pré-processador enxerga: #error para configuração obrigatória e o #warning, que o
 * C23 padronizou. Compile sem -DUART_BAUD para ver o #error. */
#ifndef UART_BAUD
#error "defina UART_BAUD (por exemplo, -DUART_BAUD=115200)"
#endif

#if UART_BAUD > 921600
#warning "UART_BAUD acima de 921600: confira o clock do periférico"
#endif

unsigned long baud = UART_BAUD;

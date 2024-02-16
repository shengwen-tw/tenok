#ifndef __UART_H__
#define __UART_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <kernel/kernel.h>
#include <kernel/kfifo.h>
#include <kernel/mutex.h>
#include <kernel/wait.h>

#include "stm32f4xx.h"

typedef struct {
    /* Tx */
    wait_queue_head_t tx_wait_list;
    struct mutex tx_mtx;
    bool tx_ready;

    /* Rx */
    wait_queue_head_t rx_wait_list;
    struct kfifo *rx_fifo;
    struct mutex rx_mtx;
    size_t rx_wait_size;
} uart_dev_t;

void uart_putc(USART_TypeDef *uart, char c);
char uart_getc(USART_TypeDef *uart);
int uart_puts(USART_TypeDef *uart, const char *data, size_t size);

void uart1_init(char *dev_name, char *description);
void uart2_init(char *dev_name, char *description);
void uart3_init(char *dev_name, char *description);

#endif
/*************************************************************************
	> File Name: app_uart.h
	> Author: 
	> Mail: 
	> Created Time: 2019年09月08日 星期日 05时22分40秒
 ************************************************************************/

#ifndef _APP_UART_H
#define _APP_UART_H

void app_uart_init(void);
void app_uart_send_data(char *para, int len);
void app_uart_run(void);

#endif

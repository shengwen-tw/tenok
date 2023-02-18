#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "stm32f4xx_gpio.h"
#include "gpio.h"
#include "uart.h"
#include "kernel.h"
#include "syscall.h"
#include "semaphore.h"
#include "shell.h"
#include "shell_cmd.h"
#include "fs.h"
#include "mutex.h"
#include "rom_dev.h"

sem_t sem_led;
_pthread_mutex_t mutex_print;
mqd_t mqdes_print;

/* shell */
struct cmd_list_entry shell_cmd_list[] = {
	DEF_SHELL_CMD(help),
	DEF_SHELL_CMD(clear),
	DEF_SHELL_CMD(history),
	DEF_SHELL_CMD(ps),
	DEF_SHELL_CMD(echo),
	DEF_SHELL_CMD(ls),
	DEF_SHELL_CMD(cd),
	DEF_SHELL_CMD(pwd),
	DEF_SHELL_CMD(cat)
};

struct shell_struct shell;


void led_task1(void)
{
	set_program_name("led1");
	setpriority(0, getpid(), 3);

	int state = 1;
	while(1) {
		sem_wait(&sem_led);

		GPIO_WriteBit(GPIOD, GPIO_Pin_12, state);
		GPIO_WriteBit(GPIOD, GPIO_Pin_13, state);

		state = (state + 1) % 2;
	}
}

void led_task2(void)
{
	set_program_name("led2");
	setpriority(0, getpid(), 3);

	int state = 1;
	while(1) {
		sem_post(&sem_led);

		volatile int pid = getpid();

		GPIO_WriteBit(GPIOD, GPIO_Pin_14, state);
		GPIO_WriteBit(GPIOD, GPIO_Pin_15, state);

		sleep(1000);

		state = (state + 1) % 2;
	}
}

void shell_task(void)
{
	set_program_name("shell");
	setpriority(0, getpid(), 2);

	/* shell initialization */
	char ret_shell_cmd[CMD_LEN_MAX];
	char path_curr[200] = {0};
	char prompt_msg[PROMPT_LEN_MAX] = {0};
	int  shell_cmd_cnt = SIZE_OF_SHELL_CMD_LIST(shell_cmd_list);

	shell_init_struct(&shell, prompt_msg, ret_shell_cmd);
	shell_path_init();

	/* clean screen */
	shell_cls();

	/* greeting */
	char s[100] = {0};
	sprintf(s, "firmware build time: %s %s\n\rtype `help' for help\n\r\n\r", __TIME__, __DATE__);
	shell_puts(s);

	while(1) {
		shell_get_pwd(path_curr);
		snprintf(prompt_msg, PROMPT_LEN_MAX, __USER_NAME__ "@stm32f407:%s$ ",  path_curr);
		shell.prompt_len = strlen(shell.prompt_msg);

		shell_cli(&shell);
		shell_cmd_exec(&shell, shell_cmd_list, shell_cmd_cnt);
	}
}

void fifo_task1(void)
{
	set_program_name("fifo1");

	int fd = open("/fifo_test", 0, 0);
	char data[] = "hello";
	int len = strlen(data);

	while(1) {
		int i;
		for(i = 0; i < len; i++) {
			write(fd, &data[i], 1);
			sleep(200);
		}
	}
}

void fifo_task2(void)
{
	set_program_name("fifo2");

	int fd = open("/fifo_test", 0, 0);
	char data[10] = {0};
	char str[20];

	while(1) {
		read(fd, &data, 5);
		sprintf(str, "received: %s\n\r", data);
		uart3_puts(str);
	}
}

void mutex_task1(void)
{
	set_program_name("mutex1");

	char *str = "mutex task 1\n\r";

	while(1) {
		pthread_mutex_lock(&mutex_print);
		uart3_puts(str);
		sleep(100);
		pthread_mutex_unlock(&mutex_print);
	}
}

void mutex_task2(void)
{
	set_program_name("mutex2");

	char *str = "mutex task 2\n\r";

	while(1) {
		pthread_mutex_lock(&mutex_print);
		uart3_puts(str);
		sleep(100);
		pthread_mutex_unlock(&mutex_print);
	}
}

/* define your own customized message type */
typedef struct {
	char data[20];
} my_message_t;

void message_queue_task1(void)
{
	set_program_name("queue1");

	my_message_t msg;

	char *str = "hello world!\n\r";
	strcpy(msg.data, str);

	struct mq_attr attr = {
		.mq_flags = 0,
		.mq_maxmsg = 100,
		.mq_msgsize = sizeof(my_message_t),
		.mq_curmsgs = 0
	};
	mqdes_print = mq_open("/my_message", 0, &attr);

	while(1) {
		mq_send(mqdes_print, (char *)&msg, 1, 0);
		sleep(1000);
	}
}

void message_queue_task2(void)
{
	set_program_name("queue2");

	my_message_t msg;

	while(1) {
		mq_receive(mqdes_print, (char *)&msg, 1, 0);
		uart3_puts(msg.data);
	}
}

void mk_cpuinfo(void)
{
	/* create a new regular file */
	mknod("/proc/cpuinfo", 0, S_IFREG);
	int fd = open("/proc/cpuinfo", 0, 0);

	char *str = "processor  : 0\n\r"
	            "model name : STM32F407 Microcontroller\n\r"
	            "cpu MHz    : 168\n\r"
	            "flash size : 1M\n\r"
	            "ram size   : 192K\n\r";

	/* write file content */
	int len = strlen(str);
	write(fd, str, len);

	/* write end-of-file */
	signed char c = EOF;
	write(fd, &c, 1);
}

void first(void)
{
	set_program_name("first");

	sem_init(&sem_led, 0, 0);
	mknod("/fifo_test", 0, S_IFIFO);
	pthread_mutex_init(&mutex_print, 0);

	mk_cpuinfo();

	mount("/dev/rom", "/");

	if(!fork()) led_task1();
	if(!fork()) led_task2();
	if(!fork()) shell_task();
	//if(!fork()) fifo_task1();
	//if(!fork()) fifo_task2();
	//if(!fork()) mutex_task1();
	//if(!fork()) mutex_task2();
	//if(!fork()) message_queue_task1();
	//if(!fork()) message_queue_task2();

	//idle loop if no work to do
	while(1);
}

void init(void *param)
{
	rootfs_init();

	rom_dev_init();

	if(!fork()) file_system();

	first();
}

int main(void)
{
	led_init();
	uart3_init(115200);

	sched_start(init);

	return 0;
}

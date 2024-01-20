/*
 * main.c
 *
 *  Created on: 20 Jan 2024
 *      Author: Efe Tunca
 */

#include <stdio.h>
#include "xparameters.h"
#include "platform.h"
#include "platform_config.h"
#include "lwipopts.h"
#include "xil_printf.h"

#include "netif/xadapter.h"
#include "lwip/priv/tcp_priv.h"
#include "lwip/init.h"
#include "lwip/inet.h"

#include "tftp_server.h"
#include "web_utils.h"

void tcp_fasttmr(void);
void tcp_slowtmr(void);

extern volatile int TcpFastTmrFlag;
extern volatile int TcpSlowTmrFlag;
struct netif server_netif;
TCHAR *Path = "0:";

int main()
{
	/* The MAC address of the board */
	u8 ethernetMACAddress[] = { 0x00, 0x0a, 0x35, 0x00, 0x01, 0x02 };

	/* The '\033\143' sequence resets the terminal window */
	xil_printf("\033\143---------- TFTP Server ----------\r\n\r\n");

	init_platform();
	xil_printf("Platform initialized...\r\n");

	lwip_init();
	xil_printf("lwIP initialized...\r\n");

	/*
	 * If the SD card is to be formatted, then the 2nd parameter
	 * of this function must be set to 1, otherwise set to 0.
	 */
	initFileSystem(Path, 0);
	xil_printf("File system initialized...\r\n\n");

	if(!xemac_add(&server_netif, NULL, NULL, NULL,
			ethernetMACAddress, PLATFORM_EMAC_BASEADDR)) {
		xil_printf("Error adding network interface\r\n");
		return -1;
	}

	netif_set_default(&server_netif);

	/* enabling interrupts */
	platform_enable_interrupts();

	/* specifying the network if it is up */
	netif_set_up(&server_netif);

	assignDefaultIP(&(server_netif.ip_addr), &(server_netif.netmask), &(server_netif.gw));
	printIPSettings(&server_netif.ip_addr, &server_netif.netmask, &server_netif.gw);

	/* printing application header */
	printAppHeader();

	/* starting the application */
	startApplication();

	/* listing directory items */
	listDirectory(Path);

	/* creating the index.html file with file tree in it */
	createIndexFileTree(Path);

	/* receiving and process packages */
	while (1) {
		if (TcpFastTmrFlag) {
			tcp_fasttmr();
			TcpFastTmrFlag = 0;
		}
		if (TcpSlowTmrFlag) {
			tcp_slowtmr();
			TcpSlowTmrFlag = 0;
		}
		xemacif_input(&server_netif);
	}

	/* program never reaches here */
    cleanup_platform();
    return 0;
}

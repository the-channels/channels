/* Test the libspectranet library. */

#include <stdio.h>
#include <spectranet.h>

extern char fixedram @ 0x3000;
extern char pagebram @ 0x2000;
extern char pagearam @ 0x1000;

void main()
{
	in_addr_t ip, netmask, gw;
	char ipbuf[19]; /* big enough for mac addr */
	char mac[6];
	int rc;

	/* cls */
	putchar(0x0c);

	printk("libspectranet test program\nIP settings:\n");
	
	get_ifconfig_inet(&ip);
	get_ifconfig_netmask(&netmask);
	get_ifconfig_gw(&gw);
	
	long2ipstring(&ip, ipbuf);
	printk("IP address:     %s\n", ipbuf);
	long2ipstring(&netmask, ipbuf);
	printk("Netmask   :     %s\n", ipbuf);
	long2ipstring(&gw, ipbuf);
	printk("Gateway   :     %s\n\n", ipbuf);

	printk("Hardware settings:\n");
	
	gethwaddr(mac);
	mac2string(mac, ipbuf);
	printk("MAC addr  :     %s\n\n", ipbuf);

	/* Test the memory paging functions */
	printk("Paging in Spectranet memory.\n");
#asm
	call 0x3ff9
#endasm

	printk("Writing to fixed RAM page\n");
	sprintf(&fixedram, "Successfully wrote and read at 0x3000\n");
	printk("%s\n", &fixedram);

	printk("Writing to RAM in page B and reading back from page A\n");
	setpageb(0xC0);		/* use page C0 */
	sprintf(&pagebram, "Successfully wrote to page A, read back from B\n");
	setpagea(0xC0);
	printk("%s\n", &pagearam);

#asm
	call 0x007c
#endasm	

	printk("Testing ipstring2long...\nTrying with '1.2.3.4': ");
	rc=ipstring2long("1.2.3.4", &ip);
	long2ipstring(&ip, ipbuf);
	printk("rc=%d, %s\n", rc, ipbuf);
	
	printk("Trying with '32f09f09uf': ");
	rc=ipstring2long("32f09f09uf", &ip);
	printk("rc=%d\n", rc);
}


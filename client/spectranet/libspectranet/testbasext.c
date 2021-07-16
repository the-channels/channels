#include <stdio.h>
#include <spectranet.h>
#include <basicext.h>

char *token="*foo";
char *p42="*print42";

void foocmd();
void printcmd();

main()
{
	struct basic_cmd bc;

	bc.errorcode=0x0b;	// nonsense in basic
	bc.command=token;
	bc.rompage=0;		// don't do paging
	bc.function=foocmd;

	if(addbasicext(&bc) < 0)
	{
		printk("Failed to add extension\n");
		return;
	}
	printk("Added basic extension.\n");

	bc.errorcode=0x0b;
	bc.command=p42;
	bc.rompage=0;
	bc.function=printcmd;
	if(addbasicext(&bc) < 0)
	{
		printk("Failed to add extension\n");
		return;
	}
	printk("Added print42 extension\n");

}

void foocmd()
{
	statement_end();
	printk("Statement executed.\n");
#asm
	jp 0x3E99
#endasm
}

void printcmd()
{
	char buf[32];
	int bytes;

	expectStringExp();
	statement_end();
	
	bytes=string_fetch(buf, 31);
	printk("String length: %d\nString value : %s\n", bytes, buf);
#asm
	jp 0x3E99
#endasm
}

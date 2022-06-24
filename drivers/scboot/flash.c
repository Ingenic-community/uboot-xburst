#include <common.h>
#include <asm/io.h>
#include <asm/arch/cpm.h>
#include <asm/reboot.h>
#include <asm/spl.h>

void read_flash(unsigned int from, unsigned int len, unsigned char *buf)
{
	u32 boot_device;

	boot_device = spl_boot_device();

	switch(boot_device) {

#ifdef CONFIG_JZ_SFC_NOR
	case BOOT_DEVICE_SFC_NOR:
		sfc_nor_read(from, len, buf);
		break;
#endif
	default:
		printf("## ERROR ## Ckey aes only support sfc_nor ##\n");
		hang();
	}
}
void write_flash(unsigned int from, unsigned int len, unsigned char *buf)
{
	u32 boot_device;

	boot_device = spl_boot_device();

	switch (boot_device) {

#ifdef CONFIG_JZ_SFC_NOR
	case BOOT_DEVICE_SFC_NOR:

		if (sfc_nor_erase(from, len)) {
			printf("sfcnor erase err!\n");
			_machine_restart();
		}

		sfc_nor_write(from, len, buf);
		break;
#endif
	default:
		printf("## ERROR ## Ckey aes only support sfc_nor ##\n");
		hang();
	}
}

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <mvm/bootimage.h>

static void dump_bootimg_hdr(boot_img_hdr *hdr)
{
	char buf[1024];

	memset(buf, 0, 1024);
	strncpy(buf, (char *)hdr->magic, BOOT_MAGIC_SIZE);

	printf("magic        - %s", buf);
	printf("kernel_size  - 0x%x\n", hdr->kernel_size);
	printf("kernel_addr  - 0x%x\n", hdr->kernel_addr);
	printf("ramdisk_size - 0x%x\n", hdr->ramdisk_size);
	printf("ramdisk_addr - 0x%x\n", hdr->ramdisk_addr);
	printf("dtb_size     - 0x%x\n", hdr->second_size);
	printf("dtb_addr     - 0x%x\n", hdr->second_addr);
	printf("tags_addr    - 0x%x\n", hdr->tags_addr);
	printf("page_size    - 0x%x\n", hdr->page_size);

	strncpy(buf, (char *)hdr->name, BOOT_NAME_SIZE);
	printf("name         - %s\n", buf);
	strncpy(buf, (char *)hdr->cmdline, BOOT_ARGS_SIZE);
	printf("cmdline      - %s\n", buf);
}

int read_bootimage_header(int fd, boot_img_hdr *hdr)
{
	int ret;
	struct stat stbuf;
	unsigned long page_size;
	unsigned long k, r, o, t;

	ret = read(fd, hdr, sizeof(boot_img_hdr));
	if (ret != sizeof(boot_img_hdr))
		return -EIO;

	/* here check the header */
	if (strncmp((char *)hdr->magic, BOOT_MAGIC, BOOT_MAGIC_SIZE) != 0)
		return 1;

	if (!hdr->kernel_size)
		return 1;

	if (!hdr->page_size)
		return 1;

	page_size = hdr->page_size;
	k = (hdr->kernel_size + page_size - 1) / page_size;
	r = (hdr->ramdisk_size + page_size - 1) / page_size;
	o = (hdr->second_size + page_size - 1) / page_size;
	t = (1 + k + r + o) * page_size;

	if ((fstat(fd, &stbuf) != 0) || (!S_ISREG(stbuf.st_mode)))
		return 1;

	if (t > stbuf.st_size)
		return -EINVAL;

	dump_bootimg_hdr(hdr);

	return 0;
}

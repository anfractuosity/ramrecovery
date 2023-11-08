#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/kallsyms.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/io.h>

#define CONTIGUOUS_START 0x2400000
#define IMAGE_SIZE 725444U

MODULE_LICENSE("GPL");

static int writetoram = 0;	// Whether to write Mona to RAM, or try to read Mona from RAM
module_param(writetoram, int, 0660);

static char *kbuf = NULL;
static dma_addr_t handle;
static size_t size = 1024 * 1024 * 650;	// Contiguous RAM to allocate
void devrelease(struct device *dev);

static struct platform_device dev = {
	.name = "ramrecovery",
	.id = 0,
};

void devrelease(struct device *dev)
{
	if (kbuf)
		dma_free_coherent(dev, size, kbuf, handle);
	printk(KERN_INFO "Ramrec, free\n");
}

static int __init ramrecovery_init(void)
{
	struct file *fp;	// File pointer, to Mona Lisa image, or memory dump binary
	int image;		// Image number
	int imagebyte;		// Index of nth byte of Mona Lisa image
	char *mona;		// Holds Mona Lisa
	int ret;

	if (writetoram) {
		// Try to fill contiguous RAM with Mona
		ret = platform_device_register(&dev);

		if (ret) {
			printk("Unable to register device\n");
			return -1;
		}

		dev.dev.release = devrelease;

		ret = dma_set_mask_and_coherent(&dev.dev, DMA_BIT_MASK(64));
		if (ret) {
			printk(KERN_ERR "dma_set_mask returned: %d\n", ret);
			goto err;
		}
		// Allocate lots of contiguous RAM
		kbuf = dma_alloc_coherent(&dev.dev, size, &handle, GFP_KERNEL);
		if (!kbuf) {
			printk(KERN_ERR "dma_alloc_coherent failed\n");
			goto err;
		}
		// Open Mona file
		fp = filp_open("./mona.tga", O_RDONLY, 0);
		if (IS_ERR(fp)) {
			printk(KERN_ERR "Loading Mona failed\n");
			goto err;
		}
		// Read Mona to buffer
		mona = kmalloc(IMAGE_SIZE, GFP_KERNEL);
		ret = kernel_read(fp, mona, IMAGE_SIZE, NULL);

		// Fill contiguous RAM with Mona
		for (image = 0; image < size; image += IMAGE_SIZE) {
			printk("Image %d\n", image);
			if (image + IMAGE_SIZE >= size)
				break;
			for (imagebyte = 0; imagebyte < IMAGE_SIZE; imagebyte++) {
				kbuf[image + imagebyte] = mona[imagebyte];
			}
		}
	} else {
		// Try to read from same amount of contiguous RAM, as we allocated previously
		kbuf = memremap(CONTIGUOUS_START, size, MEMREMAP_WB);

		if (!kbuf) {
			printk("Memory mapping failed\n");
			return -1;
		}
		// Dump memory to a file
		fp = filp_open("./out.bin", O_RDWR | O_CREAT, 0644);
		ret = kernel_write(fp, kbuf, size, 0);
		iounmap(kbuf);
	}

	return 0;

 err:
	// Something bad happened
	platform_device_unregister(&dev);
	return -1;
}

static void __exit ramrecovery_exit(void)
{
	if (writetoram)
		platform_device_unregister(&dev);
	printk(KERN_INFO "Ramrec, fin.\n");
}

module_init(ramrecovery_init);
module_exit(ramrecovery_exit);

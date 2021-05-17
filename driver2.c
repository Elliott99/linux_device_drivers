#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/irq.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/poll.h>
#include <linux/cdev.h>
#include "driver2.h"
#include <linux/string.h>

//a simple driver
//when you write to it, it writes in kernel space what you sent to it
//when you read from it, it prints out a message, with a count of how many times output was read from the device

#define DEVICE_NAME "Edev_Mk_2"
#define SUCCESS 0
#define FAILURE -1
#define BUF_LEN 1024

static struct class * cls;

static int major_num;
static int counter=0;
static char message_buffer[BUF_LEN];
static char * message_pointer;
static char * write_pointer;
static char write_buffer[BUF_LEN];

//init_module: register dynamic major #
//create class that device belongs to
//create device that 
//create class 

static struct file_operations elliott_fops = {
	.read=device_read,
	.write=device_write,
	.owner=THIS_MODULE
};


int init_module(void){
	major_num=register_chrdev(0,DEVICE_NAME,&elliott_fops);
	if (major_num < 0){
		pr_alert("Major number registration failed");
		return FAILURE;
	}
	cls=class_create(THIS_MODULE,DEVICE_NAME);
	device_create(cls,NULL,MKDEV(major_num,0),NULL,DEVICE_NAME);
	pr_info("Created device in /dev/%s",DEVICE_NAME);
	return SUCCESS;
}

void cleanup_module(void){
	device_destroy(cls,MKDEV(major_num,0));
	pr_info("Removed device from module");
	class_destroy(cls);
	unregister_chrdev(major_num,DEVICE_NAME);
}


//buffer is the buffer to be filled
//length is the length of the buffer to be filled
static ssize_t device_read(struct file * filp2, char *buffer2, size_t length2, loff_t *offset2){
	sprintf(message_buffer,"Yes,you read from the driver this many times: %d",counter++);
	ssize_t message_length=strlen(message_buffer);
	message_pointer=message_buffer;
	if(copy_to_user(buffer2,message_pointer,message_length+1)!=0){
		pr_alert("Could not successfully copy all bytes to userspace");
		return -EFAULT;
	}
	buffer2[message_length]='\0';
	pr_info("Outputted %lu bytes with %lu bytes remaining in the buffer",strlen(buffer2),length2-strlen(buffer2));
	pr_info("The data in the userspace buffer is now %s",buffer2);
	return 0;
}

static ssize_t device_write(struct file * filp, const char *buffer, size_t length, loff_t *offset){
	ssize_t bytes_read_in=0;
	ssize_t ret=length;
	write_pointer=write_buffer;
	if (copy_from_user(write_pointer,buffer,length)!=0){
		pr_alert("Failed to read all bytes from userspace");
		return -EFAULT;
	}

	write_buffer[length-1]='\0';
	while (length && *write_pointer){
		length--;
		write_pointer++;
		bytes_read_in++;
	}
	pr_info("The length value is %lu",length);
	pr_info("write pointer now points to %c",*write_pointer);
	pr_info("The device had %lu bytes written to it",bytes_read_in);
	pr_info("Got message from user: %s",write_buffer);
	*offset+=bytes_read_in;
	return ret;
}

MODULE_LICENSE("GPL");

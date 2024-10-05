#include<linux/module.h> /* for every modules */

#include <linux/init.h>

#include <linux/kernel.h>

#include <linux/fs.h> /* for alloc_chrdev_region function */

#include<linux/cdev.h> /* for cdev_init function */

#include <linux/device.h>

#include <linux/kdev_t.h>

#include<linux/uaccess.h>

#include<linux/err.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Truong Le Van");
MODULE_DESCRIPTION("A simple module to read/write a buffer");
MODULE_VERSION("1.0");



#define FIRST_MINOR_NUMBER       0
#define TOTAL_MINOR_NUMBERS      1
#define NAME_OF_DEVICE           "Buffering"   /*This is just name of the device not the device file name  */
#define BUFFER_SIZE              512


loff_t pcd_lseek (struct file * filp, loff_t off, int whence);
ssize_t pcd_read (struct file * filp, char __user *buff, size_t count, loff_t * f_pos);
ssize_t pcd_write (struct file *filp, const char __user *buff, size_t count, loff_t * f_pos);
int pcd_open (struct inode * inode, struct file * filp);
int pcd_release (struct inode * inode, struct file * filp);


char g_buffer[BUFFER_SIZE];


/** This variable holds device number
 * uint32_t :
 * 12 bits to store major number -> MAJOR(device_number) -> linux/kdev_t.h
 * 20 bits to store minor number -> MINOR(device_number) -> linux/kdev_t.h
 * device_number = MKDEV(major,minor);
 */
dev_t g_device_number;



/**
 * Initialize file ops structure with driver's system call implementation methods
 *
 */
struct file_operations g_fops =
{
    .open = pcd_open,
    .write = pcd_write,
    .read = pcd_read,
    .llseek = pcd_lseek,
    .release = pcd_release,
    .owner = THIS_MODULE,
};


struct cdev g_cdev;

struct class * gp_class;

struct device  *gp_device;

static int __init buffering_module_init(void)
{
    int status = 0 ;

    /* 1. Dynamically allocate a device number */
    status = alloc_chrdev_region(&g_device_number, FIRST_MINOR_NUMBER, TOTAL_MINOR_NUMBERS, NAME_OF_DEVICE);
    if (status < 0)
        goto erro_tag;

    pr_info("%s: Device number <major>:<minor> = %d:%d\n ", __func__,  MAJOR(g_device_number), MINOR(g_device_number));


    /* 2. Initialize the cdev structure with fops */
    (void) cdev_init(&g_cdev, &g_fops);
    
    /* 3. Register a device (cdev) structure with VFS  */
   status =  cdev_add(&g_cdev, g_device_number, TOTAL_MINOR_NUMBERS);

   if (status < 0)
        goto unregister_chrdev_region_tag;

    g_cdev.owner = THIS_MODULE;

    /* 4.1 Create device class under /sys/class/  */
    gp_class = class_create(THIS_MODULE, "pcd_class");
    if(IS_ERR(gp_class))
    {
        pr_err("Class creation failed \n");
        status = PTR_ERR(gp_class);
        goto c_dev_del_tag;
    }

    /* 4.2 Populate the sysfs with device information  */
    gp_device =  device_create(gp_class, NULL, g_device_number, NULL, "pcd");
     if(IS_ERR(gp_device))
    {
        pr_err("Device create failed \n");
        status = PTR_ERR(gp_device);
        goto class_destroy_tag;
    }

   
    pr_info (" Module  init was successfull \n");

    return 0;

class_destroy_tag:
    class_destroy(gp_class);

c_dev_del_tag:
    cdev_del(&g_cdev);

unregister_chrdev_region_tag:
    unregister_chrdev_region(g_device_number, TOTAL_MINOR_NUMBERS); 

erro_tag:
    pr_err("Module init was failed \n");
    return status;
}




static void __exit buffering_module_cleanup(void)
{
    device_destroy(gp_class, g_device_number);

    class_destroy(gp_class);
    
    cdev_del(&g_cdev);

    unregister_chrdev_region(g_device_number, TOTAL_MINOR_NUMBERS);

    pr_info(" Module unloaded\n");
}


loff_t pcd_lseek (struct file * filp, loff_t off, int whence)
{
    pr_info("lseek requested \n");
    pr_info(" Current file position = %lld\n", filp->f_pos);    
    
    switch(whence)
    {
        case SEEK_SET:
          if ((off >= BUFFER_SIZE) || (off < 0))
           {
             return -EINVAL;
           }

            filp->f_pos = off;
            break;
        case SEEK_CUR:
          if( ((filp->f_pos + off) >= BUFFER_SIZE)||( (filp->f_pos + off) < 0)) 
           {
             return -EINVAL;
           }
            filp->f_pos += off;
            break;
        case SEEK_END:
         if( ((BUFFER_SIZE + off) >= BUFFER_SIZE)||( (BUFFER_SIZE+ off) < 0)) 
          {
             return -EINVAL;
          }
        
         filp->f_pos = BUFFER_SIZE + off;
         break;

        default:
            return -EINVAL;
    }

    pr_info(" New value of file position = %lld\n", filp->f_pos);    
  
    return filp->f_pos;
}

ssize_t pcd_read (struct file * filp, char __user *buff, size_t count, loff_t * f_pos)
{
   pr_info(" Read requested for %zu bytes \n", count);

   pr_info(" Current file position = %lld\n", *f_pos);    
  
   /* Adjust  the 'count' */
   if ((*f_pos + count) > BUFFER_SIZE)
    {
        count = BUFFER_SIZE - *f_pos;
    }

   /* Copy to user */
   if ( copy_to_user(buff, &g_buffer[*f_pos], count))
    {
        return -EFAULT;
    }


    /* Update current file location  */
    *f_pos = (*f_pos + count);

   pr_info(" Number of bytes successully read =  %zu \n", count);

   pr_info(" Updated file position  =  %lld \n", *f_pos);

    /* Return number of bytes  which have been successully read */
    
    return count;
}

ssize_t pcd_write (struct file *filp, const char __user *buff, size_t count, loff_t * f_pos)
{
    pr_info(" Write requested for %zu bytes \n", count);

    pr_info(" Current file position = %lld\n", *f_pos);    
  
    /* Adjust 'count' */
    if ((*f_pos + count) > BUFFER_SIZE)
        count = BUFFER_SIZE - *f_pos;
  
    if(!count)
        return -ENOMEM;

    /* Copy from user */
   if ( copy_from_user(&g_buffer[*f_pos],buff ,count))
    {
        return -EFAULT;
    }

    /* Update current file location  */
    *f_pos = (*f_pos + count);

   pr_info(" Number of bytes successully written =  %zu \n", count);

   pr_info(" Updated file position  =  %lld \n", *f_pos);


    /* Return number of bytes  which have been successully written */ 

    return count;
}

int pcd_open (struct inode * inode, struct file * filp)
{
    pr_info( "Open was successull\n");
    
    return 0;
}


int pcd_release (struct inode * inode, struct file * filp)
{
    
    pr_info( "Close was successull\n")
      ;
    return 0;
}




module_init(buffering_module_init);
module_exit(buffering_module_cleanup);

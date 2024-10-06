/*******************************************************************************
 * Includes
 ******************************************************************************/

#include<linux/module.h> /* for every modules */

#include <linux/init.h>

#include <linux/kernel.h>

#include <linux/fs.h> /* for alloc_chrdev_region function */

#include<linux/cdev.h> /* for cdev_init function */

#include <linux/device.h>

#include <linux/kdev_t.h>

#include<linux/uaccess.h>

#include<linux/err.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#undef pr_fmt
#define pr_fmt(fmt) "%s : " fmt , __func__

#define NO_OF_DEVICES                 4
#define BUFFER_SIZE_DEV1              512
#define BUFFER_SIZE_DEV2              128
#define BUFFER_SIZE_DEV3              1024
#define BUFFER_SIZE_DEV4              256

#define FIRST_MINOR_NUMBER       0
#define TOTAL_MINOR_NUMBERS      1
#define NAME_OF_DEVICE           "Multiple Buffering"   /*This is just name of the device not the device file name  */

#define READ_ONLY 0x1
#define WRITE_ONLY 0x10
#define READ_WRITE 0x11


/* Device private data structure */
struct cdev_private_data_T
{
    char * buffer;
    unsigned size;
    const char *serial_number;
    int perm;
    struct cdev cdev;
};

/* Driver private data structure */
struct drv_private_data_T
{
    /** This variable holds device number
     * uint32_t :
     * 12 bits to store major number -> MAJOR(device_number) -> linux/kdev_t.h
     * 20 bits to store minor number -> MINOR(device_number) -> linux/kdev_t.h
     * device_number = MKDEV(major,minor);
     */
    dev_t device_number;

    struct class *class;

    struct device *device;

    int total_devices;

    struct cdev_private_data_T cdev_dataArray[NO_OF_DEVICES];
};

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

loff_t pcd_lseek (struct file * filp, loff_t off, int whence);
ssize_t pcd_read (struct file * filp, char __user *buff, size_t count, loff_t * f_pos);
ssize_t pcd_write (struct file *filp, const char __user *buff, size_t count, loff_t * f_pos);
int pcd_open (struct inode * inode, struct file * filp);
int pcd_release (struct inode * inode, struct file * filp);

static int check_permission(int perm, int acc_mode);


/*******************************************************************************
 * Variables
 ******************************************************************************/

char g_buffer1[BUFFER_SIZE_DEV1];
char g_buffer2[BUFFER_SIZE_DEV2];
char g_buffer3[BUFFER_SIZE_DEV3];
char g_buffer4[BUFFER_SIZE_DEV4];


struct drv_private_data_T g_drv_private_data =
{
    .total_devices = NO_OF_DEVICES,
    .cdev_dataArray =
        {
            [0] =
                {
                   .buffer = g_buffer1,
                   .size =BUFFER_SIZE_DEV1,
                   .serial_number = "Buffer 1",
                   .perm = READ_ONLY, /* RDONLY*/
                },
            [1] =
                {
                   .buffer = g_buffer2,
                   .size =BUFFER_SIZE_DEV2,
                   .serial_number  = "Buffer 2",
                   .perm = WRITE_ONLY, /* WRONLY */
                },
            [2] =
                {
                   .buffer = g_buffer3,
                   .size =BUFFER_SIZE_DEV3,
                   .serial_number  = "Buffer 3",
                   .perm = READ_WRITE, /* RDWR */
                },
            [3] =
                {
                   .buffer = g_buffer4,
                   .size =BUFFER_SIZE_DEV4,
                   .serial_number  = "Buffer 4",
                   .perm = READ_WRITE, /* RDWR */
                },
        }
};


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


/*******************************************************************************
 * Static Function
 ******************************************************************************/

static int __init mulBuffer_module_init(void)
{
    int status = 0 ;
    int i = -1;
    /* 1. Dynamically allocate a device number */
    status = alloc_chrdev_region( &g_drv_private_data.device_number, FIRST_MINOR_NUMBER, NO_OF_DEVICES, NAME_OF_DEVICE);
    if (status < 0)
        goto erro_tag;

    /* 2 Create device class under /sys/class/ */
    g_drv_private_data.class = class_create(THIS_MODULE, "mul_Buffers_class");
    if(IS_ERR(g_drv_private_data.class))
    {
        pr_err("Class creation failed \n");
        status = PTR_ERR(g_drv_private_data.class);
        goto unregister_chrdev_region_tag;
    }

    for ( i = 0; i < g_drv_private_data.total_devices; i++)
    {
        pr_info("Device number <major>:<minor> = %d:%d\n ", MAJOR(g_drv_private_data.device_number + i), MINOR(g_drv_private_data.device_number + i));

        /* 3. Initialize the cdev structure with fops */
        (void) cdev_init(&g_drv_private_data.cdev_dataArray[i].cdev, &g_fops);

        /* 4. Register a device (cdev) structure with VFS  */
        status =  cdev_add(&g_drv_private_data.cdev_dataArray[i].cdev, g_drv_private_data.device_number +i, TOTAL_MINOR_NUMBERS);

        if (status < 0)
                goto cdev_del_tag;

        g_drv_private_data.cdev_dataArray[i].cdev.owner = THIS_MODULE;

        /* 5 Populate the sysfs with device information  */
        g_drv_private_data.device =  device_create(g_drv_private_data.class, NULL,  g_drv_private_data.device_number + i, NULL, "pcdev-%d",i+1);
        if(IS_ERR(g_drv_private_data.device))
        {
            pr_err("Device create failed \n");
            status = PTR_ERR(g_drv_private_data.device);
            goto class_destroy_tag;
        }
    }



    pr_info (" Module  init was successfull \n");

    return 0;


cdev_del_tag:
class_destroy_tag:

    for (;i>=0; i--)
    {
        device_destroy(g_drv_private_data.class,  g_drv_private_data.device_number + i);

        cdev_del(&g_drv_private_data.cdev_dataArray[i].cdev);
    }

    class_destroy(g_drv_private_data.class);

unregister_chrdev_region_tag:
    unregister_chrdev_region(g_drv_private_data.device_number, NO_OF_DEVICES);
erro_tag:
    pr_err("Module init was failed \n");
    return status;
}

static void __exit mulBuffer_module_cleanup(void)
{
    int i =0;

    for (i = 0; i < g_drv_private_data.total_devices; i++)
    {
        device_destroy(g_drv_private_data.class, g_drv_private_data.device_number + i);

        cdev_del(&g_drv_private_data.cdev_dataArray[i].cdev);
    }

    class_destroy(g_drv_private_data.class);

    unregister_chrdev_region(g_drv_private_data.device_number, NO_OF_DEVICES);

    pr_info(" Module unloaded\n");
}


static int check_permission(int perm, int acc_mode)
{
    if (READ_WRITE == perm)
    {
        return 0;
    }

    if ((READ_ONLY == perm) && ((acc_mode & FMODE_READ )&& (acc_mode & FMODE_WRITE)))
    {
        return 0;
    }

    if ((WRITE_ONLY == perm) && ((acc_mode & FMODE_WRITE ) && !(acc_mode & FMODE_READ)))
    {
        return 0;
    }

    return -EPERM;
}


/*******************************************************************************
 *  Function
 ******************************************************************************/

loff_t pcd_lseek (struct file * filp, loff_t off, int whence)
{
    struct cdev_private_data_T * pcdev = (struct cdev_private_data_T *) filp->private_data;
    int max_size = pcdev->size;

    pr_info("lseek requested \n");
    pr_info(" Current file position = %lld\n", filp->f_pos);    

    switch(whence)
    {
        case SEEK_SET:
          if ((off >= max_size) || (off < 0))
           {
             return -EINVAL;
           }

            filp->f_pos = off;
            break;
        case SEEK_CUR:
          if( ((filp->f_pos + off) >= max_size)||( (filp->f_pos + off) < 0))
           {
             return -EINVAL;
           }
            filp->f_pos += off;
            break;
        case SEEK_END:
         if( ((max_size + off) >= max_size)||( (max_size+ off) < 0))
          {
             return -EINVAL;
          }
        
         filp->f_pos = max_size + off;
         break;

        default:
            return -EINVAL;
    }

    pr_info(" New value of file position = %lld\n", filp->f_pos);    
  
    return filp->f_pos;
}

ssize_t pcd_read (struct file * filp, char __user *buff, size_t count, loff_t * f_pos)
{
    struct cdev_private_data_T * pcdev = (struct cdev_private_data_T *) filp->private_data;
    int max_size = pcdev->size;

   pr_info(" Read requested for %zu bytes \n", count);

   pr_info(" Current file position = %lld\n", *f_pos);    
  
   /* Adjust  the 'count' */
   if ((*f_pos + count) > max_size)
    {
        count = max_size - *f_pos;
    }

   /* Copy to user */
   if (copy_to_user(buff, &(pcdev->buffer[*f_pos]), count))
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
    struct cdev_private_data_T * pcdev = (struct cdev_private_data_T *) filp->private_data;
    int max_size = pcdev->size;

    pr_info(" Write requested for %zu bytes \n", count);

    pr_info(" Current file position = %lld\n", *f_pos);    
  
    /* Adjust 'count' */
    if ((*f_pos + count) > max_size)
        count = max_size - *f_pos;
  
    if(!count)
        return -ENOMEM;

    /* Copy from user */
   if (copy_from_user(&(pcdev->buffer[*f_pos]),buff ,count))
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
    int status = 0;
    int minor_number;
    struct cdev_private_data_T *pcdev;

    /*  find out on which device file the operation was attempted */
    minor_number = MINOR(inode->i_rdev);

    pr_info( "Minor access = %d\n",minor_number);

    /* get device's private data structure */
    pcdev = container_of(inode->i_cdev, struct cdev_private_data_T, cdev);

    /* To supply device  private data to other methods of the driver */
    filp->private_data = (void *)pcdev;

    status = check_permission(pcdev->perm, filp->f_mode);

    (!status) ? pr_info( "Open was successull\n") : pr_info( "Open was failed\n");

    return 0;
}


int pcd_release (struct inode * inode, struct file * filp)
{
    
    pr_info( "Close was successull\n")
      ;
    return 0;
}




module_init(mulBuffer_module_init);
module_exit(mulBuffer_module_cleanup);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Truong Le Van");
MODULE_DESCRIPTION("A simple module to read/write multiple buffers");
MODULE_VERSION("1.0");

/*******************************************************************************
 *  END_OF_FILE
 ******************************************************************************/
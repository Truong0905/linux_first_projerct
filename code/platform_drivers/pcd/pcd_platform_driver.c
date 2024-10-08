/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "pcd_platform.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
struct device_cfg_type
{
    int cfg_0;
    int cfg_1;
};

enum device_version_t{
    T_VER_1 =0,
    T_VER_2 =1,
};



/*******************************************************************************
 * Prototypes
 ******************************************************************************/

loff_t pcd_lseek (struct file * filp, loff_t off, int whence);
ssize_t pcd_read (struct file * filp, char __user *buff, size_t count, loff_t * f_pos);
ssize_t pcd_write (struct file *filp, const char __user *buff, size_t count, loff_t * f_pos);
int pcd_open (struct inode * inode, struct file * filp);
int pcd_release (struct inode * inode, struct file * filp);
int pcd_platform_driver_prove(struct platform_device *pdev);
int pcd_platform_driver_remove(struct platform_device *pdev);


static int check_permission(int perm, int acc_mode);


/*******************************************************************************
 * Variables
 ******************************************************************************/
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


struct pcdrv_private_data pcdrv_data =
{
    .total_devices = 0,
};

struct device_cfg_type device_cfg_array[2] =
{
    [T_VER_1] = {.cfg_0 = 1, .cfg_1 = 2},
    [T_VER_2] = {.cfg_0 = 3, .cfg_1 = 4}
};

struct platform_device_id pcdevs_id[DEVICE_VERSION_NUM] =
{
    [0] = {
        .name = DEVICE_NAME_DETECTED_1,
        .driver_data = T_VER_1 , /* [T_VER_2] = {.cfg_0 = 1, .cfg_1 = 2}, */
    },
    [1] = {
        .name = DEVICE_NAME_DETECTED_2,
        .driver_data = T_VER_2 , /* [T_VER_2] = {.cfg_0 = 3, .cfg_1 = 4}, */
    }
};

struct platform_driver pcd_platform_driver =
{
    .probe      = pcd_platform_driver_prove,
    .remove     = pcd_platform_driver_remove,
    .id_table   = pcdevs_id,
    .driver = {
		.name = "pseudo-char-device"
	}
};

/*******************************************************************************
 * Static Function
 ******************************************************************************/

static int __init pcd_platform_driver_init (void)
{
    int status = 0 ;

    pr_info("Call alloc_chrdev_region \n");
    /* 1. Dynamically allocate a device number for MAX_DEVICES */
    status = alloc_chrdev_region( &pcdrv_data.device_base_num, 0, MAX_DEVICES, DEVICE_NAME);
    if (status < 0)
    {
        pr_err("alloc chrdev failed \n");
        goto erro_tag;
    }

    pr_info("Call class_create \n");
    /* 2 Create device class under /sys/class/ */
    pcdrv_data.class = class_create(THIS_MODULE, CLASS_FILE_NAME);
    if(IS_ERR(pcdrv_data.class))
    {
        pr_err("Class creation failed \n");
        status = PTR_ERR(pcdrv_data.class);
        goto unregister_chrdev_region_tag;
    }

    pr_info("Call platform_driver_register \n");
   status = platform_driver_register(&pcd_platform_driver);
   if (status)
   {
        pr_err("platform_driver_register failed \n");
        goto class_destroy_tag;
   }

   pr_info("Platform driver loaded \n");

   return 0;

class_destroy_tag:
    class_destroy(pcdrv_data.class);
unregister_chrdev_region_tag:
    unregister_chrdev_region(pcdrv_data.device_base_num, MAX_DEVICES);
erro_tag:
    return status;

}

static void __exit pcd_platform_driver_exit(void)
{
    platform_driver_unregister(&pcd_platform_driver);

    class_destroy(pcdrv_data.class);

    unregister_chrdev_region(pcdrv_data.device_base_num, MAX_DEVICES);


   pr_info("Platform driver unloaded \n");
}

module_init(pcd_platform_driver_init);
module_exit(pcd_platform_driver_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Truong Le Van");
MODULE_DESCRIPTION("PCD platform driver");
MODULE_VERSION("1.0");


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
    struct pcdev_private_data * pcdev = (struct pcdev_private_data *) filp->private_data;
    int max_size = pcdev->pData.size;

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
    struct pcdev_private_data * pcdev = (struct pcdev_private_data *) filp->private_data;
    int max_size = pcdev->pData.size;

   pr_info(" Read requested for %zu bytes \n", count);

   pr_info(" Current file position = %lld\n", *f_pos);
   /* Adjust  the 'count' */
   if ((*f_pos + count) > max_size)
    {
        count = max_size - *f_pos;
    }

   /* Copy to user */
   if ( copy_to_user(buff, &(pcdev->buffer[*f_pos]), count))
    {
        pr_err(" copy_to_user failed \n");
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
    struct pcdev_private_data * pcdev = (struct pcdev_private_data *) filp->private_data;
    int max_size = pcdev->pData.size;

    pr_info(" Write requested for %zu bytes \n", count);

    pr_info(" Current file position = %lld\n", *f_pos);
  
    /* Adjust 'count' */
    if ((*f_pos + count) > max_size)
        count = max_size - *f_pos;
  
    if(!count)
    {
        pr_err(" No Memory \n");
        return -ENOMEM;
    }

    /* Copy from user */
   if ( copy_from_user(&(pcdev->buffer[*f_pos]),buff ,count))
    {
        pr_err(" copy_from_user failed \n");
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
    struct pcdev_private_data *pcdev;

    /*  find out on which device file the operation was attempted */
    minor_number = MINOR(inode->i_rdev);

    pr_info( "Minor access = %d\n",minor_number);

    /* get device's private data structure */
    pcdev = container_of(inode->i_cdev, struct pcdev_private_data, cdev);

    /* To supply device  private data to other methods of the driver */
    filp->private_data = (void *)pcdev;

    status = check_permission(pcdev->pData.perm, filp->f_mode);

    (!status) ? pr_info( "Open was successull\n") : pr_info( "Open was failed\n");

    return 0;
}


int pcd_release (struct inode * inode, struct file * filp)
{
    
    pr_info( "Close was successull\n");
    return 0;
}

/* Gets called when matched platform device is found */
int pcd_platform_driver_prove(struct platform_device *pdev)
{
    int status = 0;

    struct pcdev_private_data *pdev_data;
    struct pcdev_platform_data *pdata;

    pr_info( "A device is detected \n");

    /* 1. Get platform data */
    pdata = (struct pcdev_platform_data *) dev_get_platdata(&pdev->dev);
    if(!pdata)
    {
        pr_err("Get platform data failed \n");
        status = -EINVAL;
        return status;
    }

    /* 2. Dynamically allowcate memory for device private data*/
    pdev_data = devm_kzalloc(&pdev->dev,sizeof(struct pcdev_private_data), GFP_KERNEL );
    if(!pdev_data)
    {
        pr_err("Cannot allowcate memory for pdev_data \n");
        status = -ENOMEM;
        return status;
    }

    /* save private data pointer in platform device structure */
    (void)dev_set_drvdata(&pdev->dev, pdev_data);

    pdev_data->pData.size = pdata->size;
    pdev_data->pData.perm = pdata->perm;
    pdev_data->pData.serial_number = pdata->serial_number;

    pr_info( "pdev_data->pData.size = %d \n", pdev_data->pData.size);
    pr_info( " pdev_data->pData.perm = %d \n", pdev_data->pData.perm);
    pr_info( "pdev_data->pData.serial_number = %s \n", pdev_data->pData.serial_number);

    pr_info (" cfg_0 = %d\n",device_cfg_array[pdev->id_entry->driver_data].cfg_0);
    pr_info (" cfg_1 = %d\n",device_cfg_array[pdev->id_entry->driver_data].cfg_1);

    /* 3. Dynamically allowcate memory for the device buffer uisng size information
            from platform data */
    pdev_data->buffer = devm_kzalloc(&pdev->dev, pdev_data->pData.size, GFP_KERNEL );
    if(!pdev_data->buffer)
    {
        pr_err("Cannot allowcate memory for buffer \n");
        status = -ENOMEM;
        return status;
    }
    /* 4. Get device number*/
    pdev_data->dev_number = pcdrv_data.device_base_num + pdev->id;

    /* 5. Do cdev_init and cdev_add */
    (void) cdev_init(&pdev_data->cdev, &g_fops);

    status =  cdev_add(&pdev_data->cdev, pdev_data->dev_number, 1);
    if (status < 0)
    {
        pr_err("cdev_add failed \n");
        return status;
    }
    pdev_data->cdev.owner = THIS_MODULE;
    /* 6. Create device file for the deteted platform device */
    pcdrv_data.device =  device_create(pcdrv_data.class, NULL,  pdev_data->dev_number, NULL, "pcdev-%d",pdev->id);

    if(IS_ERR(pcdrv_data.device))
    {
        pr_err("Device create failed \n");

        status = PTR_ERR(pcdrv_data.device);

        cdev_del(&pdev_data->cdev);

        return status;
    }

    pcdrv_data.total_devices++;
    pr_info( "The probe was succesfull\n");

    return 0;
}

/* Gets called when the device is removed from the system */
int pcd_platform_driver_remove(struct platform_device *pdev)
{
    struct pcdev_private_data *pdev_data = dev_get_drvdata(&pdev->dev);


    /* 1. Remove a device that was created by device_create*/
    device_destroy(pcdrv_data.class, pdev_data->dev_number);

    /* 2. Remove a cdev entry from the system */
    cdev_del(&pdev_data->cdev);

    pcdrv_data.total_devices--;
    pr_info( "A device is removed \n");

    return 0;
}

/*******************************************************************************
 *  END_OF_FILE
 ******************************************************************************/
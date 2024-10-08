#ifndef __PCD_PLATFORM_H__
#define __PCD_PLATFORM_H__
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

#include<linux/platform_device.h>

#include<linux/slab.h> /* Kmalloc and Kfree */

#include<linux/mod_devicetable.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifdef pr_fmt
    #undef pr_fmt
    #define pr_fmt(fmt) "%s : " fmt , __func__
#endif /* pr_fmt */

#define MAX_DEVICES  5
#define DEVICE_VERSION_NUM 2
#define DEVICE_NAME_DETECTED_1 "pcd1\0"
#define DEVICE_NAME_DETECTED_2 "pcd2\0"

#define DEVICE_NAME "pseudo device"
#define CLASS_FILE_NAME "class_pcd"


#define READ_ONLY 0x1
#define WRITE_ONLY 0x10
#define READ_WRITE 0x11



struct pcdev_platform_data
{
    int size;
    int perm;
    const char * serial_number;
};


/* Device private data structure */
struct pcdev_private_data
{
    struct pcdev_platform_data pData;
    char * buffer;
    dev_t dev_number;    /** This variable holds device number
                            * uint32_t :
                            * 12 bits to store major number -> MAJOR(device_number) -> linux/kdev_t.h
                            * 20 bits to store minor number -> MINOR(device_number) -> linux/kdev_t.h
                            * device_number = MKDEV(major,minor);
                            */
    struct cdev cdev;
};

/* Driver private data structure */
struct pcdrv_private_data
{
    int total_devices;
    dev_t device_base_num;
    struct class *class;
    struct device *device;
};

/*******************************************************************************
 * Prototypes
 ******************************************************************************/


/*******************************************************************************
 * Variables
 ******************************************************************************/


#endif /* __PCD_PLATFORM_H__ */
/*******************************************************************************
 *  END_OF_FILE
 ******************************************************************************/

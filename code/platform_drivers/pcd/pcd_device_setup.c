/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "pcd_platform.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void pcdev_release(struct device *dev);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/* Create 2 platform data */
struct pcdev_platform_data pcdev_data[MAX_DEVICES] =
{
    [0] = {.size = 512, .perm = READ_WRITE, .serial_number = "PCDEV1"},
    [1] = {.size = 512, .perm = READ_WRITE, .serial_number = "PCDEV1"},
};

/* Create 2 platform devices  */
struct platform_device platform_pcdev_1 =
{
    .name   = DEVICE_NAME_DETECTED,
    .id     = 0,
    .dev    = {
                .platform_data = (void *)&pcdev_data[0],
                .release = pcdev_release,
              },
};

struct platform_device platform_pcdev_2 =
{
    .name   = DEVICE_NAME_DETECTED,
    .id     = 1,
    .dev    = {
                .platform_data = (void *)&pcdev_data[1],
                .release = pcdev_release,
             },
};

/*******************************************************************************
 * Static Function
 ******************************************************************************/

static int __init pcdev_platform_init(void)
{
    int retVal = 0;
    /* Register platform devices */
    retVal |= platform_device_register(&platform_pcdev_1);
    retVal |= platform_device_register(&platform_pcdev_2);

    if (!retVal)
    {
        pr_info(" Device setup module loaded \n");
    }
    else
    {
        pr_info(" Device setup module cannot load \n");

    }

    return retVal;
}

static void __exit pcdev_platform_exit(void)
{
    platform_device_unregister(&platform_pcdev_1);
    platform_device_unregister(&platform_pcdev_2);

    pr_info(" Device setup module unloaed \n");
}


module_init(pcdev_platform_init);
module_exit(pcdev_platform_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Truong Le Van");
MODULE_DESCRIPTION("PCD platform driver");
MODULE_VERSION("1.0");

/*******************************************************************************
 *  Function
 ******************************************************************************/
void pcdev_release(struct device *dev)
{
    pr_info(" Device released \n");
}


/*******************************************************************************
 *  END_OF_FILE
 ******************************************************************************/
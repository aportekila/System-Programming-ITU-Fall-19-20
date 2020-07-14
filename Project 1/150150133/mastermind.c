#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h>	/* printk() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/seq_file.h>
#include <linux/cdev.h>
#include <linux/string.h> /* strncpy */

#include <asm/switch_to.h>		/* cli(), *_flags */
#include <asm/uaccess.h>	/* copy_*_user */


//  check if linux/uaccess.h is required for copy_*_user
//instead of asm/uaccess
//required after linux kernel 4.1+ ?
#ifndef __ASM_ASM_UACCESS_H
    #include <linux/uaccess.h>
#endif


#include "mastermind_ioctl.h"

MODULE_AUTHOR("Bengisu Guresti, Ufuk Demir, Abdullah Akgul");
MODULE_LICENSE("Dual BSD/GPL");


#define MMIND_MAJOR 0
#define MMIND_MAX_GUESSES 10
#define LINE_SIZE 16
#define NUMBER_OF_LINES 256

int mmind_major = MMIND_MAJOR;
int mmind_minor = 0; // predefined in assignment
int mmind_line_size = LINE_SIZE;
int mmind_number_of_lines = NUMBER_OF_LINES;
int mmind_max_guesses = MMIND_MAX_GUESSES;
int mmind_current_line = 0;
int mmind_current_guesses = 0;
int mmind_remaining = MMIND_MAX_GUESSES;
char *mmind_number = "4070\0";


module_param(mmind_major, int, S_IRUGO);
module_param(mmind_minor, int, S_IRUGO);
module_param(mmind_max_guesses, int, S_IRUGO);
module_param(mmind_number, charp, S_IRUGO);



struct mmind_dev{
	char **data;
	int mmind_max_guesses;
	int mmind_current_guesses;
    int mmind_current_line;
    int mmind_max_lines;
    char* mmind_number;
    struct semaphore sem;
    struct cdev cdev;
};

struct mmind_dev *mmind_device;

int mmind_trim(struct mmind_dev *dev)
{
	int i;
	printk(KERN_INFO "mmind: %s started\n", __FUNCTION__);
    
    if (dev->data) {
        for (i = 0; i < NUMBER_OF_LINES; i++) {
            if (dev->data[i])
                kfree(dev->data[i]);
        }
        kfree(dev->data);
    }
    
	dev->data = NULL;
    dev->mmind_current_guesses = mmind_current_guesses;
    dev->mmind_current_line = mmind_current_line;
    dev->mmind_max_guesses = mmind_max_guesses;
    dev->mmind_max_lines = NUMBER_OF_LINES;
    mmind_remaining = mmind_max_guesses;

    printk(KERN_INFO "mmind: %s finished\n", __FUNCTION__);
    return 0;
}




ssize_t mmind_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	
    struct mmind_dev *dev = filp->private_data;
    int current_guesses = dev->mmind_current_guesses;
    int max_guesses = dev->mmind_max_guesses;
    ssize_t retval = -ENOMEM;
    int current_line = dev->mmind_current_line;
    char* to_write;
    char* original = NULL;
    char _buf[4];
    int i = 0, j = 0, it = 0;
    int _pos = 0, _neg = 0;
    char org;
    int flag, _trash, temp;
    char attempt[4];
	int guessNumCount[10] = {0,0,0,0,0,0,0,0,0,0};
	int trueGuessCount[10] = {0,0,0,0,0,0,0,0,0,0};
	int falseGuessCount[10] = {0,0,0,0,0,0,0,0,0,0};
	
	
	
	
    printk(KERN_INFO "mmind: %s started\n", __FUNCTION__);
    
    if (down_interruptible(&dev->sem))
        return -ERESTARTSYS;

    if (current_line >= NUMBER_OF_LINES) {
        retval = -ENOMEM;
        goto out;
    }
	
    if (current_guesses >= max_guesses){
    	retval = -EDQUOT;
        goto out;
    }
    

    if (!dev->data) {
        dev->data = kmalloc(NUMBER_OF_LINES * sizeof(char *), GFP_KERNEL);
        if (!dev->data)
            goto out;
        memset(dev->data, 0, NUMBER_OF_LINES * sizeof(char *));
    }
    
    if (!dev->data[current_line]) {
        dev->data[current_line] = kmalloc(16 * sizeof(char), GFP_KERNEL);
        if (!dev->data[current_line])
            goto out;
    }
    
    i = 0;
    j = 0;
    while (((char *)buf)[i] != '\0' && i < 5){
		it = buf[i] - '0';
		if (!(it > 9 || it < 0)){
			j++;
		}
		i++;
	}
	
    if(j != 4){
		retval = - EINVAL; 
		printk(KERN_INFO "mmind: Given guess's length is not 4\n");
		goto out;
	}
	else{
		for(i = 0; i < 4; i++){
			it = buf[i] - '0';
			if (it > 9 || it < 0){
				retval = - EINVAL;
				printk(KERN_INFO "mmind: Given guess is not digit\n");
				goto out;
			}
		}
	}
    i = 0;
    j = 0;

    original = dev->mmind_number;

	to_write = kmalloc(16 * sizeof(char), GFP_KERNEL);
	
	if (copy_from_user(to_write, buf , count)) {
        retval = -EFAULT;
        goto out0;
    }

    
    for (i = 0; i < 4; i ++){
		_buf[i] = buf[i];
		guessNumCount[_buf[i] - '0']++;
		
	}
	
	
	
	
    
    for(i = 0; i < 4; i ++){
        org = original[i];
        flag = 0;
        for (j = 0; j < 4; j++){
            if(org == _buf[j]){
                if(i == j){
                    _pos += 1;
                    flag = 0;
                    _buf[j] = 'x';
                    trueGuessCount[org - '0'] += 1;
                    break;
                }
                else{
                    flag++;
                }
            }
        }
        if (flag){
            _neg += 1;
            falseGuessCount[org - '0'] += 1;
        }
    }

	for(i = 0; i < 4; i++){
		if(guessNumCount[buf[i] - '0'] != -1){
			if(guessNumCount[buf[i]  - '0'] < trueGuessCount[buf[i]  - '0'] + falseGuessCount[buf[i] - '0'] ){
				_neg -= trueGuessCount[buf[i] - '0'] + falseGuessCount[buf[i] - '0'] - guessNumCount[buf[i] - '0'];
				guessNumCount[buf[i] - '0'] = -1;
			}
		}
	}


    to_write[4] = 32;
    to_write[5] = _pos + '0';
    to_write[6] = '+';
    to_write[7] = 32;
    to_write[8] = _neg + '0';
    to_write[9] = '-';
    to_write[10] = 32;
    dev->mmind_current_guesses++;
    temp = dev->mmind_current_guesses;
    for(i = 0; i < 4; i++){
        _trash = temp % 10;
        temp /= 10;
        attempt[3-i] = _trash + '0';
    }
    

    to_write[11] = attempt[0];
    to_write[12] = attempt[1];
    to_write[13] = attempt[2];
    to_write[14] = attempt[3];
    to_write[15] = '\n';

	strncpy(dev->data[current_line], to_write, 16);
    
    *f_pos += count;
    retval = count;
	
	mmind_current_guesses++;
	mmind_current_line++;
	mmind_remaining--;
    dev->mmind_current_line += 1;
  out0:
	kfree(to_write);
  out:
  	up(&dev->sem);
    printk(KERN_INFO "mmind: %s finished\n", __FUNCTION__);
    return retval;
}


ssize_t mmind_read(struct file *filp, char __user *buf, size_t count,
                   loff_t *f_pos)
{
    
    struct mmind_dev *dev = filp->private_data;
    int current_line = dev->mmind_current_line;
    char *to_return;
    int i = 0, j = 0, k = 0;
    ssize_t retval = 0;
    
    printk(KERN_INFO "mmind: %s started\n", __FUNCTION__);
    
    if (down_interruptible(&dev->sem))
        return -ERESTARTSYS;
        
    if (*f_pos > 16* NUMBER_OF_LINES){
		goto out;
	}

	if (*f_pos + count > 16* NUMBER_OF_LINES){
		count = 16* mmind_current_line - *f_pos;
	}
	  
    if (current_line > NUMBER_OF_LINES)
        goto out;
    

    if (dev->data == NULL){
        goto out;
    }
    

    if (16*current_line < count){
        count = 16*current_line;
    }

	to_return = kmalloc(count*sizeof(char), GFP_KERNEL);
	memset(to_return, '\0', count );
	
	
	i = 0;
	j = 0;
	k = 0;
	
	while(i < count){
		to_return[i] = dev->data[j][k];
		k++;
		i++;
		if (i%16 == 0){
			j++;
			k = 0;
		}
	}
	
	
    if (copy_to_user(buf, to_return, count)) {
        retval = -EFAULT;
        goto out;
    }
    
    *f_pos += count;
    kfree(to_return);
    retval = count;

  out:
	up(&dev->sem);
	printk(KERN_INFO "mmind: %s finished\n", __FUNCTION__);
    return retval;
}


int mmind_open(struct inode *inode, struct file *filp)
{
	struct mmind_dev *dev;
	printk(KERN_INFO "mmind: %s started\n", __FUNCTION__);
    dev = container_of(inode->i_cdev, struct mmind_dev, cdev);
    filp->private_data = dev;
    printk(KERN_INFO "mmind: dev->mmind_max_guesses = %d\n", dev->mmind_max_guesses);
    printk(KERN_INFO "mmind: dev->mmind_current_guesses = %d\n", dev->mmind_current_guesses);
    printk(KERN_INFO "mmind: dev->mmind_current_line = %d\n", dev->mmind_current_line);
    printk(KERN_INFO "mmind: dev->mmind_max_lines = %d\n", dev->mmind_max_lines);
    printk(KERN_INFO "mmind: dev->mmind_number = %s\n", dev->mmind_number);
    printk(KERN_INFO "mmind: mmind_major= %d\n", mmind_major);
    printk(KERN_INFO "mmind: %s finished\n", __FUNCTION__);
    return 0;
}

int mmind_release(struct inode *inode, struct file *filp)
{
	printk(KERN_INFO "mmind: %s started\n", __FUNCTION__);
	printk(KERN_INFO "mmind: %s finished\n", __FUNCTION__);
    return 0;
}


long mmind_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    
    int err = 0, i = 0, retval = 0, it = 0;
    struct mmind_dev *dev;
    
    printk(KERN_INFO "mmind: %s started\n", __FUNCTION__);
    
    
    if (_IOC_TYPE(cmd) != MMIND_IOC_MAGIC) return -ENOTTY;
    if (_IOC_NR(cmd)  > MMIND_IOC_MAXNR) return -ENOTTY;
    
    if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	if (err) return -EFAULT;
    
    
    switch(cmd) {
		case MMIND_REMAINING:
			printk(KERN_INFO "mmind: mmind_ioctl MMIND_REMAINING started\n");
			printk(KERN_INFO "mmind: mmind_remaining = %d\n", mmind_remaining);
			retval = __put_user(mmind_remaining, (int __user *)arg);
			printk(KERN_INFO "mmind: mmind_ioctl MMIND_REMAINING finished\n");
			break;
		case MMIND_ENDGAME:
			printk(KERN_INFO "mmind: mmind_ioctl MMIND_ENDGAME started\n");
			dev = filp->private_data;
			mmind_current_guesses = 0;
			mmind_current_line = 0;
			mmind_remaining = mmind_max_guesses;
			mmind_trim(dev);
			printk(KERN_INFO "mmind: mmind_ioctl MMIND_ENDGAME finished\n");
			break;
		case MMIND_NEWGAME:
			printk(KERN_INFO "mmind: mmind_ioctl MMIND_NEWGAME started\n");
			dev = filp->private_data;
			if (dev->mmind_current_line >= dev->mmind_max_lines){
				printk(KERN_INFO "mmind: mmind_ioctl MMIND_NEWGAME \
					finished with err ENOMEM\n");
				return -ENOMEM;
			}
			
			if(strlen((char *)arg) != 4){
				printk(KERN_INFO "mmind: Given guess's length is not 4\n");
				return - EINVAL; 
			}
			else{
				for(i = 0; i < 4; i++){
					it = ((char *) arg)[i] - '0';
					if (it > 9 || it < 0){
						printk(KERN_INFO "mmind: Given guess is not digit\n");
						return - EINVAL; 
					}
				}
			}
			
			mmind_current_guesses = 0;
			dev->mmind_current_guesses = 0;
			mmind_remaining = dev->mmind_max_guesses - dev->mmind_current_guesses;
			
			strncpy(dev->mmind_number, (char *) arg, 5);
			
			
			printk(KERN_INFO "mmind: mmind_ioctl MMIND_NEWGAME finished\n");
			break;
		default:  /* redundant, as cmd was checked against MAXNR */
			printk(KERN_INFO "mmind: there is no ioctl command mmind_ioctl finished\n");
			return -ENOTTY;
	}
    
    
	printk(KERN_INFO "mmind: %s finished\n", __FUNCTION__);
	return retval;
}




struct file_operations mmind_fops = {
    .owner =    THIS_MODULE,
    .read =     mmind_read,
    .write =    mmind_write,
    .unlocked_ioctl =  mmind_ioctl,
    .open =     mmind_open,
    .release =  mmind_release,
};



void mmind_cleanup_module(void){
    dev_t devno = MKDEV(mmind_major, mmind_minor);

	printk(KERN_INFO "mmind: %s started\n", __FUNCTION__);
    if (mmind_device) {
        
        mmind_trim(mmind_device);
        cdev_del(&mmind_device->cdev);
        kfree(mmind_device);
        kfree(mmind_device->mmind_number);
    }
	mmind_current_guesses = 0;
	mmind_current_line = 0;
	mmind_remaining = mmind_max_guesses;
    unregister_chrdev_region(devno, 1);
    printk(KERN_INFO "mmind: %s finished\n", __FUNCTION__);
}
	


int mmind_init_module(void)
{
	int result, i, it = 0;
    int err;
    dev_t devno = 0;
    struct mmind_dev *dev;
    printk(KERN_INFO "mmind: %s started\n", __FUNCTION__);
    
    if (mmind_major) {
        devno = MKDEV(mmind_major, mmind_minor);
        result = register_chrdev_region(devno, 1, "mastermind");
    } 
    else { // dynamic allocation of major number as stated in assignment
        result = alloc_chrdev_region(&devno, mmind_minor, 1,
                                     "mastermind");
        mmind_major = MAJOR(devno);
    }

    if (result < 0) {
        printk(KERN_WARNING "mmind: can't get major %d\n", mmind_major);
        return result;
    }


    mmind_device = kmalloc(sizeof(struct mmind_dev),
                            GFP_KERNEL);
    if (!mmind_device) {
        result = -ENOMEM;
        goto fail;
    }
	
    memset(mmind_device, 0, sizeof(struct mmind_dev));

    /* Initialize  device. */
    
    if(strlen(mmind_number) != 4){
		result = - EINVAL; 
		printk(KERN_INFO "mmind: Given guess's length is not 4\n");
		goto fail;
	}
	else{
		for(i = 0; i < 4; i++){
			it = mmind_number[i] - '0';
			if (it > 9 || it < 0){
				result = - EINVAL;
				printk(KERN_INFO "mmind: Given guess is not digit\n");
				goto fail;
			}
		}
	}
    
    dev = mmind_device;
    dev->mmind_max_guesses = mmind_max_guesses;
    mmind_current_guesses = 0;
    mmind_current_line = 0;
    dev->mmind_current_guesses = mmind_current_guesses;
    dev->mmind_current_line = mmind_current_line;
    dev->mmind_max_lines = NUMBER_OF_LINES;
    dev->mmind_number = kmalloc(5 * sizeof(char), GFP_KERNEL);
    mmind_remaining = mmind_max_guesses;
    
    for (i = 0; i < 5; i ++){
		dev->mmind_number[i] = mmind_number[i];
	}
    printk("mmind: dev->mmind_max_guesses = %d\n", dev->mmind_max_guesses);
    printk("mmind: dev->mmind_current_guesses = %d\n", dev->mmind_current_guesses);
    printk("mmind: dev->mmind_current_line = %d\n", dev->mmind_current_line);
    printk("mmind: dev->mmind_max_lines = %d\n", dev->mmind_max_lines);
    printk("mmind: dev->mmind_number = %s\n", dev->mmind_number);
    printk("mmind: mmind_major= %d\n", mmind_major);
    
    sema_init(&dev->sem,1);
    devno = MKDEV(mmind_major, mmind_minor);
    cdev_init(&dev->cdev, &mmind_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &mmind_fops;

    err = cdev_add(&dev->cdev, devno, 1);
    if (err)
        printk(KERN_NOTICE "mmind: Error %d adding mmind", err);
    
	
	printk(KERN_INFO "mmind: %s finished success\n", __FUNCTION__);
    return 0; /* succeed */

  fail:
	mmind_cleanup_module();
    printk(KERN_INFO "mmind: %s finished failed\n", __FUNCTION__);
    return result;
}




module_init(mmind_init_module);
module_exit(mmind_cleanup_module);

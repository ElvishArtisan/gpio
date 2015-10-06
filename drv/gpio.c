/* gpio.c
 *
 * A device driver for ComputerBoard DIO cards.
 *
 * Copyright 2002,2004,2006 Fred Gleason <fredg@paravelsystems.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/module.h>
#ifndef KERNEL_2_4
#include <linux/moduleparam.h>
#endif
#include <asm/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/version.h>

#include "gpio.h"


/*
 * Global Variables
 */
int gpio_major=0;
unsigned board_quan=0;
spinlock_t gpio_spinlock=SPIN_LOCK_UNLOCKED;
struct gpio_info boards[GPIO_MAX_CARDS];
unsigned board_state[GPIO_MAX_CARDS];
unsigned board_filter[GPIO_MAX_CARDS];
unsigned long board_io[GPIO_MAX_CARDS][6];
unsigned board_len[GPIO_MAX_CARDS][6];
int board_irq[GPIO_MAX_CARDS];
unsigned board_model[GPIO_MAX_CARDS];  
unsigned board_open_count[GPIO_MAX_CARDS];
wait_queue_head_t board_queue[GPIO_MAX_CARDS];
volatile int board_sample[GPIO_MAX_CARDS];
unsigned short board_type[GPIO_MAX_CARDS];
unsigned short board_port[GPIO_MAX_CARDS];
unsigned short board_type_0=0;
unsigned short board_port_0=0;
unsigned short board_type_1=0;
unsigned short board_port_1=0;
unsigned short board_type_2=0;
unsigned short board_port_2=0;
unsigned short board_type_3=0;
unsigned short board_port_3=0;
unsigned short board_type_4=0;
unsigned short board_port_4=0;
unsigned short board_type_5=0;
unsigned short board_port_5=0;
unsigned short board_type_6=0;
unsigned short board_port_6=0;
unsigned short board_type_7=0;
unsigned short board_port_7=0;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 14)
struct class *gpio_class;
#endif

static struct pci_device_id gpio_pci_tbl[]=
  {
    {PCI_DEVICE(GPIO_CBOARDS_VENDOR_ID,GPIO_PCI_DIO24_DEVICE_ID)},
    {PCI_DEVICE(GPIO_CBOARDS_VENDOR_ID,GPIO_PCI_DIO24H_DEVICE_ID)},
    {PCI_DEVICE(GPIO_CBOARDS_VENDOR_ID,GPIO_PCI_PDISO8_DEVICE_ID)},
    {PCI_DEVICE(GPIO_CBOARDS_VENDOR_ID,GPIO_PCI_PDISO16_DEVICE_ID)},
    {PCI_DEVICE(GPIO_CBOARDS_VENDOR_ID,GPIO_PCI_DAS1000_DEVICE_ID)},
    {0}
  };
MODULE_DEVICE_TABLE(pci,gpio_pci_tbl);


unsigned gpio_get_inputs(int dev)
{
  switch(board_model[dev]) {
      case GPIO_PCI_DIO24_DEVICE_ID:
      case GPIO_PCI_DIO24H_DEVICE_ID:
	if(boards[dev].mode==GPIO_MODE_INPUT) {
	  return inb(board_io[dev][2])|
	    (inb(board_io[dev][2]+1)<<8)|
	    (inb(board_io[dev][2]+2)<<16);
	}
	break;
	
      case GPIO_PCI_PDISO8_DEVICE_ID:
	return inb(1+board_io[dev][1]);
	break;

      case GPIO_PCI_PDISO16_DEVICE_ID:
	  return inb(board_io[dev][1]+1)|(inb(board_io[dev][1]+5)<<8);
	break;

      case GPIO_PCI_DAS1000_DEVICE_ID:
	return inb(board_io[dev][3]+4);
	break;
  }
  return 0;
}


void gpio_set_output(struct gpio_line *out,int dev)
{
  unsigned state_mask;

  state_mask=1<<out->line;
  spin_lock(&gpio_spinlock);
  if(out->state) {
    board_state[dev]|=state_mask;
  }
  else {
    board_state[dev]&=(~state_mask);
  }
  switch(board_model[dev]) {
      case GPIO_PCI_DIO24_DEVICE_ID:
      case GPIO_PCI_DIO24H_DEVICE_ID:
	outb(board_state[dev]&0xFF,board_io[dev][2]);
	outb((board_state[dev]>>8)&0xFF,board_io[dev][2]+1);
	outb((board_state[dev]>>16)&0xFF,board_io[dev][2]+2);
	break;
	
      case GPIO_PCI_PDISO8_DEVICE_ID:
	outb(board_state[dev]&0xFF,board_io[dev][1]);
	break;

      case GPIO_PCI_PDISO16_DEVICE_ID:
	outb(board_state[dev]&0xFF,board_io[dev][1]);
	outb((board_state[dev]>>8)&0xFF,board_io[dev][1]+4);
	break;

      case GPIO_PCI_DAS1000_DEVICE_ID:
	outb(board_state[dev]&0xFF,board_io[dev][3]+5);
	outb((board_state[dev]>>8)&0xFF,board_io[dev][3]+6);
	break;
  }
  spin_unlock(&gpio_spinlock);
}


void gpio_set_filter(struct gpio_line *out,int dev)
{
  unsigned state_mask;


  state_mask=1<<out->line;
  spin_lock(&gpio_spinlock);
  if(out->state) {
    board_filter[dev]|=state_mask;
  }
  else {
    board_filter[dev]&=(~state_mask);
  }
  switch(board_model[dev]) {
      case GPIO_PCI_DIO24_DEVICE_ID:
      case GPIO_PCI_DIO24H_DEVICE_ID:
	break;
	
      case GPIO_PCI_PDISO8_DEVICE_ID:
	outb(board_filter[dev]&0xFF,board_io[dev][1]+2);
	break;

      case GPIO_PCI_PDISO16_DEVICE_ID:
	outb(board_filter[dev]&0xFF,board_io[dev][1]+2);
	outb((board_filter[dev]>>8)&0xFF,board_io[dev][1]+6);
	break;

      case GPIO_PCI_DAS1000_DEVICE_ID:
	break;
  }
  spin_unlock(&gpio_spinlock);
}


void gpio_set_outputs(struct gpio_mask *mask,int dev)
{
  spin_lock(&gpio_spinlock);
  board_state[dev]=mask->mask[0];
  switch(board_model[dev]) {
      case GPIO_PCI_DIO24_DEVICE_ID:
      case GPIO_PCI_DIO24H_DEVICE_ID:
	outb(board_state[dev]&0xFF,board_io[dev][2]);
	outb((board_state[dev]>>8)&0xFF,board_io[dev][2]+1);
	outb((board_state[dev]>>16)&0xFF,board_io[dev][2]+2);
	break;
	
      case GPIO_PCI_PDISO8_DEVICE_ID:
	outb(board_state[dev]&0xFF,board_io[dev][1]);
	break;

      case GPIO_PCI_PDISO16_DEVICE_ID:
	outb(board_state[dev]&0xFF,board_io[dev][1]);
	outb((board_state[dev]>>8)&0xFF,board_io[dev][1]+4);
	break;

      case GPIO_PCI_DAS1000_DEVICE_ID:
	outb(board_state[dev]&0xFF,board_io[dev][3]+5);
	outb((board_state[dev]>>8)&0xFF,board_io[dev][3]+6);
	break;
  }
  spin_unlock(&gpio_spinlock);
}


int gpio_set_filters(struct gpio_mask *mask,int dev)
{
  spin_lock(&gpio_spinlock);
  board_filter[dev]=mask->mask[0];
  switch(board_model[dev]) {
      case GPIO_PCI_DIO24_DEVICE_ID:
      case GPIO_PCI_DIO24H_DEVICE_ID:
	spin_unlock(&gpio_spinlock);
	return 1;
	break;
	
      case GPIO_PCI_PDISO8_DEVICE_ID:
	outb(board_filter[dev]&0xFF,board_io[dev][1]+2);
	spin_unlock(&gpio_spinlock);
	return 0;
	break;

      case GPIO_PCI_PDISO16_DEVICE_ID:
	outb(board_filter[dev]&0xFF,board_io[dev][1]+2);
	outb((board_filter[dev]>>8)&0xFF,board_io[dev][1]+6);
	break;

      case GPIO_PCI_DAS1000_DEVICE_ID:
	spin_unlock(&gpio_spinlock);
	return 1;
	break;
  }
  spin_unlock(&gpio_spinlock);
  return 1;
}


void gpio_set_mode(unsigned mode,int dev)
{
  spin_lock(&gpio_spinlock);
  switch(mode) {
      case GPIO_MODE_INPUT:
	boards[dev].inputs=24;
	boards[dev].outputs=0;
	outb(0x9B,board_io[dev][2]+3);
	board_state[dev]=0;
	outb(0,board_io[dev][2]);
	outb(0,board_io[dev][2]+1);
	outb(0,board_io[dev][2]+2);
	break;

      case GPIO_MODE_OUTPUT:
	boards[dev].inputs=0;
	boards[dev].outputs=24;
	outb(0x80,board_io[dev][2]+3);
	break;
  }
  boards[dev].mode=mode;
  spin_unlock(&gpio_spinlock);
}


int gpio_open(struct inode *inode,struct file *filp)
{
#ifdef KERNEL_2_4
  MOD_INC_USE_COUNT;
  if(MINOR(inode->i_rdev)>=board_quan) {
    MOD_DEC_USE_COUNT;
    return -ENODEV;
  }
#else
  if(iminor(inode)>=board_quan) {
    return -ENODEV;
  }
#endif  // KERNEL_2_4
  return 0;
}


int gpio_release(struct inode *inode,struct file *filp)
{
#ifdef KERNEL_2_4
  MOD_DEC_USE_COUNT;
#endif  // KERNEL_2_4
  return 0;
}


int gpio_ioctl(struct inode *inode,struct file *filp,
	       unsigned cmd,unsigned long arg)
{
  struct gpio_line out;
  struct gpio_mask mask;
  int mode;
  unsigned minor;

#ifdef KERNEL_2_4
  minor=MINOR(inode->i_rdev);
#else
  minor=iminor(inode);
#endif  // KERNEL_2_4

  switch(cmd) {
      case GPIO_GETINFO:
	if(copy_to_user((unsigned long *)arg,&boards[minor],
			sizeof(struct gpio_info))) {
	  return -EFAULT;
	}
	return 0;
	break;

      case GPIO_SETMODE:
	if(boards[minor].mode==GPIO_MODE_AUTO) {
	  return -EINVAL;
	}
	if(copy_from_user(&mode,(unsigned long *)arg,sizeof(unsigned))) {
	  return -EFAULT;
	}
       	if(boards[minor].mode==mode) {
	  return 0;
	}
	gpio_set_mode(mode,minor);
	return 0;
	break;

      case GPIO_GET_INPUTS:
	memset(&mask,0,sizeof(struct gpio_mask));
	mask.mask[0]=gpio_get_inputs(minor);
	if(copy_to_user((unsigned long *)arg,&mask,sizeof(struct gpio_mask))) {
	  return -EFAULT;
	}
	return 0;
	break;

      case GPIO_GET_OUTPUTS:
	memset(&mask,0,sizeof(struct gpio_mask));
	spin_lock(&gpio_spinlock);
	mask.mask[0]=board_state[minor];
	spin_unlock(&gpio_spinlock);
	if(copy_to_user((unsigned long *)arg,&mask,sizeof(struct gpio_mask))) {
	  return -EFAULT;
	}
	return 0;
	break;

      case GPIO_GET_FILTERS:
	memset(&mask,0,sizeof(struct gpio_mask));
	spin_lock(&gpio_spinlock);
	mask.mask[0]=board_filter[minor];
	spin_unlock(&gpio_spinlock);
	if(copy_to_user((unsigned long *)arg,&mask,sizeof(struct gpio_mask))) {
	  return -EFAULT;
	}
	return 0;
	break;

      case GPIO_SET_OUTPUT:
	if(copy_from_user(&out,(unsigned long *)arg,
			  sizeof(struct gpio_line))) {
	  return -EFAULT;
	}
	gpio_set_output(&out,minor);
	return 0;
	break;

      case GPIO_SET_FILTER:
	if(copy_from_user(&out,(unsigned long *)arg,
			  sizeof(struct gpio_line))) {
	  return -EFAULT;
	}
	gpio_set_filter(&out,minor);
	return 0;
	break;

      case GPIO_SET_OUTPUTS:
	if(copy_from_user(&mask,(unsigned long *)arg,
			  sizeof(struct gpio_mask))) {
	  return -EFAULT;
	}
	gpio_set_outputs(&mask,minor);
	return 0;
	break;

      case GPIO_SET_FILTERS:
	if(copy_from_user(&mask,(unsigned long *)arg,
			  sizeof(struct gpio_mask))) {
	  return -EFAULT;
	}
	if(gpio_set_filters(&mask,minor)==1) {
	  return -EINVAL;
	}
	return 0;
	break;
  }
  return -ENOTTY;
}


struct file_operations gpio_fops = {
  owner:THIS_MODULE,
  open:gpio_open,
  release:gpio_release,
  ioctl:gpio_ioctl,
};


int gpio_read_proc(char *page,char **start,off_t offset,int count,
		   int *eof,void *data) {
  int i;
  int j;
  int len=0;
  unsigned state;
  unsigned state_mask;

  len+=sprintf(page+len,"             Board: %s\n",boards[(long)data].name);
  switch(boards[(long)data].mode) {
      case GPIO_MODE_AUTO:
	len+=sprintf(page+len,"              Mode: AUTO\n");
	break;

      case GPIO_MODE_INPUT:
	len+=sprintf(page+len,"              Mode: INPUT\n");
	break;

      case GPIO_MODE_OUTPUT:
	len+=sprintf(page+len,"              Mode: OUTPUT\n");
	break;
  }
  len+=sprintf(page+len,"       Input Lines: %d\n",boards[(long)data].inputs);
  len+=sprintf(page+len,"      Output Lines: %d\n",boards[(long)data].outputs);
  len+=sprintf(page+len,"      Analog Lines: %d\n",boards[(long)data].samples);
  len+=sprintf(page+len,"  Sample Bit Depth: %d\n",boards[(long)data].depth);
  len+=sprintf(page+len,"\n");
  len+=sprintf(page+len,"   Inputs:\n");
  state=gpio_get_inputs((long)data);
  state_mask=1;
  for(i=0;i<(boards[(long)data].inputs/8);i++) {
    len+=sprintf(page+len,"    ");
    for(j=0;j<8;j++) {
      if((state_mask&state)==0) {
	len+=sprintf(page+len,"%02d:Off   ",8*i+j);
      }
      else {
	len+=sprintf(page+len,"%02d:On    ",8*i+j);
      }
      state_mask=state_mask<<1;
    }
    len+=sprintf(page+len,"\n");
  }

  len+=sprintf(page+len,"\n");
  len+=sprintf(page+len,"  Outputs:\n");
  spin_lock(&gpio_spinlock);
  state=board_state[(long)data];
  spin_unlock(&gpio_spinlock);
  state_mask=1;
  for(i=0;i<(boards[(long)data].outputs/8);i++) {
    len+=sprintf(page+len,"    ");
    for(j=0;j<8;j++) {
      if((state_mask&state)==0) {
	len+=sprintf(page+len,"%02d:Off   ",8*i+j);
      }
      else {
	len+=sprintf(page+len,"%02d:On    ",8*i+j);
      }
      state_mask=state_mask<<1;
    }
    len+=sprintf(page+len,"\n");
  }

  len+=sprintf(page+len,"\n");
  len+=sprintf(page+len,"  Filters:\n");
  if((boards[(long)data].caps&GPIO_CAP_FILTER)!=0) {
    spin_lock(&gpio_spinlock);
    state=board_filter[(long)data];
    spin_unlock(&gpio_spinlock);
    state_mask=1;
    for(i=0;i<(boards[(long)data].inputs/8);i++) {
      len+=sprintf(page+len,"    ");
      for(j=0;j<8;j++) {
	if((state_mask&state)==0) {
	  len+=sprintf(page+len,"%02d:Off   ",8*i+j);
	}
	else {
	  len+=sprintf(page+len,"%02d:On    ",8*i+j);
	}
	state_mask=state_mask<<1;
      }
      len+=sprintf(page+len,"\n");
    }
  }
  len+=sprintf(page+len,"\n");

  *eof=1;
  return len;
}


void gpio_config_boards(void)
{
  struct pci_dev *dev=NULL;
  int i;
  u8 irq;

  //
  // CIO-DIO24
  //
  for(i=0;i<GPIO_MAX_CARDS;i++) {
    if(board_type[i]==GPIO_PCI_DIO24_DEVICE_ID) {
      board_open_count[board_quan]=0;
      board_io[board_quan][2]=board_port[i];
      board_len[board_quan][2]=4;
      request_region(board_io[board_quan][2],board_len[board_quan][2],"gpio");
      board_model[board_quan]=GPIO_PCI_DIO24_DEVICE_ID;
      strcpy(boards[board_quan].name,"MeasurementComputing CIO-DIO24");
      boards[board_quan].mode=GPIO_MODE_INPUT;
      boards[board_quan].inputs=24;
      boards[board_quan].outputs=0;
      boards[board_quan].samples=0;
      boards[board_quan].depth=0;
      boards[board_quan].caps=GPIO_CAP_MODE;
      init_waitqueue_head(&board_queue[board_quan]);
      board_quan++; 
    }
  }

  //
  // PCI-DIO24
  //
  while(((dev=pci_find_device(GPIO_CBOARDS_VENDOR_ID,
			     GPIO_PCI_DIO24_DEVICE_ID,dev))!=NULL)&&
	(board_quan<GPIO_MAX_CARDS)) {
    board_open_count[board_quan]=0;
    for(i=0;i<6;i++) {
      board_io[board_quan][i]=pci_resource_start(dev,i);
      board_len[board_quan][i]=pci_resource_len(dev,i);
      request_region(board_io[board_quan][i],board_len[board_quan][i],"gpio");
    }
    board_model[board_quan]=GPIO_PCI_DIO24_DEVICE_ID;
    strcpy(boards[board_quan].name,"MeasurementComputing PCI-DIO24");
    boards[board_quan].mode=GPIO_MODE_INPUT;
    boards[board_quan].inputs=24;
    boards[board_quan].outputs=0;
    boards[board_quan].samples=0;
    boards[board_quan].depth=0;
    boards[board_quan].caps=GPIO_CAP_MODE;
    init_waitqueue_head(&board_queue[board_quan]);
    board_quan++;
  }

  //
  // PCI-DIO24H
  //
  while(((dev=pci_find_device(GPIO_CBOARDS_VENDOR_ID,
			     GPIO_PCI_DIO24H_DEVICE_ID,dev))!=NULL)&&
	(board_quan<GPIO_MAX_CARDS)) {
    board_open_count[board_quan]=0;
    for(i=0;i<6;i++) {
      board_io[board_quan][i]=pci_resource_start(dev,i);
      board_len[board_quan][i]=pci_resource_len(dev,i);
      request_region(board_io[board_quan][i],board_len[board_quan][i],"gpio");
    }
    board_model[board_quan]=GPIO_PCI_DIO24H_DEVICE_ID;
    strcpy(boards[board_quan].name,"MeasurementComputing PCI-DIO24H");
    boards[board_quan].mode=GPIO_MODE_INPUT;
    boards[board_quan].inputs=24;
    boards[board_quan].outputs=0;
    boards[board_quan].samples=0;
    boards[board_quan].depth=0;
    boards[board_quan].caps=GPIO_CAP_MODE;
    init_waitqueue_head(&board_queue[board_quan]);
    board_quan++;
  }

  //
  // PCI-PDISO8
  //
  dev=NULL;
  while(((dev=pci_find_device(GPIO_CBOARDS_VENDOR_ID,
			     GPIO_PCI_PDISO8_DEVICE_ID,dev))!=NULL)&&
	(board_quan<GPIO_MAX_CARDS)) {
    board_open_count[board_quan]=0;
    for(i=0;i<6;i++) {
      board_io[board_quan][i]=pci_resource_start(dev,i);
      board_len[board_quan][i]=pci_resource_len(dev,i);
      request_region(board_io[board_quan][i],board_len[board_quan][i],"gpio");
    }
//    base_io[board_quan]=pci_resource_start(dev,1);
    board_model[board_quan]=GPIO_PCI_PDISO8_DEVICE_ID;
    strcpy(boards[board_quan].name,"MeasurementComputing PCI-PDISO8");
    boards[board_quan].mode=GPIO_MODE_AUTO;
    boards[board_quan].inputs=8;
    boards[board_quan].outputs=8;
    boards[board_quan].samples=0;
    boards[board_quan].depth=0;
    boards[board_quan].caps=GPIO_CAP_FILTER;
    init_waitqueue_head(&board_queue[board_quan]);
    board_quan++;
  }

  //
  // PCI-PDISO16
  //
  dev=NULL;
  while(((dev=pci_find_device(GPIO_CBOARDS_VENDOR_ID,
			     GPIO_PCI_PDISO16_DEVICE_ID,dev))!=NULL)&&
	(board_quan<GPIO_MAX_CARDS)) {
    board_open_count[board_quan]=0;
    for(i=0;i<6;i++) {
      board_io[board_quan][i]=pci_resource_start(dev,i);
      board_len[board_quan][i]=pci_resource_len(dev,i);
      request_region(board_io[board_quan][i],board_len[board_quan][i],"gpio");
    }
//    base_io[board_quan]=pci_resource_start(dev,1);
    board_model[board_quan]=GPIO_PCI_PDISO16_DEVICE_ID;
    strcpy(boards[board_quan].name,"MeasurementComputing PCI-PDISO16");
    boards[board_quan].mode=GPIO_MODE_AUTO;
    boards[board_quan].inputs=16;
    boards[board_quan].outputs=16;
    boards[board_quan].samples=0;
    boards[board_quan].depth=0;
    boards[board_quan].caps=GPIO_CAP_FILTER;
    init_waitqueue_head(&board_queue[board_quan]);
    board_quan++;
  }

  //
  // PCI-DAS1000
  //
  dev=NULL;
  while(((dev=pci_find_device(GPIO_CBOARDS_VENDOR_ID,
			     GPIO_PCI_DAS1000_DEVICE_ID,dev))!=NULL)&&
	(board_quan<GPIO_MAX_CARDS)) {
    board_open_count[board_quan]=0;
    for(i=0;i<6;i++) {
      board_io[board_quan][i]=pci_resource_start(dev,i);
      board_len[board_quan][i]=pci_resource_len(dev,i);
      request_region(board_io[board_quan][i],board_len[board_quan][i],"gpio");
    }
//    base_io[board_quan]=pci_resource_start(dev,3);
    pci_read_config_byte(dev,PCI_INTERRUPT_LINE,&irq);
    board_irq[board_quan]=(long)irq;
    board_model[board_quan]=GPIO_PCI_DAS1000_DEVICE_ID;
    strcpy(boards[board_quan].name,"MeasurementComputing PCI-DAS1000");
    boards[board_quan].mode=GPIO_MODE_AUTO;
    boards[board_quan].inputs=8;
    boards[board_quan].outputs=16;
    boards[board_quan].samples=8;
    boards[board_quan].depth=12;
    boards[board_quan].caps=0;
    outb(0x90,board_io[board_quan][3]+7);
    init_waitqueue_head(&board_queue[board_quan]);
    board_quan++;
  }
}


int init_module(void)
{
  struct gpio_mask mask;
  unsigned i,j;
  char name[20];
  
#ifdef KERNEL_2_4
  EXPORT_NO_SYMBOLS;
#endif  // KERNEL_2_4

  board_type[0]=board_type_0;
  board_port[0]=board_port_0;
  board_type[1]=board_type_1;
  board_port[1]=board_port_1;
  board_type[2]=board_type_2;
  board_port[2]=board_port_2;
  board_type[3]=board_type_3;
  board_port[3]=board_port_3;
  board_type[4]=board_type_4;
  board_port[4]=board_port_4;
  board_type[5]=board_type_5;
  board_port[5]=board_port_5;
  board_type[6]=board_type_6;
  board_port[6]=board_port_6;
  board_type[7]=board_type_7;
  board_port[7]=board_port_7;

  gpio_config_boards();

  if(board_quan==0) {
    return -ENODEV;
  }
  if((gpio_major=register_chrdev(0,"gpio",&gpio_fops))<0) {
    for(i=0;i<board_quan;i++) {
      for(j=0;j<6;j++) {
	release_region(board_io[i][j],board_len[i][j]);
      }
    }
    return -EBUSY;
  }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 14)
  gpio_class=class_create(THIS_MODULE,"gpio");
  if(IS_ERR(gpio_class)) {
    printk(KERN_ERR "Error creating gpio class.\n");
    unregister_chrdev(gpio_major,"gpio");
    for(i=0;i<board_quan;i++) {
      for(j=0;j<6;j++) {
	release_region(board_io[i][j],board_len[i][j]);
      }
    }
    return -EBUSY;
  } 
#endif
  for(i=0;i<board_quan;i++) {
    memset(&mask,0,sizeof(struct gpio_mask));
    gpio_set_outputs(&mask,i);
    gpio_set_filters(&mask,i);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)
    device_create(gpio_class,NULL,MKDEV(gpio_major,i),NULL, "gpio%d",i);
#elif  LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 26)
    device_create(gpio_class,NULL,MKDEV(gpio_major,i),"gpio%i",i);
#elif  LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 14)
    class_device_create(gpio_class,NULL,MKDEV(gpio_major,i),NULL,"gpio%d",i);
#endif
  }
  proc_mkdir("driver/gpio",NULL);
  for(i=0;i<board_quan;i++) {
    sprintf(name,"driver/gpio/card%d",i);
    create_proc_read_entry(name,0,NULL,gpio_read_proc,(int *)i);
  }

  return 0;
}


void cleanup_module(void)
{
  unsigned i,j;
  char name[20];

  for(i=0;i<board_quan;i++) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 26)
    device_destroy(gpio_class,MKDEV(gpio_major,i));
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 14)
    class_device_destroy(gpio_class,MKDEV(gpio_major,i));
#endif
    //class_device_destroy(gpio_class,MKDEV(gpio_major,i));
    sprintf(name,"driver/gpio/card%d",i);
    remove_proc_entry(name,NULL);
    for(j=0;j<6;j++) {
      if(board_io[i][j]!=0) {
	release_region(board_io[i][j],board_len[i][j]);
      }
    }
  }
  remove_proc_entry("driver/gpio",NULL);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 14)
  class_destroy(gpio_class);
#endif
  unregister_chrdev(gpio_major,"gpio");
}


MODULE_AUTHOR("Fred Gleason <fredg@paravelsystems.com>");
MODULE_DESCRIPTION("A driver for MeasurementComputing GPIO cards");
MODULE_LICENSE("GPL");

#ifdef KERNEL_2_4
  MODULE_PARM(board_type_0,"h");
  MODULE_PARM(board_port_0,"h");
#else
  module_param(board_type_0,ushort,S_IRUGO);
  module_param(board_port_0,ushort,S_IRUGO);
#endif  // KERNEL_2_4
MODULE_PARM_DESC(board_type_0,"Type of GPIO board (default 0 = autodetect)");
MODULE_PARM_DESC(board_port_0,"I/O port of board (default is 0x0000)");

#ifdef KERNEL_2_4
  MODULE_PARM(board_type_1,"h");
  MODULE_PARM(board_port_1,"h");
#else
  module_param(board_type_1,ushort,S_IRUGO);
  module_param(board_port_1,ushort,S_IRUGO);
#endif  // KERNEL_2_4
MODULE_PARM_DESC(board_type_1,"Type of GPIO board (default 0 = autodetect)");
MODULE_PARM_DESC(board_port_1,"I/O port of board (default is 0x0000)");

#ifdef KERNEL_2_4
  MODULE_PARM(board_type_2,"h");
  MODULE_PARM(board_port_2,"h");
#else
  module_param(board_type_2,ushort,S_IRUGO);
  module_param(board_port_2,ushort,S_IRUGO);
#endif  // KERNEL_2_4
MODULE_PARM_DESC(board_type_2,"Type of GPIO board (default 0 = autodetect)");
MODULE_PARM_DESC(board_port_2,"I/O port of board (default is 0x0000)");

#ifdef KERNEL_2_4
  MODULE_PARM(board_type_3,"h");
  MODULE_PARM(board_port_3,"h");
#else
  module_param(board_type_3,ushort,S_IRUGO);
  module_param(board_port_3,ushort,S_IRUGO);
#endif  // KERNEL_2_4
MODULE_PARM_DESC(board_type_3,"Type of GPIO board (default 0 = autodetect)");
MODULE_PARM_DESC(board_port_3,"I/O port of board (default is 0x0000)");

#ifdef KERNEL_2_4
  MODULE_PARM(board_type_4,"h");
  MODULE_PARM(board_port_4,"h");
#else
  module_param(board_type_4,ushort,S_IRUGO);
  module_param(board_port_4,ushort,S_IRUGO);
#endif  // KERNEL_2_4
MODULE_PARM_DESC(board_type_4,"Type of GPIO board (default 0 = autodetect)");
MODULE_PARM_DESC(board_port_4,"I/O port of board (default is 0x0000)");

#ifdef KERNEL_2_4
  MODULE_PARM(board_type_5,"h");
  MODULE_PARM(board_port_5,"h");
#else
  module_param(board_type_5,ushort,S_IRUGO);
  module_param(board_port_5,ushort,S_IRUGO);
#endif  // KERNEL_2_4
MODULE_PARM_DESC(board_type_5,"Type of GPIO board (default 0 = autodetect)");
MODULE_PARM_DESC(board_port_5,"I/O port of board (default is 0x0000)");

#ifdef KERNEL_2_4
  MODULE_PARM(board_type_6,"h");
  MODULE_PARM(board_port_6,"h");
#else
  module_param(board_type_6,ushort,S_IRUGO);
  module_param(board_port_6,ushort,S_IRUGO);
#endif  // KERNEL_2_4
MODULE_PARM_DESC(board_type_6,"Type of GPIO board (default 0 = autodetect)");
MODULE_PARM_DESC(board_port_6,"I/O port of board (default is 0x0000)");

#ifdef KERNEL_2_4
  MODULE_PARM(board_type_7,"h");
  MODULE_PARM(board_port_7,"h");
#else
  module_param(board_type_7,ushort,S_IRUGO);
  module_param(board_port_7,ushort,S_IRUGO);
#endif  // KERNEL_2_4
MODULE_PARM_DESC(board_type_7,"Type of GPIO board (default 0 = autodetect)");
MODULE_PARM_DESC(board_port_7,"I/O port of board (default is 0x0000)");

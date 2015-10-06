//   rgpio.h
//
//   A driver for General-Purpose I/O devices.
//
//   (C) Copyright 2002-2007 Fred Gleason <fredg@paravelsystems.com>
//
//    $Id: rgpio.h,v 1.1 2007/11/07 20:36:32 fredg Exp $
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Library General Public License 
//   version 2 as published by the Free Software Foundation.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public
//   License along with this program; if not, write to the Free Software
//   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//

#ifndef RGPIO_H
#define RGPIO_H

#include <gpio.h>
#include <linux/input.h>

#include <qobject.h>
#include <qtimer.h>
#include <qsignalmapper.h>

#define GPIO_CLOCK_INTERVAL 100
#define GPIO_MAX_LINES 24

/**
 * @short A driver for Genral Purpose I/O Devices
 * @author Fred Gleason <fredg@wava.com>
 * 
 * This class implements a driver for a GPIO device.  
 **/
class RGpio : public QObject
{
 Q_OBJECT
 public:
 /**
  * Board Mode
  **/
 enum Mode {Auto=GPIO_MODE_AUTO,Input=GPIO_MODE_INPUT,Output=GPIO_MODE_OUTPUT};

 /**
  * Instantiates the widget.
  * @param mode The mode of the board.  Needed only for the DIO24.
  **/
  RGpio(QObject *parent=0,const char *name=0);

 /**
  * Get the device name.
  * Returns: The device name.
  **/
  QString device() const;

 /**
  * Set the device name.
  * @param dev = The device name.
  **/
  void setDevice(QString dev);


 /**
  * Get the device description
  **/
  QString description() const;

 /**
  * Get the device mode.
  * Returns: The device mode.
  **/
  RGpio::Mode mode();

 /**
  * Set the device mode.
  * @param mode = The device mode.
  **/
  void setMode(RGpio::Mode mode);

 /**
  * Attempt to open the device.
  * Returns:  True = success, false = failure.
  **/
  bool open();

 /**
  * Close the device.
  **/
  void close();

 /**
  * Returns the number of available input (GPI) lines available.
  **/
  int inputs() const;

 /**
  * Returns the number of available output (GPO) lines available.
  **/
  int outputs() const;

 /**
  * Returns the current input mask
  **/
  unsigned inputMask();

 /**
  * Returns the current state of the indicated GPI line
  * @param line Line number
  **/
  bool inputState(int line);

 /**
  * Returns the current output mask
  **/
  unsigned outputMask() const;


 signals:
  /** 
   * Emitted upon a change to an input (GPI) line
   * @param pin The number of the GPI line
   * @param state True = Line set, false = line clear
   **/
  void inputChanged(int line,bool state);

  /** 
   * Emitted upon a change to an output (GPO) line
   * @param pin The number of the GPO line
   * @param state True = Line set, false = line clear
   **/
  void outputChanged(int line,bool state);

 public slots:
 /**
  * Set (turn on) a given output (GPO) line.
  * @param line The line to set.
  * @param interval The length of time to hold the new state (in mS).  
  * 0 = hold new state indefinetly.
  **/
  void gpoSet(int line,unsigned interval=0);

 /**
  * Reset (turn off) a given output (GPO) line.
  * @param line The line to clear.
  * @param interval The length of time to hold the new state (in mS).  
  * 0 = hold new state indefinetly.
  **/
  void gpoReset(int line,unsigned interval=0);

 private slots:
  void inputTimerData();
  void revertData(int);

 private:
  enum Api {ApiGpio=0,ApiInput=1};
  void RemapTimers();
  void SetReversion(int,unsigned);
  void Clear();
  void InitGpio();
  void InitInput();
  Api gpio_api;
  int gpio_fd;
  QString gpio_device;
  bool gpio_open;
  struct gpio_info gpio_info;
  QTimer *gpio_input_timer;
  unsigned gpio_input_mask;
  unsigned gpio_output_mask;
  QSignalMapper *gpio_revert_mapper;
  QTimer *gpio_revert_timer[GPIO_MAX_LINES];
  int gpio_key_map[KEY_MAX];
  QString gpio_description;
};


#endif

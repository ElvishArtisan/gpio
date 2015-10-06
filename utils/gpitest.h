// gpitest.h
//
// A Qt-based application for testing general purpose input (GPI) devices.
//
//   (C) Copyright 2002-2007 Fred Gleason <fredg@paravelsystems.com>
//
//    $Id: gpitest.h,v 1.1 2007/11/07 20:36:32 fredg Exp $
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


#ifndef GPITEST_H
#define GPITEST_H

#include <qwidget.h>
#include <qsize.h>
#include <qsizepolicy.h>
#include <qpushbutton.h>
#include <qcolor.h>
#include <qstring.h>
#include <qsignalmapper.h>
#include <qlabel.h>

#include <rgpio.h>

//
// Widget Settings
//
#define MAX_GPIO_PINS 24

class MainWidget : public QWidget
{
  Q_OBJECT
  public:
   MainWidget(QWidget *parent=0,const char *name=0);
   ~MainWidget();
   QSize sizeHint() const;
   QSizePolicy sizePolicy() const;

  protected:

  private slots:
   void inputChangedData(int line,bool state);
   void quitMainWidget();

  private:
   RGpio *gpi_gpio;
   unsigned short gpi_port;
   QLabel *gpi_label[MAX_GPIO_PINS];
   bool gpi_state[MAX_GPIO_PINS];
};


#endif 

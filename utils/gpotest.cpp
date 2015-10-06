// gpotest.cpp
//
// A Qt-based application for testing General Purpose Outputs (GPO).
//
//   (C) Copyright 2002-2007 Fred Gleason <fredg@paravelsystems.com>
//
//    $Id: gpotest.cpp,v 1.1 2007/11/07 20:36:32 fredg Exp $
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


#include <qapplication.h>
#include <qwidget.h>
#include <qpushbutton.h>
#include <qrect.h>
#include <qpoint.h>
#include <qpainter.h>
#include <qstring.h>
#include <qmessagebox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qsignalmapper.h>
#include <qtextcodec.h>
#include <qtranslator.h>

#include <rconf.h>
#include <gpotest.h>

//
// Global Classes
//
MainWidget::MainWidget(QWidget *parent,const char *name)
  :QWidget(parent,name)
{
  QFont font;

  font=QFont("Helvetica",12,QFont::Bold);
  font.setPixelSize(12);

  setCaption(tr("GPOTest"));
  for(int i=0;i<MAX_GPIO_PINS;i++) {
    gpo_state[i]=false;
  }
  gpo_gpio=new RGpio(this,"gpo_gpio");
  if(qApp->argc()>1) {
    gpo_gpio->setDevice(qApp->argv()[qApp->argc()-1]);
  }
  else {
    gpo_gpio->setDevice("/dev/gpio0");
  }
  if(!gpo_gpio->open()) {
    QMessageBox::warning(this,tr("GPOTest"),
			 tr("Unable to open GPIO device."));
    exit(1);
  }
  connect(gpo_gpio,SIGNAL(outputChanged(int,bool)),
	  this,SLOT(outputChangedData(int,bool)));

  //
  // Set Device Mode
  //
  gpo_gpio->setMode(RGpio::Output);

  //
  // GPIO Device
  //
  QLineEdit *line_edit=new QLineEdit(this,"device");
  line_edit->setGeometry(80,10,120,21);
  line_edit->setReadOnly(true);
  line_edit->setText(gpo_gpio->device());
  QLabel *label=new QLabel(line_edit,tr("Device:"),this,"device_label");
  label->setGeometry(20,10,55,21);
  label->setFont(font);
  label->setAlignment(AlignRight|AlignVCenter);

  //
  // Relay Buttons
  //
  gpo_mapper=new QSignalMapper(this,"gpo_mapper");
  connect(gpo_mapper,SIGNAL(mapped(int)),this,SLOT(buttonData(int)));
  for(int i=0;i<(gpo_gpio->outputs()/8);i++) {
    for(int j=0;j<8;j++) {
      gpo_button[8*i+j]=new QPushButton(this);
      gpo_button[8*i+j]->setGeometry(10+48*j,40+48*i,43,43);
      gpo_button[8*i+j]->setText(QString().sprintf("%d",8*i+j+1));
      gpo_mapper->setMapping(gpo_button[8*i+j],8*i+j);
      connect(gpo_button[8*i+j],SIGNAL(clicked()),gpo_mapper,SLOT(map()));
    }
  }

  //
  // Relay Ops Box
  //
  gpo_latch_box=new QComboBox(this,"gpo_latch_box");
  gpo_latch_box->setFont(font);
  gpo_latch_box->setGeometry(145,sizeHint().height()-50,110,20);
  gpo_latch_box->insertItem(tr("Latched"));
  gpo_latch_box->insertItem(tr("Momentary"));
  label=new QLabel(gpo_latch_box,tr("Relay Operation:"),
		   this,"gpo_latch_label");
  label->setGeometry(10,sizeHint().height()-50,130,20);
  label->setFont(font);
  label->setAlignment(AlignRight|AlignVCenter);

  //
  // Close Button
  //
  QPushButton *close_button=new QPushButton(this,"close_button");
  close_button->setGeometry(310,sizeHint().height()-60,80,50);
  close_button->setFont(font);
  close_button->setText(tr("&Close"));
  connect(close_button,SIGNAL(clicked()),this,SLOT(quitMainWidget()));
}


MainWidget::~MainWidget()
{
}


QSize MainWidget::sizeHint() const
{
  return QSize(400,50+6*gpo_gpio->outputs()+60);
}


QSizePolicy MainWidget::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}


void MainWidget::buttonData(int id)
{
  if(gpo_state[id]) {
    gpo_state[id]=false;
    gpo_gpio->gpoReset(id);
  }
  else {
    gpo_state[id]=true;
    gpo_gpio->gpoSet(id,1000*gpo_latch_box->currentItem());
  }
}


void MainWidget::outputChangedData(int id,bool state)
{
  gpo_state[id]=state;
  if(state) {
    gpo_button[id]->setPaletteBackgroundColor(QColor(green));
  }
  else {
    gpo_button[id]->setPaletteBackgroundColor(backgroundColor());
  }
}


void MainWidget::quitMainWidget()
{
  exit(0);
}


int main(int argc,char *argv[])
{
  QApplication a(argc,argv);
  
  //
  // Load Translations
  //
  QTranslator qt(0);
  qt.load(QString(QTDIR)+QString("/translations/qt_")+QTextCodec::locale(),
	  ".");
  a.installTranslator(&qt);

  QTranslator libradio(0);
  libradio.load(QString(PREFIX)+QString("/share/srlabs/libradio_")+
		QTextCodec::locale(),".");
  a.installTranslator(&libradio);

  QTranslator tests(0);
  tests.load(QString(PREFIX)+QString("/share/srlabs/libradio_tests_")+
	     QTextCodec::locale(),".");
  a.installTranslator(&tests);

  //
  // Start Event Loop
  //
  MainWidget *w=new MainWidget(NULL,"main");
  a.setMainWidget(w);
  w->setGeometry(QRect(QPoint(0,0),w->sizeHint()));
  w->show();
  return a.exec();
}



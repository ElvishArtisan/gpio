// gpitest.cpp
//
// A Qt-based application for testing General Purpose Input (GPI) devices.
//
//   (C) Copyright 2002-2007 Fred Gleason <fredg@paravelsystems.com>
//
//    $Id: gpitest.cpp,v 1.1 2007/11/07 20:36:32 fredg Exp $
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
#include <qtextcodec.h>
#include <qtranslator.h>

#include <rconf.h>
#include <gpitest.h>


//
// Global Classes
//
MainWidget::MainWidget(QWidget *parent,const char *name)
  :QWidget(parent,name)
{
  unsigned mask=0;
  unsigned offset;
  QFont font;

  //
  // Generate Font
  //
  font=QFont("Helvetica",12,QFont::Bold);
  font.setPixelSize(12);

  setCaption(tr("GPITest"));
  for(int i=0;i<MAX_GPIO_PINS;i++) {
    gpi_state[i]=false;
  }
  gpi_gpio=new RGpio(this,"gpi_gpio");
  if(qApp->argc()>1) {
    gpi_gpio->setDevice(qApp->argv()[qApp->argc()-1]);
  }
  else {
    gpi_gpio->setDevice("/dev/gpio0");
  }
  if(!gpi_gpio->open()) {
    QMessageBox::warning(this,tr("GPITest"),
			 tr("Unable to open GPIO device."));
    exit(1);
  }
  connect(gpi_gpio,SIGNAL(inputChanged(int,bool)),
	  this,SLOT(inputChangedData(int,bool)));

  //
  // Set Device Mode
  //
  gpi_gpio->setMode(RGpio::Input);

  //
  // Device Name
  //
  QLineEdit *line_edit=new QLineEdit(this,"device");
  line_edit->setGeometry(120,7,120,21);
  line_edit->setReadOnly(true);
  line_edit->setText(gpi_gpio->device());
  QLabel *label=new QLabel(line_edit,tr("Device:"),this,"device_label");
  label->setGeometry(10,7,105,21);
  label->setFont(font);
  label->setAlignment(AlignRight|AlignVCenter);

  //
  // Device Description
  //
  line_edit=new QLineEdit(this,"description");
  line_edit->setGeometry(120,30,sizeHint().width()-140,21);
  line_edit->setReadOnly(true);
  line_edit->setText(gpi_gpio->description());
  label=new QLabel(line_edit,tr("Description:"),this,"description_label");
  label->setGeometry(10,30,105,21);
  label->setFont(font);
  label->setAlignment(AlignRight|AlignVCenter);

  //
  // Input Indicators
  //
  mask=gpi_gpio->inputMask();
  for(int i=0;i<(gpi_gpio->inputs()/8);i++) {
    for(int j=0;j<8;j++) {
      gpi_label[8*i+j]=new QLabel(QString().sprintf("%d",8*i+j+1),this);
      gpi_label[8*i+j]->setGeometry(10+48*j,60+48*i,43,43);
      gpi_label[8*i+j]->setAlignment(AlignHCenter|AlignVCenter);
      if(((1>>(8*i+j))&mask)==0) {
	gpi_label[8*i+j]->setPalette(QColor(gray));
      }
      else {
	gpi_label[8*i+j]->setPalette(QColor(green));
      }
    }
  }
  if(8*(gpi_gpio->inputs()/8)<gpi_gpio->inputs()) {
    offset=gpi_gpio->inputs()/8;
    for(int j=0;j<(gpi_gpio->inputs()-8*(gpi_gpio->inputs()/8));j++) {
      gpi_label[8*offset+j]=
	new QLabel(QString().sprintf("%d",8*offset+j+1),this);
      gpi_label[8*offset+j]->setGeometry(10+48*j,60+48*offset,43,43);
      gpi_label[8*offset+j]->setAlignment(AlignHCenter|AlignVCenter);
      if(((1>>(8*offset+j))&mask)==0) {
	gpi_label[8*offset+j]->setPalette(QColor(gray));
      }
      else {
	gpi_label[8*offset+j]->setPalette(QColor(green));
      }
    }
  }

  //
  // Close Button
  //
  QPushButton *close_button=new QPushButton(this,"close_button");
  close_button->setGeometry(sizeHint().width()-90,sizeHint().height()-60,
			    80,50);
  close_button->setFont(font);
  close_button->setText(tr("&Close"));
  connect(close_button,SIGNAL(clicked()),this,SLOT(quitMainWidget()));
}


MainWidget::~MainWidget()
{
}


QSize MainWidget::sizeHint() const
{
  return QSize(400,130+6*gpi_gpio->inputs());
}


QSizePolicy MainWidget::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}


void MainWidget::inputChangedData(int line,bool state)
{
  if(state) {
    gpi_label[line]->setPalette(QColor(green));
  }
  else {
    gpi_label[line]->setPalette(QColor(gray));
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

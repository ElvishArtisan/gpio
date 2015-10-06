// rconf.h
//
// The header file for the rconf package
//
//   (C) Copyright 1996-2007 Fred Gleason <fredg@paravelsystems.com>
//
//    $Id: rconf.h,v 1.2 2011/09/16 21:20:26 cvs Exp $
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

#ifndef RCONF_H
#define RCONF_H

#define MAX_RETRIES 10
#include <qstring.h>
#include <qdatetime.h>
#include <qfont.h>
#include <qhostaddress.h>
#include <qsqldatabase.h>
#include <qvariant.h>
#include <stdlib.h>
#include <stdio.h>

/* Function Prototypes */
int GetPrivateProfileBool(const char *,const char *,const char *,bool);
int GetPrivateProfileString(const char *,const char *,const char *,char *,
			    const char *,int);
int GetPrivateProfileInt(const char *,const char *,const char *,int);
int GetPrivateProfileHex(const char *,const char *,const char *,int);
double GetPrivateProfileDouble(const char *,const char *,const char *,double);
int GetIni(const char *,const char *,const char *,char *,int);
int GetIniLine(FILE *,char *);
void Prepend(char *,char *);
int IncrementIndex(char *,int);
void StripLevel(char *); 
bool GetLock(const char *);
void ClearLock(const char *);
QString RGetPathPart(QString path);
QString RGetBasePart(QString path);
QString RGetShortDate(QDate);
/**
 * Returns the name of the weekday in english regardless of the locale
 * configured.
 * @param weekday Integer value for the weekday; 1 = "Mon", 2 = "Tue", 
 * ... 7 = "Sun".  If the value is out of range 1 is defaulted to.
 **/
QString RGetShortDayNameEN(int weekday);
QFont::Weight RGetFontWeight(QString);
bool RDetach();
bool RBool(QString);
QString RYesNo(bool);
QHostAddress RGetHostAddr();
QString RGetDisplay(bool strip_point=false);
bool RDoesRowExist(QString table,QString name,QString test,
		   QSqlDatabase *db=0);
bool RDoesRowExist(QString table,QString name,unsigned test,
		   QSqlDatabase *db=0);
QVariant RGetSqlValue(QString table,QString name,QString test,
		      QString param,QSqlDatabase *db=0,bool *valid=0);
QVariant RGetSqlValue(QString table,QString name,unsigned test,
		      QString param,QSqlDatabase *db=0,bool *valid=0);
bool RIsSqlNull(QString table,QString name,QString test,
		QString param,QSqlDatabase *db=0);
bool RIsSqlNull(QString table,QString name,unsigned test,
		QString param,QSqlDatabase *db=0);
QString RGetTimeLength(int mseconds,bool leadzero=false,bool tenths=true);
int RSetTimeLength(QString string);
bool RCopy(QString srcfile,QString destfile);
#ifndef WIN32
bool RWritePid(QString dirname,QString filename,int owner=-1,int group=-1);
void RDeletePid(QString dirname,QString filename);
bool RCheckPid(QString dirname,QString filename);
pid_t RGetPid(QString pidfile);
#endif  // WIN32
QString RGetHomeDir(bool *found=0);
QString RTruncateAfterWord(QString str,int word,bool add_dots=false);
QString RHomeDir();
QString RTempDir();

#endif   // RCONF_H

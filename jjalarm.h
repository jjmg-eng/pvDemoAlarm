#ifndef JJALARM_H
#define JJALARM_H
//#define _MAIN_
// GLOBAL
typedef struct
{
    int state;
    int na;
}
MyDATA;

/*
 * inserir em pvmain():
 *
 * MyDATA u;
 * p->user = &u;
 *
 */

#include "alarm.h"


#ifndef _MAIN_
// Masks
extern Alarm myalarm;
extern int AlarmWord[];
extern int AlarmNotAck[];
extern int NewAlarm,AudibleAlarm;
extern int MAX_ALARMS;
int writeAlarm(int alm, int bit);
int readAlarm(int alm);
void alarmInit(PARAM *p, int idGroupBox, int idHTML, int idDOCK);
void alarmNullEvent(PARAM *p, int idHTML, int idDOCK);
void alarmTextEvent(const char *text);

#else
// Seção main.cpp

//Alarm myalarm(1);   // with event log
Alarm myalarm;        // without event log
rlThread thread;
int NewAlarm=0,AudibleAlarm;
int AlarmWord[]   = {0,0,0,0,0,0,0,0,0,0};
int AlarmWord0[]  = {0,0,0,0,0,0,0,0,0,0};
int AlarmNotAck[] = {0,0,0,0,0,0,0,0,0,0};
int AlarmNotAck0[]= {0,0,0,0,0,0,0,0,0,0};
int nWords = sizeof(AlarmWord)/sizeof(int);
int MAX_ALARMS = nWords * 32;

int writeAlarm(int alm, int bit)
{
    if(bit) AlarmWord[alm / 32] |= 1 << (alm % 32);
    else    AlarmWord[alm / 32] &= ~(1 << (alm % 32));
    return !!bit;
}

int readAlarm(int alm)
{
    return !!(AlarmWord[alm / 32] & 1 << (alm % 32));
}

static void updateAlarm()
{
  int a,i,j,SetAlarm,ResetAlarm;
  char buf[100];
  for(j = 0; j< nWords; j++){
      SetAlarm    = (AlarmWord[j]^AlarmWord0[j]) & AlarmWord[j];
      if(SetAlarm)
          for(i=0; i<32; i++)
              if(SetAlarm & 1<<i)
              {
                  AlarmNotAck[j] |= 1<<i;
                  AudibleAlarm=NewAlarm=1;
                  a = j*32+i+1;
                  sprintf(buf,"A%d",a);
                  myalarm.set(buf);
                  puts("SET");
                  puts(myalarm.table.text(2,a));
              }
      ResetAlarm    = (AlarmWord[j]^AlarmWord0[j]) & AlarmWord0[j];
      if(ResetAlarm)
          for(i=0; i<32; i++)
              if(ResetAlarm & 1<<i)
              {
                  a = j*32+i+1;
                  puts("RESET");
                  puts(myalarm.table.text(2,a));
              }
      ResetAlarm  = ((AlarmWord[j] | AlarmNotAck[j]) ^ AlarmWord0[j]) & AlarmWord0[j];
      ResetAlarm |= (AlarmNotAck[j] ^ AlarmNotAck0[j]) & ~AlarmWord[j];
      if(ResetAlarm)
          for(i=0; i<32; i++)
              if(ResetAlarm & 1<<i)
              {
                  a = j*32+i+1;
                  sprintf(buf,"A%d",a);
                  myalarm.reset(buf);
              }

      AlarmWord0[j]   = AlarmWord[j];
      AlarmNotAck0[j] = AlarmNotAck[j];
  }
}


void alarmInit(PARAM *p, int idGroupBox, int idHTML, int idDOCK)
{
    MyDATA *u = (MyDATA *) p->user;
    u->state = -1;
    u->na=0;

    pvSetStyleSheet(p,idGroupBox,"QGroupBox {background-color: rgba(0,0,0,0); border:0; qproperty-flat:true;}");
    pvSetGeometry(p,idGroupBox,25,20,580,144);
    pvSetFont(p,idGroupBox,"Arial",11,0,0,0,0);
    pvSetGeometry(p,idHTML,5,1,570,110);
    pvSetFont(p,idHTML,"Arial",10,0,0,0,0);

    pvAddDockWidget(p, pvtr("ALARMES/EVENTOS"), idDOCK, idGroupBox, 1,1,0,0,0,0);
    pvDownloadFile(p,"/home/jjmg/PVprojects/sounds/alarm.wav");
}

void alarmNullEvent(PARAM *p, int idHTML, int idDOCK)
{
    MyDATA *u = (MyDATA *)p->user;

    myalarm.updateWidgetHTML(p,idHTML,&u->state);

    if(NewAlarm && u->na == 0)
    {
        u->na=5;
        pvShow(p,idDOCK);
    }
    if(u->na == 1) NewAlarm = 0;
    if(u->na) u->na--;

    if(myalarm.count() > 0)
    {
      char buf[80];
      sprintf(buf, "There are %d alarms pending", myalarm.count());
      pvStatusMessage(p,255,0,0,buf);
    }
    else
    {
      pvStatusMessage(p,0,255,0,"Normal operation");
      pvHide(p,idDOCK);
    }
    myalarm.ackblk = 0;
}

void alarmTextEvent(const char *text)
{
	if(!myalarm.ackblk) {
	  myalarm.ackblk = 1;
	  int a = (atoi(strrchr(text,'a')+1) - 1);
	  AlarmNotAck[a / 32] &= ~ (1 << (a % 32));
	  a++;
	  puts("ACK");
	  puts(myalarm.table.text(2,a));
	  myalarm.ack(text);
	}
}

#endif // _MAIN_

#endif // JJALARM_H

//***************************************************************************
//                          pvapp.h  -  description
//                             -------------------
//  begin            : ter mai 7 07:25:59 2024
//  generated by     : pvdevelop (C) Lehrig Software Engineering
//  email            : lehrig@t-online.de
//***************************************************************************
#ifndef _PVAPP_H_
#define _PVAPP_H_

#ifdef USE_INETD
static int trace=0; // todo: set trace=0 if you do not want printf() within event loop
#else
static int trace=1; // todo: set trace=0 if you do not want printf() within event loop
#endif

#include "processviewserver.h"
// todo: comment me out
#include "rlmodbusclient.h"
//#include "rlsiemenstcpclient.h"
//#include "rlppiclient.h"
#include "modbusdaemon.h"             // this is generated. Change for name.h -> "name.mkmodbus"
//#include "siemensdaemon.h"            // this is generated
//#include "ppidaemon.h"                // this is generated

int show_mask1(PARAM *p);

//***************************************************************************
//                                 mainloop.h
//                                 ----------
//  O código a seguir foi incluído diretamente em "pvapp.h" por esta ser uma 
//  aplicação pequena.
//  Em projetos maiores, será mais adequado que seja criado um arquivo a parte
//  com o nome "mainloop.h".
//  O objetivo de mainloop é criar uma thread que permita executar lógicas 
//  independentes das masks usando o estilo dos programas para Arduino com
//  setup() e loop().
// 
#ifndef _MAINLOOP_
#define _MAINLOOP_

// GLOBAL

#include "stdlib.h"
#include "rlthread.h"
#include "rltime.h"
#include "qtdatabase.h"
#include "jjalarm.h"

/////////////////////////////////////////////
#ifndef _MAIN_  // Código exclusivo das Masks
/////////////////////////////////////////////
extern rlMutex    dbmutex;
extern qtDatabase db;


/////////////////////////////////////////////////////////////////
#else // Aqui começa o código que vai ser compilado na seção Main
/////////////////////////////////////////////////////////////////

rlModbusClient     modbus(modbusdaemon_MAILBOX,modbusdaemon_SHARED_MEMORY,modbusdaemon_SHARED_MEMORY_SIZE);

#include <QCoreApplication>

rlThread   usrThread;
rlMutex    dbmutex;
qtDatabase db;
static const char *tableName="REMOTA";
static char buf[16384];
static unsigned int oldBits[10] = {0,0,0,0,0,0,0,0,0,0}; // 320 bits

static int readBit(int b)
{
	unsigned int bit = b % 32;
	unsigned int w = b / 32;
	return !!(oldBits[w] & 1 << bit);
}

static void writeBit(int b, int val)
{
	int bit = b % 32;
	int w = b / 32;
	if(val) oldBits[w] |=   1 << bit;
	else    oldBits[w] &= ~(1 << bit);
}

static void task01()
{
	static int i = 0, day0 = 0;
	int j;
	rlTime HMS;

	//~ printf("Task01 : %d\n",i);
	HMS.getLocalTime();
	i=sprintf(buf,"insert %s values (now(3)",tableName);
	for(j=0; j<8; j++)
		i+=sprintf(&buf[i],",%g", modbus.readShort(modbusdaemon_CYCLE3_BASE,j)*20.0/20000);
	i+=sprintf(&buf[i],")"); buf[i]=0;
	dbmutex.lock();
	db.dbQuery(buf);
	if(HMS.day!=day0) 
	{
		day0=HMS.day; 
		sprintf(buf,"delete from %s where t < (now() - interval 3 week)",tableName); 
		db.dbQuery(buf);
	}
	dbmutex.unlock();

	i++;
};

static void task02()
{
	static int i = 0;
 	i++;
};

static void setup()
{
	int i,j,n;

	dbmutex.lock();
	db.open("QMYSQL","localhost","pvdb","","");
	db.dbQuery("create table if not exists ALARM (t timestamp(3), Estado varchar(3), Descrição varchar(255));");
	db.dbQuery("create index if not exists idx_ALARM on ALARM(t);");
//	db.dbQuery("create index idx_ALARM on ALARM(t);");

	i=sprintf(buf,"create table if not exists %s (t timestamp(3)",tableName);
	for (j=0; j<8; j++) i+=sprintf(&buf[i],",AI%d float", j+1);
	i+=sprintf(&buf[i],");"); buf[i]=0;
	db.dbQuery(buf);
	i=sprintf(buf,"create index if not exists idx_%s on %s(t);",tableName, tableName);
//	i=sprintf(buf,"create index idx_%s on %s(t);",tableName, tableName);
        db.dbQuery(buf);
        i=sprintf(buf,"select * from %s limit 1;",tableName);
        db.dbQuery(buf);
        dbmutex.unlock();
        n=db.result->record().count()-1;
        printf("records:%d\n",n);
    
};

#define TZ 100

const char *DIOname[16] = { "DI1", "DI2", "DI3", "DI4", "DI5", "DI6", "DI7", "DI8", "DO1", "DO2", "DO3", "DO4", "DO5", "DO6", "DO7", "DO8" };
 
static void loop()
{
	static int i=0;
	
	if(!(i%10)) task01(); // Chamar task01 a cada 1 segundo
	if(!(i%2)) task02(); // Chamar task02 a cada 0.5 segundos

	for(int j=0; j<16; j++)
	{
		int b  = !!modbus.readBit(modbusdaemon_CYCLE1_BASE,j);
		writeAlarm(j, b);
		if(b != readBit(j))
		{
			sprintf(buf,"INSERT ALARM VALUES (NOW(3), '%s', '%s');", b ? "ON" : "OFF", DIOname[j]);
			dbmutex.lock(); db.dbQuery(buf); dbmutex.unlock();
			writeBit(j, b);
			// printf("%s\n",buf);
		}
	}

	updateAlarm();

	i++;
	pvSleep(TZ); // TZ = 100 ms
};

static void *usrMain(void *arg)
{
 setup();
 while(1) loop();
 return arg;
};

////////////////////////////////////////////////////////////////////////

#endif

#endif

// FIM DE _MAINLOOP_
//
//***************************************************************************


#endif

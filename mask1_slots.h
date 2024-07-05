//###############################################################
//# mask1_slots.h for ProcessViewServer created: ter mai 7 07:25:59 2024
//# please fill out these slots
//# here you find all possible events
//# Yours: Lehrig Software Engineering
//###############################################################

// todo: uncomment me if you want to use this data aquisiton
// also uncomment this classes in main.cpp and pvapp.h
// also remember to uncomment rllib in the project file
extern rlModbusClient     modbus;  //Change if applicable
//extern rlSiemensTCPClient siemensTCP;
//extern rlPPIClient        ppi;

#include "rltime.h"
#define maxPnt 600
#define nCurves 4
#define _DEBUG_ 1

typedef struct // (todo: define your data structure here)
{
double x[maxPnt],y[nCurves][maxPnt];
int points;
double t0;
int init;
int c[8];
int ackblk;
}
DATA;

enum Docks {
    ALARM_DOCK = ID_DOCK_WIDGETS
};

static int slotInit(PARAM *p, DATA *d)
{
  if(p == NULL || d == NULL) return -1;
  memset(d,0,sizeof(DATA));
  configMask(p,MaskBrowser,1);
  alarmInit(p,GroupBox_alarm,alarmHTML,ALARM_DOCK);
  pvResize(p,0,1024,768);
//****************************************************************************
// Configuração dos medidores
  for(int i = 0; i<4; i++) 
      qwtDialSetRange(p,AI1+i,0,20,4);
//****************************************************************************
// Configuração do gráfico de tendência
  qpwSetTitle(p,Trend,"Módulo de IO analógico");         // Título do gráfico e cor de fundo
  qpwSetCanvasBackground(p,Trend,239,239,239);
  qpwSetLegendPos(p,Trend,RightLegend);   // Legenda
  qpwSetLegendFrameStyle(p,Trend,Box|Sunken);

  qpwSetAxisScale(p,Trend,yLeft,0,21,4);   // Escala da ordenada esquerda
  qpwSetAxisTitle(p,Trend,yLeft, "[mA]");  // Título da ordenada esquerda

  qpwSetAxisScaleDraw(p,Trend,xBottom,"hh:mm:ss"); // Abscissa

  int i=0;                                // Curvas
  qpwInsertCurve(p,Trend, i , "AI1 [mA]");
  qpwSetCurvePen(p,Trend, i, 255,0,0 , 2, SolidLine);
  qpwSetCurveYAxis(p,Trend, i++, yLeft);

  qpwInsertCurve(p,Trend, i , "AI2 [mA]");
  qpwSetCurvePen(p,Trend, i, 0,0,255 , 2, SolidLine);
  qpwSetCurveYAxis(p,Trend, i++, yLeft);

  qpwInsertCurve(p,Trend, i , "AI3 [mA]");
  qpwSetCurvePen(p,Trend, i, 255,0,255 , 2, SolidLine);
  qpwSetCurveYAxis(p,Trend, i++, yLeft);

  qpwInsertCurve(p,Trend, i , "AI4 [mA]");
  qpwSetCurvePen(p,Trend, i, 0,255,255 , 2, SolidLine);
  qpwSetCurveYAxis(p,Trend, i++, yLeft);
//****************************************************************************
  rlTime t0;
  t0.getLocalTime();  
  t0.hour=1;
  t0.minute=0;
  t0.second=0;
  t0.millisecond=0;
  d->t0=t0.secondsSinceEpoche();

//****************************************************************************
// Configuração da tabela
  char buf[32];
  int id = Table1;
  int x,y;
  pvSetRowHeight(p,id,-1,1);
  pvSetColumnWidth(p,id,-1,0);
  pvSetEditable(p,id,0);
  for(x = 0; x<8; x++){
      pvSetColumnWidth(p,id,x,127);
      y = 0;
      pvSetTableTextAlignment(p,id,x,y,AlignHCenter);
      pvTablePrintf(p,id,x,y++,"color(100,255,100)DI%d",x+1);

      i = !!modbus.readBit(modbusdaemon_CYCLE2_BASE,x);
      d->c[x] = i;
      sprintf(buf,"color(%d,%d,%d)DO%d", 228+27*i, 228-127*i, 228-127*i, x+1);
      pvSetTableButton(p,id,x,y++,buf);
  }
//****************************************************************************
  return 0;
}

static int slotNullEvent(PARAM *p, DATA *d)
{
  if(p == NULL || d == NULL) return -1;
  alarmNullEvent(p, alarmHTML, ALARM_DOCK); //d->ackblk = 0;

  int i, AIoffset = modbusdaemon_CYCLE3_BASE;
  for(i = 0; i<4; i++)
      qwtDialSetValue(p,AI1+i,modbus.readShort(AIoffset,i)*20.0/20000);

//************************************************************************************
// Gráfico de tendências
  int j;
  rlTime HMS;

  memmove(&d->x[1],&d->x,sizeof(double)*(maxPnt-1));
  for(j=0; j<nCurves; j++) 
      memmove(&d->y[j][1],&d->y[j],sizeof(double)*(maxPnt-1));

  HMS.getLocalTime();
  d->x[0] = HMS.secondsSinceEpoche() + 4*60*60 - d->t0;
  for(i = 0; i<4; i++)
      d->y[i][0] = modbus.readShort(AIoffset,i)*20.0/20000;

  if(d->x[0] < d->x[1]+p->sleep/1000.0) d->x[0] = d->x[1] + p->sleep/1000.0;

  d->points += d->points<maxPnt ? 1 : 0;
  for(j=0; j<nCurves; j++) qpwSetCurveData(p, Trend, j, d->points, d->x, d->y[j]);

  qpwSetAxisScale(p,Trend, xBottom, d->x[d->points-1], d->x[0],(maxPnt*p->sleep)/1000/5);              
  qpwReplot(p,Trend);
//************************************************************************************

//****************************************************************************
// Atualização da tabela
  int id = Table1;
  int x, y;
  for(x = 0; x<8; x++){
      y = 0;
      i = modbus.readBit(0,x);
      pvTablePrintf(p,id,x,y++,"color(%d,%d,100)DI%d", 100+125*i, 100+125*(!i), x+1);
      i = !!modbus.readBit(modbusdaemon_CYCLE2_BASE,x);
      if(d->c[x] != i){
          d->c[x] = i;
          char buf[32];
          sprintf(buf,"color(%d,%d,%d)DO%d", 228+27*i, 228-127*i, 228-127*i, x+1);
          pvSetTableButton(p,id,x,y++,buf);
      }
  }
//****************************************************************************

  if(d->init < 100) d->init++;
  return 0;

}

static int slotButtonEvent(PARAM *p, int id, DATA *d)
{
  if(p == NULL || id == 0 || d == NULL) return -1;
  return 0;
}

static int slotButtonPressedEvent(PARAM *p, int id, DATA *d)
{
  if(p == NULL || id == 0 || d == NULL) return -1;
  return 0;
}

static int slotButtonReleasedEvent(PARAM *p, int id, DATA *d)
{
  if(p == NULL || id == 0 || d == NULL) return -1;
  return 0;
}

static int slotTextEvent(PARAM *p, int id, DATA *d, const char *text)
{
  if(p == NULL || id == 0 || d == NULL || text == NULL) return -1;
//  if((id == alarmHTML) && !d->ackblk) { d->ackblk = 1; alarmTextEvent(text); }
  if(id == alarmHTML) alarmTextEvent(text);
  return maskTextEvent(text);
}

static int slotSliderEvent(PARAM *p, int id, DATA *d, int val)
{
  if(p == NULL || id == 0 || d == NULL || val < -1000) return -1;
  return 0;
}

static int slotCheckboxEvent(PARAM *p, int id, DATA *d, const char *text)
{
  if(p == NULL || id == 0 || d == NULL || text == NULL) return -1;
  return 0;
}

static int slotRadioButtonEvent(PARAM *p, int id, DATA *d, const char *text)
{
  if(p == NULL || id == 0 || d == NULL || text == NULL) return -1;
  return 0;
}

static int slotGlInitializeEvent(PARAM *p, int id, DATA *d)
{
  if(p == NULL || id == 0 || d == NULL) return -1;
  return 0;
}

static int slotGlPaintEvent(PARAM *p, int id, DATA *d)
{
  if(p == NULL || id == 0 || d == NULL) return -1;
  return 0;
}

static int slotGlResizeEvent(PARAM *p, int id, DATA *d, int width, int height)
{
  if(p == NULL || id == 0 || d == NULL || width < 0 || height < 0) return -1;
  return 0;
}

static int slotGlIdleEvent(PARAM *p, int id, DATA *d)
{
  if(p == NULL || id == 0 || d == NULL) return -1;
  return 0;
}

static int slotTabEvent(PARAM *p, int id, DATA *d, int val)
{
  if(p == NULL || id == 0 || d == NULL || val < -1000) return -1;
  return 0;
}

static int slotTableTextEvent(PARAM *p, int id, DATA *d, int x, int y, const char *text)
{
  if(p == NULL || id == 0 || d == NULL || x < -1000 || y < -1000 || text == NULL) return -1;
  if(id == Table1 && !(d->init < 10)) {
      int offset = modbusdaemon_CYCLE2_BASE;
      int i =  !modbus.readBit(offset,x);
      char buf[32];
      if(x<4)
          for(int j = 0; j<4; j++) 
              if(j != x){
                  modbus.writeSingleCoil(1,j,0);
                  sprintf(buf,"color(228,228,228)DO%d", j+1);
                  pvSetTableButton(p,id,j,y,buf);
              }
      modbus.writeSingleCoil(1,x,i);
      sprintf(buf,"color(%d,%d,%d)DO%d", 228+27*i, 228-127*i, 228-127*i, x+1);
      pvSetTableButton(p,id,x,y,buf);
  }
  return 0;
}

static int slotTableClickedEvent(PARAM *p, int id, DATA *d, int x, int y, int button)
{
  if(p == NULL || id == 0 || d == NULL || x < -1000 || y < -1000 || button < 0) return -1;
  printf("TableClickedEvent\n");
  return 0;
}

static int slotSelectionEvent(PARAM *p, int id, DATA *d, int val, const char *text)
{
  if(p == NULL || id == 0 || d == NULL || val < -1000 || text == NULL) return -1;
  return 0;
}

static int slotClipboardEvent(PARAM *p, int id, DATA *d, int val)
{
  if(p == NULL || id == -1 || d == NULL || val < -1000) return -1;
  return 0;
}

static int slotRightMouseEvent(PARAM *p, int id, DATA *d, const char *text)
{
  if(p == NULL || id == 0 || d == NULL || text == NULL) return -1;
  //pvPopupMenu(p,-1,"Menu1,Menu2,,Menu3");
  return 0;
}

static int slotKeyboardEvent(PARAM *p, int id, DATA *d, int val, int modifier)
{
  if(p == NULL || id == 0 || d == NULL || val < -1000 || modifier < -1000) return -1;
  return 0;
}

static int slotMouseMovedEvent(PARAM *p, int id, DATA *d, float x, float y)
{
  if(p == NULL || id == 0 || d == NULL || x < -1000 || y < -1000) return -1;
  return 0;
}

static int slotMousePressedEvent(PARAM *p, int id, DATA *d, float x, float y)
{
  if(p == NULL || id == 0 || d == NULL || x < -1000 || y < -1000) return -1;
  return 0;
}

static int slotMouseReleasedEvent(PARAM *p, int id, DATA *d, float x, float y)
{
  if(p == NULL || id == 0 || d == NULL || x < -1000 || y < -1000) return -1;
  return 0;
}

static int slotMouseOverEvent(PARAM *p, int id, DATA *d, int enter)
{
  if(p == NULL || id == 0 || d == NULL || enter < -1000) return -1;
  return 0;
}

static int slotUserEvent(PARAM *p, int id, DATA *d, const char *text)
{
  if(p == NULL || id == 0 || d == NULL || text == NULL) return -1;
  return 0;
}

//###############################################################
//# mask4_slots.h for ProcessViewServer created: dom. jul. 7 00:05:40 2024
//# please fill out these slots
//# here you find all possible events
//# Yours: Lehrig Software Engineering
//###############################################################

// todo: uncomment me if you want to use this data aquisiton
// also uncomment this classes in main.cpp and pvapp.h
// also remember to uncomment rllib in the project file
//extern rlModbusClient     modbus;  //Change if applicable
//extern rlSiemensTCPClient siemensTCP;
//extern rlPPIClient        ppi;

typedef struct // (todo: define your data structure here)
{
  rlSvgAnimator s;
  meterSvg m[11];
}
DATA;

const char *svgFile = "ISA101.svg";

static int slotInit(PARAM *p, DATA *d)
{
  char buf[32];
  int i;
  if(p == NULL || d == NULL) return -1;
  //memset(d,0,sizeof(DATA));
  p->sleep = 100;
  configMask(p,MaskBrowser,4);
  initSVG(p, mySVG, &d->s, svgFile, 21,100,1024,768,1.0);//600.0/768);
  pvSetPaletteBackgroundColor(p,mySVG, BG_COLOR);
  for(i = 0; i<4; i++) {
    sprintf(buf,"PV.AI%d",i+1);
    initBargraph(&d->m[i], &d->s, svgFile, buf, -100, 100, -95, 95, -50, 50, -25, 25);
    sprintf(buf,"PV.AI%d",i+5);
    initMeter(&d->m[i+4], &d->s, svgFile, buf, -100, 100);
  }

  initBargraph(&d->m[8], &d->s, svgFile, "PV.PID", -100, 100, -95, 95, -50, 50, -25, 25);
  initRadar(&d->m[9], &d->s, svgFile, "Radar", 8);
  initRadar(&d->m[10], &d->s, svgFile, "Radar2", 3);

  return 0;
}

static int slotNullEvent(PARAM *p, DATA *d)
{
  int i;
  if(p == NULL || d == NULL) return -1;
  for(i = 0; i<4; i++) {
    BargraphSetValue(&d->m[i], modbus.readShort(modbusdaemon_CYCLE3_BASE,i)*200.0/20000-100);
    MeterSetValue(&d->m[i+4], modbus.readShort(modbusdaemon_CYCLE3_BASE,i+4)*200.0/20000-100);
  }

  BargraphSetValue(&d->m[8], modbus.readShort(modbusdaemon_CYCLE3_BASE,0)*200.0/20000-100);
  PIDSetValue(&d->m[8], modbus.readShort(modbusdaemon_CYCLE3_BASE,1)*200.0/20000-100, modbus.readShort(modbusdaemon_CYCLE3_BASE,2)*100.0/20000);
  float val[8];
  for(i = 0; i<8; i++) val[i] = modbus.readShort(modbusdaemon_CYCLE3_BASE,i)*100.0/20000;
  RadarSetValue(&d->m[9], val);
  RadarSetValue(&d->m[10], val);
  drawSVG(p,mySVG,&d->s);
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
  int i;

  i = maskTextEvent(text);
  if(i) p->sleep = 500;
  return i;
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
  return 0;
}

static int slotTableClickedEvent(PARAM *p, int id, DATA *d, int x, int y, int button)
{
  if(p == NULL || id == 0 || d == NULL || x < -1000 || y < -1000 || button < 0) return -1;
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

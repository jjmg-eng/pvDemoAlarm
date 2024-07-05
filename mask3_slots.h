//###############################################################
//# mask2_slots.h for ProcessViewServer created: ter abr 12 08:51:24 2022
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
 int Q1,Q2,Q3,Q12,Q23;
 int G1,G2,G3;
 int B12,B23,B32;
}
DATA;


static int slotInit(PARAM *p, DATA *d)
{
  if(p == NULL || d == NULL) return -1;
  memset(d,0,sizeof(DATA));
  configMask(p,MaskBrowser,3);
  initSVG(p,mySVG,&d->s,"unifilar.svg",21,100,1073,526,980.0/1073);
  pvSetPaletteBackgroundColor(p,mySVG, BG_COLOR);
  return 0;
}

static int slotNullEvent(PARAM *p, DATA *d)
{
  if(p == NULL || d == NULL) return -1;

  d->B12 = d->Q1 || (d->Q12 && (d->Q2 || (d->Q23 && d->Q3)));
  d->B23 = d->Q2 || (d->Q12 && d->Q1) || (d->Q23 && d->Q3);
  d->B32 = d->Q3 || (d->Q23 && (d->Q2 || (d->Q12 && d->Q1)));

  d->s.setMatrix ("PV.Q1",   1, !d->Q1  * 20 * acos(-1)/180, 0, 0, 169.03514, 211.87003);
  d->s.setMatrix ("PV.Q2",   1, !d->Q2  * 20 * acos(-1)/180, 0, 0, 533.10724, 211.87003);
  d->s.setMatrix ("PV.Q3",   1, !d->Q3  * 20 * acos(-1)/180, 0, 0, 897.17932, 211.87003);
  d->s.setMatrix ("PV.Q12",  1, !d->Q12 * 20 * acos(-1)/180, 0, 0, 327.02383, 15.113641);
  d->s.setMatrix ("PV.Q23",  1, !d->Q23 * 20 * acos(-1)/180, 0, 0, 691.09601, 15.113641);

  if(d->G1)
    {
 	d->s.svgChangeStyleOption("PV.G1","fill:","#ff5555\n");
 	d->s.svgChangeStyleOption("PV.G1","stroke:","#ff0000\n");
 	d->s.svgChangeStyleOption("B1"   ,"stroke:","#ff0000\n");
    }
  else
    {
	d->s.svgChangeStyleOption("PV.G1","fill:","#99ff55\n");
	d->s.svgChangeStyleOption("PV.G1","stroke:","#008000\n");
	d->s.svgChangeStyleOption("B1"   ,"stroke:","#008000\n");
    }

  if(d->G2)
    {
 	d->s.svgChangeStyleOption("PV.G2","fill:","#ff5555\n");
 	d->s.svgChangeStyleOption("PV.G2","stroke:","#ff0000\n");
 	d->s.svgChangeStyleOption("B2"   ,"stroke:","#ff0000\n");
    }
  else
    {
	d->s.svgChangeStyleOption("PV.G2","fill:","#99ff55\n");
	d->s.svgChangeStyleOption("PV.G2","stroke:","#008000\n");
	d->s.svgChangeStyleOption("B2"   ,"stroke:","#008000\n");
    }

  if(d->G3)
    {
 	d->s.svgChangeStyleOption("PV.G3","fill:","#ff5555\n");
 	d->s.svgChangeStyleOption("PV.G3","stroke:","#ff0000\n");
 	d->s.svgChangeStyleOption("B3"   ,"stroke:","#ff0000\n");
    }
  else
    {
	d->s.svgChangeStyleOption("PV.G3","fill:","#99ff55\n");
	d->s.svgChangeStyleOption("PV.G3","stroke:","#008000\n");
	d->s.svgChangeStyleOption("B3"   ,"stroke:","#008000\n");
    }

  if(d->B12)	d->s.svgRecursiveChangeStyleOption("B12","stroke:","#ff0000\n");
  else   	d->s.svgRecursiveChangeStyleOption("B12","stroke:","#008000\n");
  if(d->B23)	d->s.svgRecursiveChangeStyleOption("B23","stroke:","#ff0000\n");
  else   	d->s.svgRecursiveChangeStyleOption("B23","stroke:","#008000\n");
  if(d->B32)	d->s.svgRecursiveChangeStyleOption("B32","stroke:","#ff0000\n");
  else   	d->s.svgRecursiveChangeStyleOption("B32","stroke:","#008000\n");
  

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
  if(textEventType(text)==SVG_LEFT_BUTTON_PRESSED)
      {
          if(strstr(svgObjectName(text),"PV.G1") != NULL) d->G1 = !d->G1;
          if(strstr(svgObjectName(text),"PV.G2") != NULL) d->G2 = !d->G2;
          if(strstr(svgObjectName(text),"PV.G3") != NULL) d->G3 = !d->G3;
          if(!d->G1) d->Q1 = 0;
          if(!d->G2) d->Q2 = 0;
          if(!d->G3) d->Q3 = 0;
          if(strstr(svgObjectName(text),"PV.Q12") != NULL) { d->Q12 = !d->Q12; return 0; }
          if(strstr(svgObjectName(text),"PV.Q23") != NULL) { d->Q23 = !d->Q23; return 0; }
          if(strstr(svgObjectName(text),"PV.Q1") != NULL && d->G1) d->Q1 = !d->Q1;
          if(strstr(svgObjectName(text),"PV.Q2") != NULL && d->G2) d->Q2 = !d->Q2;
          if(strstr(svgObjectName(text),"PV.Q3") != NULL && d->G3) d->Q3 = !d->Q3;
      }
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

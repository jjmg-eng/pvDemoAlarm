////////////////////////////////////////////////////////////////////////////
//
// show_mask1 for ProcessViewServer created: ter mai 7 07:25:59 2024
//
////////////////////////////////////////////////////////////////////////////
#include "pvapp.h"
static const char *maskName[] ={
" Mask1 - Leituras Analógicas",
" Mask2 - Histórico de Eventos",
" Mask3 - Unifilar para Teste de SVG",
" Mask4 - Módulos SVG para ISA101 ",
" Mask5 - Tanques ",
"" };

void configMask(PARAM *p, int id, int index)
{
    pvSetGeometry(p,id,341,9,500,30);
    pvSetFont(p,id,"Ubuntu",16,0,0,0,0);
    for(int i = 0; maskName[i][0]; i++)   pvInsertItem(p, id, -1, NULL, maskName[i]);
    pvSetCurrentItem(p,id,index - 1);
    pvResize(p,0,1024,768);
}

int maskTextEvent(const char *text)
{
    int j = -1;
    for(int i = 0; maskName[i][0]; i++)   if(!strcmp(maskName[i],text)) j = i;
    return ++j;
}

int drawSVG(PARAM *p, int id,rlSvgAnimator *s)
{
	if(s->isModified == 0) return 0;
	gBeginDraw(p,id);
	s->writeSocket();
	gEndDraw(p);
	return 0;
}

int initSVG(PARAM *p, int id, rlSvgAnimator *s, const char *filename, int x, int y, int w, int h, float SCALE)
{
	pvSetGeometry(p,id,x,y,w*SCALE,h*SCALE);
	s->setSocket(&p->s);
	s->setId(id);
	s->read(filename);
	s->setScale(SCALE);
	s->setWindowSize(w,h);
	s->setMainObject("main");
	s->setMatrix ("main", SCALE, 0, 0, 0, 0, 0);
	return 0;	
}

////////////////////////////////////////////////////////////////////////

typedef struct
{
 float x,y;
} cartesian;

static void polarToCartesian(float centerX, float centerY, float radius, float angleInDegrees, cartesian *p)
{
  float angleInRadians = angleInDegrees * acos(-1) / 180.0;
  p->x = centerX + radius * cos(angleInRadians);
  p->y = centerY + radius * sin(angleInRadians);
}

static char *describeArc(char *buf, float x, float y, float radius, float startAngle, float endAngle)
{
    cartesian start, end;

    endAngle += endAngle > startAngle ? 0 : 360;
    polarToCartesian(x, y, radius, startAngle, &start);
    polarToCartesian(x, y, radius, endAngle, &end);

    int arcSweep = (endAngle - startAngle <= 180);

    sprintf(buf,"M %g,%g A %g,%g 0 %d 1 %g,%g", start.x, start.y, radius, radius, !arcSweep, end.x, end.y);
    return buf;
}

void initMeter(meterSvg *m, rlSvgAnimator *s, const char *fileName,
              const char *Name, float Vmin, float Vmax)
{
    pugi::xml_document doc;
    doc.load_file(fileName);
    pugi::xml_node node = doc.child("svg").child("g").child("g");
    pugi::xml_node meter = node.find_child_by_attribute("g", "id", Name);

    meter = meter.first_child();
    m->start = meter.attribute("sodipodi:start").as_float()*180/acos(-1);
    m->end   = meter.attribute("sodipodi:end").as_float()*180/acos(-1);

    meter = meter.next_sibling();
    m->cx = meter.attribute("sodipodi:cx").as_float();
    m->cy = meter.attribute("sodipodi:cy").as_float();
    m->rx = meter.attribute("sodipodi:rx").as_float();
    m->ry = meter.attribute("sodipodi:ry").as_float();
    strcpy(m->NAME,Name);
    strcpy(m->VAL,meter.attribute("id").value());

    meter = meter.next_sibling();
    strcpy(m->TAG,meter.attribute("id").value());

    meter = meter.next_sibling();
    strcpy(m->NEEDLE,meter.attribute("id").value());
    m->s = s;
    m->Vmin = Vmin;
    m->Vmax = Vmax;
}

void MeterSetValue(meterSvg *m, float val)
{
    char buf[256];
    float Vpu = ((val-m->Vmin)/(m->Vmax-m->Vmin));
    Vpu = Vpu > 1 ? 1 : Vpu < 0 ? 0 : Vpu;
    float arc = m->end - m->start;
    arc += m->end > m->start ? 0 : 360;
    float ea = m->start + Vpu * arc, sa = m->start;
    m->s->svgPrintf(m->VAL,"d=","%s",describeArc(buf, m->cx, m->cy, m->rx, sa, ea));
    m->s->setMatrix(m->NEEDLE, 1, (Vpu * arc - 270 + sa) * acos(-1)/180, 0,0, m->cx, m->cy);
    m->s->svgTextPrintf(m->TAG,"%4.0f",val);
}

void initBargraph(meterSvg *m, rlSvgAnimator *s, const char *fileName, const char *Name,
                float Vmin, float Vmax,
                float LL, float HH,
                float L, float H,
                float LO, float HI)
{
    char buf[256];
    float T = (Vmax-Vmin) / 100;
    float AH = HH - H;
    float AL = L - LL;
    float N = H - L;
    float O = HI- LO;

    pugi::xml_document doc;
    doc.load_file(fileName);
    pugi::xml_node node = doc.child("svg").child("g").child("g");
    pugi::xml_node meter = node.find_child_by_attribute("g", "id", Name);
    meter = meter.first_child();  // Escala
    meter = meter.next_sibling(); // Faixa total

    char c1, c2;
    sscanf(meter.attribute("d").value(),"%c %f,%f %c %g", &c1, &m->cx, &m->cy, &c2, &m->ry);

    meter = meter.next_sibling(); // Faixa alarme alto
    sprintf(buf,"M %f,%f v %f", m->cx, m->cy - (HH - Vmax)/T, AH / T);
    s->svgPrintf(meter.attribute("id").value(),"d=","%s", buf);
    strcpy(m->AH,meter.attribute("id").value());

    meter = meter.next_sibling(); // Faixa alarme baixo
    sprintf(buf,"M %f,%f v %f", m->cx, m->cy - (L - Vmax)/T, AL / T);
    s->svgPrintf(meter.attribute("id").value(),"d=","%s", buf);
    strcpy(m->AL,meter.attribute("id").value());

    meter = meter.next_sibling(); // Faixa normal
    sprintf(buf,"M %f,%f v %f", m->cx, m->cy - (H - Vmax)/T, N / T);
    s->svgPrintf(meter.attribute("id").value(),"d=","%s", buf);

    meter = meter.next_sibling(); // Faixa ótima
    sprintf(buf,"M %f,%f v %f", m->cx, m->cy - (HI - Vmax)/T, O / T);
    s->svgPrintf(meter.attribute("id").value(),"d=","%s", buf);

    meter = meter.next_sibling(); // Agulha
    strcpy(m->NEEDLE,meter.attribute("id").value());

    meter = meter.next_sibling(); // Display
    strcpy(m->TAG,meter.attribute("id").value());

    meter = meter.next_sibling(); // Alarme prioridade 1
    strcpy(m->A1,meter.attribute("id").value());

    meter = meter.next_sibling(); // Alarme prioridade 2
    strcpy(m->A2,meter.attribute("id").value());

    meter = meter.next_sibling();
    meter = meter.next_sibling(); // PID
    strcpy(m->SP,meter.attribute("id").value());
    meter = meter.next_sibling();
    strcpy(m->MV,meter.attribute("id").value());

    strcpy(m->NAME,Name);
    m->s = s;
    m->Vmin = Vmin;
    m->Vmax = Vmax;
    m->LL = LL;
    m->HH = HH;
    m->L = L;
    m->H = H;
    s->show(m->A1,0);
    s->show(m->A2,0);
}

void BargraphSetValue(meterSvg *m, float val)
{
    rlSvgAnimator *s = m->s;

    float Vpu = ((val-m->Vmin)/(m->Vmax-m->Vmin));
    Vpu = Vpu > 1 ? 1 : Vpu < 0 ? 0 : Vpu;
    s->setMatrix(m->NEEDLE, 1, 0, 0,(0.5 - Vpu)*100 , 0,0);
    s->svgTextPrintf(m->TAG,"%4.0f",val);

    if(val > m->HH)    s->svgChangeStyleOption(m->AH,"stroke:","#ff6600\n");
    else
        if(val > m->H) s->svgChangeStyleOption(m->AH,"stroke:","#ffff00\n");
        else           s->svgChangeStyleOption(m->AH,"stroke:","#808080\n");
    if(val < m->LL)    s->svgChangeStyleOption(m->AL,"stroke:","#ff6600\n");
    else
        if(val < m->L) s->svgChangeStyleOption(m->AL,"stroke:","#ffff00\n");
        else           s->svgChangeStyleOption(m->AL,"stroke:","#808080\n");

    s->show(m->A1, (val >= m->HH) || (val <= m->LL) );
    s->show(m->A2, (val > m->H && val < m->HH) || (val > m->LL && val < m->L) );
}

void PIDSetValue(meterSvg *m, float SP, float MV)
{
    rlSvgAnimator *s = m->s;

    float Vpu = ((SP-m->Vmin)/(m->Vmax-m->Vmin));
    Vpu = Vpu > 1 ? 1 : Vpu < 0 ? 0 : Vpu;
    s->setMatrix(m->SP, 1, 0, 0,(0.5 - Vpu)*100 , 0,0);

    Vpu = MV / 100;
    Vpu = Vpu > 1 ? 1 : Vpu < 0 ? 0 : Vpu;
    s->setMatrix(m->MV, 1, 0, 0,(0.5 - Vpu)*100 , 0,0);
}

void initRadar(meterSvg *m, rlSvgAnimator *s, const char *fileName,
              const char *Name, int n)
{
    float alpha = 90, delta = 360.0 / n, radius = 35;
    cartesian p;
    int i, j, k;
    char buf[1024], id[16];

    strcpy(m->NAME,Name);
    m->s = s;
    m->n = n;
    pugi::xml_document doc;
    doc.load_file(fileName);
    pugi::xml_node node = doc.child("svg").child("g").child("g");
    pugi::xml_node meter = node.find_child_by_attribute("g", "id", Name);

    meter = meter.first_child();
    strcpy(id, meter.attribute("id").value()); printf("%s\n", id);

    i=0;
    for(j=0; j<n; j++){
        polarToCartesian(0, 0, radius, alpha + delta * j,  &p);
        i += sprintf(&buf[i],"M 0,0 %g,%g ", p.x, -p.y);
    }
    printf("%s\n", buf);
    s->svgPrintf(id, "d=", "%s", buf); 
    puts("ok");


    for(k = 0; k<6; k++){
	    meter = meter.next_sibling();
	    strcpy(id, meter.attribute("id").value()); printf("%s\n", id);
	    i = sprintf(buf,"M ");
	    for(j = 0; j<=n; j++){
	        polarToCartesian(0, 0, radius * (k == 5 ? 0.5 : 1 - 0.2 * k), alpha + delta * (j == n ? 0 : j),  &p);

	        i += sprintf(&buf[i],"%g,%g ", p.x, -p.y);
	    }
	    s->svgPrintf(id, "d=", "%s", buf); 
    }

    strcpy(m->VAL,id);
}

void RadarSetValue(meterSvg *m, float *val)
{
    char buf[1024];
    int i, j, n = m->n;
    float alpha = 90, delta = 360.0 / n;
    cartesian p;
    i = sprintf(buf,"M ");
    for(j = 0; j<=n; j++){
        polarToCartesian(0, 0, 35 * val[j == n ? 0 : j] / 100, alpha + delta * (j == n ? 0 : j),  &p);
        i += sprintf(&buf[i],"%g,%g ", p.x, -p.y);
    }
    m->s->svgPrintf(m->VAL,"d=","%s",buf);
}


////////////////////////////////////////////////////////////////////////

// _begin_of_generated_area_ (do not edit -> use ui2pvc) -------------------

// our mask contains the following objects
enum {
  ID_MAIN_WIDGET = 0,
  AI1,
  AI2,
  AI3,
  AI4,
  Trend,
  Table1,
  GroupBox_alarm,
  alarmHTML,
  obj1,
  MaskBrowser,
  ID_END_OF_WIDGETS
};

// our mask contains the following widget names
  static const char *widgetName[] = {
  "ID_MAIN_WIDGET",
  "AI1",
  "AI2",
  "AI3",
  "AI4",
  "Trend",
  "Table1",
  "GroupBox_alarm",
  "alarmHTML",
  "obj1",
  "MaskBrowser",
  "ID_END_OF_WIDGETS",
  ""};

  static const char *toolTip[] = {
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  ""};

  static const char *whatsThis[] = {
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  ""};

  static const int widgetType[ID_END_OF_WIDGETS+1] = {
  0,
  TQwtDial,
  TQwtDial,
  TQwtDial,
  TQwtDial,
  TQwtPlotWidget,
  TQTable,
  TQGroupBox,
  TQTextBrowser,
  TQPushButton,
  TQComboBox,
  -1 };

static int generated_defineMask(PARAM *p)
{
  int w,h,depth;

  if(p == NULL) return 1;
  if(widgetName[0] == NULL) return 1; // suppress unused warning
  w = h = depth = strcmp(toolTip[0],whatsThis[0]);
  if(widgetType[0] == -1) return 1;
  if(w==h) depth=0; // fool the compiler
  pvStartDefinition(p,ID_END_OF_WIDGETS);

  pvQwtDial(p,AI1,0);
  pvSetGeometry(p,AI1,3,181,210,210);
  pvSetFont(p,AI1,"Ubuntu",11,0,0,0,0);

  pvQwtDial(p,AI2,0);
  pvSetGeometry(p,AI2,255,181,210,210);
  pvSetFont(p,AI2,"Ubuntu",11,0,0,0,0);

  pvQwtDial(p,AI3,0);
  pvSetGeometry(p,AI3,515,181,210,210);
  pvSetFont(p,AI3,"Ubuntu",11,0,0,0,0);

  pvQwtDial(p,AI4,0);
  pvSetGeometry(p,AI4,771,181,210,210);
  pvSetFont(p,AI4,"Ubuntu",11,0,0,0,0);

  pvQwtPlotWidget(p,Trend,0,10,1000);
  pvSetGeometry(p,Trend,3,400,1020,360);
  qpwSetCanvasBackground(p,Trend,239,240,241);
  qpwEnableAxis(p,Trend,yLeft);
  qpwEnableAxis(p,Trend,xBottom);
  pvSetFont(p,Trend,"Ubuntu",11,0,0,0,0);

  pvQTable(p,Table1,0,2,8);
  pvSetGeometry(p,Table1,3,50,1024,103);
  pvSetFont(p,Table1,"Ubuntu",11,0,0,0,0);

  pvQGroupBox(p,GroupBox_alarm,0,-1,HORIZONTAL,pvtr(""));
  pvSetGeometry(p,GroupBox_alarm,1089,111,204,132);
  pvSetFont(p,GroupBox_alarm,"Ubuntu",11,0,0,0,0);

  pvQTextBrowser(p,alarmHTML,GroupBox_alarm);
  pvSetGeometry(p,alarmHTML,54,51,99,30);
  pvSetFont(p,alarmHTML,"Ubuntu",11,0,0,0,0);

  pvQPushButton(p,obj1,0);
  pvSetGeometry(p,obj1,1122,321,99,30);
  pvSetText(p,obj1,pvtr("PushButton"));
  pvSetFont(p,obj1,"Ubuntu",11,0,0,0,0);

  pvQComboBox(p,MaskBrowser,0,0,AtBottom);
  pvSetGeometry(p,MaskBrowser,402,12,99,30);
  pvSetFont(p,MaskBrowser,"Ubuntu",11,0,0,0,0);


  pvEndDefinition(p);
  return 0;
}

// _end_of_generated_area_ (do not edit -> use ui2pvc) ---------------------

#include "mask1_slots.h"

static int defineMask(PARAM *p)
{
  if(p == NULL) return 1;
  generated_defineMask(p);
  // (todo: add your code here)
  return 0;
}


static int showData(PARAM *p, DATA *d)
{
  if(p == NULL) return 1;
  if(d == NULL) return 1;
  return 0;
}

static int readData(DATA *d) // from shared memory, database or something else
{
  if(d == NULL) return 1;
  // (todo: add your code here)
  return 0;
}


int show_mask1(PARAM *p)
{
  DATA d;
  char event[MAX_EVENT_LENGTH];
  char text[MAX_EVENT_LENGTH];
  char str1[MAX_EVENT_LENGTH];
  int  i,w,h,val,x,y,button,ret;
  float xval, yval;

  defineMask(p);
  //rlSetDebugPrintf(1);
  if((ret=slotInit(p,&d)) != 0) return ret;
  readData(&d); // from shared memory, database or something else
  showData(p,&d);
  pvClearMessageQueue(p);
  while(1)
  {
    pvPollEvent(p,event);
    switch(pvParseEvent(event, &i, text))
    {
      case NULL_EVENT:
        readData(&d); // from shared memory, database or something else
        showData(p,&d);
        if((ret=slotNullEvent(p,&d)) != 0) return ret;
        break;
      case BUTTON_EVENT:
        if(trace) printf("BUTTON_EVENT id=%d\n",i);
        if((ret=slotButtonEvent(p,i,&d)) != 0) return ret;
        break;
      case BUTTON_PRESSED_EVENT:
        if(trace) printf("BUTTON_PRESSED_EVENT id=%d\n",i);
        if((ret=slotButtonPressedEvent(p,i,&d)) != 0) return ret;
        break;
      case BUTTON_RELEASED_EVENT:
        if(trace) printf("BUTTON_RELEASED_EVENT id=%d\n",i);
        if((ret=slotButtonReleasedEvent(p,i,&d)) != 0) return ret;
        break;
      case TEXT_EVENT:
        if(trace) printf("TEXT_EVENT id=%d %s\n",i,text);
        if((ret=slotTextEvent(p,i,&d,text)) != 0) return ret;
        break;
      case SLIDER_EVENT:
        sscanf(text,"(%d)",&val);
        if(trace) printf("SLIDER_EVENT val=%d\n",val);
        if((ret=slotSliderEvent(p,i,&d,val)) != 0) return ret;
        break;
      case CHECKBOX_EVENT:
        if(trace) printf("CHECKBOX_EVENT id=%d %s\n",i,text);
        if((ret=slotCheckboxEvent(p,i,&d,text)) != 0) return ret;
        break;
      case RADIOBUTTON_EVENT:
        if(trace) printf("RADIOBUTTON_EVENT id=%d %s\n",i,text);
        if((ret=slotRadioButtonEvent(p,i,&d,text)) != 0) return ret;
        break;
      case GL_INITIALIZE_EVENT:
        if(trace) printf("you have to call initializeGL()\n");
        if((ret=slotGlInitializeEvent(p,i,&d)) != 0) return ret;
        break;
      case GL_PAINT_EVENT:
        if(trace) printf("you have to call paintGL()\n");
        if((ret=slotGlPaintEvent(p,i,&d)) != 0) return ret;
        break;
      case GL_RESIZE_EVENT:
        sscanf(text,"(%d,%d)",&w,&h);
        if(trace) printf("you have to call resizeGL(w,h)\n");
        if((ret=slotGlResizeEvent(p,i,&d,w,h)) != 0) return ret;
        break;
      case GL_IDLE_EVENT:
        if((ret=slotGlIdleEvent(p,i,&d)) != 0) return ret;
        break;
      case TAB_EVENT:
        sscanf(text,"(%d)",&val);
        if(trace) printf("TAB_EVENT(%d,page=%d)\n",i,val);
        if((ret=slotTabEvent(p,i,&d,val)) != 0) return ret;
        break;
      case TABLE_TEXT_EVENT:
        sscanf(text,"(%d,%d,",&x,&y);
        pvGetText(text,str1);
        if(trace) printf("TABLE_TEXT_EVENT(%d,%d,\"%s\")\n",x,y,str1);
        if((ret=slotTableTextEvent(p,i,&d,x,y,str1)) != 0) return ret;
        break;
      case TABLE_CLICKED_EVENT:
        sscanf(text,"(%d,%d,%d)",&x,&y,&button);
        if(trace) printf("TABLE_CLICKED_EVENT(%d,%d,button=%d)\n",x,y,button);
        if((ret=slotTableClickedEvent(p,i,&d,x,y,button)) != 0) return ret;
        break;
      case SELECTION_EVENT:
        sscanf(text,"(%d,",&val);
        pvGetText(text,str1);
        if(trace) printf("SELECTION_EVENT(column=%d,\"%s\")\n",val,str1);
        if((ret=slotSelectionEvent(p,i,&d,val,str1)) != 0) return ret;
        break;
      case CLIPBOARD_EVENT:
        sscanf(text,"(%d",&val);
        if(trace) printf("CLIPBOARD_EVENT(id=%d)\n",val);
        if((ret=slotClipboardEvent(p,i,&d,val)) != 0) return ret;
        break;
      case RIGHT_MOUSE_EVENT:
        if(trace) printf("RIGHT_MOUSE_EVENT id=%d text=%s\n",i,text);
        if((ret=slotRightMouseEvent(p,i,&d,text)) != 0) return ret;
        break;
      case KEYBOARD_EVENT:
        sscanf(text,"(%d",&val);
        if(trace) printf("KEYBOARD_EVENT modifier=%d key=%d\n",i,val);
        if((ret=slotKeyboardEvent(p,i,&d,val,i)) != 0) return ret;
        break;
      case PLOT_MOUSE_MOVED_EVENT:
        sscanf(text,"(%f,%f)",&xval,&yval);
        if(trace) printf("PLOT_MOUSE_MOVE %f %f\n",xval,yval);
        if((ret=slotMouseMovedEvent(p,i,&d,xval,yval)) != 0) return ret;
        break;
      case PLOT_MOUSE_PRESSED_EVENT:
        sscanf(text,"(%f,%f)",&xval,&yval);
        if(trace) printf("PLOT_MOUSE_PRESSED %f %f\n",xval,yval);
        if((ret=slotMousePressedEvent(p,i,&d,xval,yval)) != 0) return ret;
        break;
      case PLOT_MOUSE_RELEASED_EVENT:
        sscanf(text,"(%f,%f)",&xval,&yval);
        if(trace) printf("PLOT_MOUSE_RELEASED %f %f\n",xval,yval);
        if((ret=slotMouseReleasedEvent(p,i,&d,xval,yval)) != 0) return ret;
        break;
      case MOUSE_OVER_EVENT:
        sscanf(text,"%d",&val);
        if(trace) printf("MOUSE_OVER_EVENT %d\n",val);
        if((ret=slotMouseOverEvent(p,i,&d,val)) != 0) return ret;
        break;
      case USER_EVENT:
        if(trace) printf("USER_EVENT id=%d %s\n",i,text);
        if((ret=slotUserEvent(p,i,&d,text)) != 0) return ret;
        break;
      default:
        if(trace) printf("UNKNOWN_EVENT id=%d %s\n",i,text);
        break;
    }
  }
}

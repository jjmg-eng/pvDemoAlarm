//
// alarm system
//
// Modificado por João J.M. Gonçalves em 27/12/2018

#include "alarm.h"

Alarm::Alarm(int useEventLog)
{
    max_alarms = num_alarms = 0;
    use_event_log = useEventLog;
}

Alarm::~Alarm()
{
}

const char *Alarm::strip(const char *text)
{
    txt2 = &text[1];
    char *mytext = txt2.text();
    char *cptr = strchr(mytext,'\"');
    if(cptr != NULL) *cptr = '\0';
    return mytext;
}

int Alarm::loadCSV(const char *filename)
{
    max_alarms = num_alarms = 0;
    int ret = table.read(filename);
    if(ret < 0) return ret;
    rlSpreadsheetRow *row = table.getFirstRow();
    while(row != NULL)
    {
        max_alarms++;
        row = row->getNextRow();
    }
    return max_alarms;
}

int Alarm::set(const char *alarmId)
{
    mutex.lock();
    time.getLocalTime();
    rlSpreadsheetRow *row = table.getFirstRow();
    while(row != NULL)
    {
        if(strcmp(alarmId,strip(row->text(1))) == 0)
        {
            if(strlen(row->text(3)) == 0) num_alarms++;
            txt1  = time.getTimeString();
            txt1 += " : ";
            txt1 += strip(row->text(2));
            row->setText(3,txt1.text()); // set the ALARM message
            mutex.unlock();
            if(use_event_log)
            {
                rlEvent(rlError,"%s",strip(row->text(2))); // send alarm to event log
            }
            return 0;
        }
        row = row->getNextRow();
    }
    mutex.unlock();
    return -1;
}

int Alarm::setInt(const char *alarmId, int i)
{
    mutex.lock();
    rlString txt;
    time.getLocalTime();
    rlSpreadsheetRow *row = table.getFirstRow();
    while(row != NULL)
    {
        if(strcmp(alarmId,strip(row->text(1))) == 0)
        {
            if(strlen(row->text(3)) == 0) num_alarms++;
            txt1  = time.getTimeString();
            txt1 += " : ";
            txt.printf(strip(row->text(2)), i);
            txt1 += txt;
            row->setText(3,txt1.text()); // set the ALARM message
            mutex.unlock();
            if(use_event_log)
            {
                rlEvent(rlError,strip(row->text(2)),i); // send alarm to event log
            }
            return 0;
        }
        row = row->getNextRow();
    }
    mutex.unlock();
    return -1;
}

int Alarm::setFloat(const char *alarmId, float f)
{
    mutex.lock();
    rlString txt;
    time.getLocalTime();
    rlSpreadsheetRow *row = table.getFirstRow();
    while(row != NULL)
    {
        if(strcmp(alarmId,strip(row->text(1))) == 0)
        {
            if(strlen(row->text(3)) == 0) num_alarms++;
            txt1  = time.getTimeString();
            txt1 += " : ";
            txt.printf(strip(row->text(2)), f);
            txt1 += txt;
            row->setText(3,txt1.text()); // set the ALARM message
            mutex.unlock();
            if(use_event_log)
            {
                rlEvent(rlError,strip(row->text(2)),f); // send alarm to event log
            }
            return 0;
        }
        row = row->getNextRow();
    }
    mutex.unlock();
    return -1;
}

int Alarm::ack(const char *alarmId)
{
    rlString alarm_id(alarmId);
    rlString temp;
    if(alarm_id.startsWith("alarm://"))
    {
        alarm_id = &alarmId[8];
    }
    mutex.lock();
    rlSpreadsheetRow *row = table.getFirstRow();
    while(row != NULL)
    {
        if(alarm_id.strnocasecmp(strip(row->text(1))) == 0)
        {
            temp.printf("Ack alarm: %s", row->text(3));
            if(strncmp(row->text(3),"Ack alarm:",10) != 0)
            {
                row->setText(3,temp.text()); // ack the ALARM message
            }
            mutex.unlock();
            if(use_event_log)
            {
                rlEvent(rlInfo,"Ack alarm: %s",strip(row->text(2))); // send alarm to event log
            }
            return 0;
        }
        row = row->getNextRow();
    }
    mutex.unlock();
    return -1;
}

int Alarm::reset(const char *alarmId)
{
    rlString alarm_id(alarmId);
    if(alarm_id.startsWith("alarm://"))
    {
        alarm_id = &alarmId[8];
    }
    mutex.lock();
    rlSpreadsheetRow *row = table.getFirstRow();
    while(row != NULL)
    {
        if(alarm_id.strnocasecmp(strip(row->text(1))) == 0)
        {
            if(strlen(row->text(3)) != 0) num_alarms--;
            if(num_alarms < 0) num_alarms = 0;
            row->setText(3,""); // reset the ALARM message
            mutex.unlock();
            if(use_event_log)
            {
                rlEvent(rlInfo,"Reset alarm: %s",strip(row->text(2))); // send alarm to event log
            }
            return 0;
        }
        row = row->getNextRow();
    }
    mutex.unlock();
    return -1;
}

int Alarm::ackAll()
{
    rlString temp;
    mutex.lock();
    rlSpreadsheetRow *row = table.getFirstRow();
    while(row != NULL)
    {
        if(strlen(row->text(3)) > 0)
        {
            temp.printf("Ack alarm: %s", row->text(3));
            if(strncmp(row->text(3),"Ack alarm:",10) != 0)
            {
                row->setText(3,temp.text()); // ack the ALARM message
            }
            if(use_event_log)
            {
                rlEvent(rlInfo,"Ack alarm: %s",strip(row->text(2))); // send alarm to event log
            }
        }
        row = row->getNextRow();
    }
    mutex.unlock();
    return 0;
}

int Alarm::resetAll()
{
    mutex.lock();
    rlSpreadsheetRow *row = table.getFirstRow();
    while(row != NULL)
    {
        if(strlen(row->text(3)) != 0) num_alarms--;
        if(num_alarms < 0) num_alarms = 0;
        row->setText(3,""); // reset the ALARM message
        if(use_event_log)
        {
            rlEvent(rlInfo,"Reset alarm: %s",strip(row->text(2))); // send alarm to event log
        }
        row = row->getNextRow();
    }
    mutex.unlock();
    return 0;
}

const char * Alarm::text(const char *alarmId)
{
    rlString alarm_id(alarmId);
    if(alarm_id.startsWith("alarm://"))
    {
        alarm_id = &alarmId[8];
    }
    mutex.lock();
    rlSpreadsheetRow *row = table.getFirstRow();
    while(row != NULL)
    {
        if(alarm_id.strnocasecmp(strip(row->text(1))) == 0)
        {
            mutex.unlock();
            return row->text(3);
        }
        row = row->getNextRow();
    }
    mutex.unlock();
    return NULL;
}

int Alarm::isSet(const char *alarmId)
{
    const char *txt = text(alarmId);
    if(txt == NULL)     return 0;
    if(strlen(txt) > 0) return 1;
    return 0;
}

int Alarm::count()
{
    return num_alarms;
}

int Alarm::maxCount()
{
    return max_alarms;
}

static const char *MYHTML_HEADER = 
"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n<html>\
<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"><head>\n";
static const char *MYHTML_TRAILER = 
"</body></html>";

int Alarm::updateWidgetHTML(PARAM *p, int id, int *state)
{
    struct texto
    {
        rlString txt;
        rlTime time;
        texto *ptr;
    };

    texto *txt1=NULL;
    texto *txt2=NULL;
    texto *txt3=NULL;
    rlString html(MYHTML_HEADER);

    if(num_alarms == 0)
    {
        if(*state > 1) return 0; // no update needed already green
        *state = 2;
        html += "<body style=\"font-family:arial;background-color:rgb(239,235,231)\">";
        html += "<p>Sem alarmes</p>";
    }
    else
    {
        if(*state == 0) *state = 1;
        else            *state = 0;
        html.printf("%s<body style=\"font-family:arial;background-color:rgb(255,255,0)\">\n<p>Número de alarmes = %d</p>\n",
                    MYHTML_HEADER, num_alarms);

        rlSpreadsheetRow *row = table.getFirstRow();
        while(row != NULL)
        {
            if(strlen(row->text(3)) != 0)
            {
                if(txt1 == NULL) txt1 = new texto;
                if(strncmp(row->text(3),"Ack alarm:",10) == 0)
                {
                    txt1->time.setTimeFromString(row->text(3)+10);
                    txt1->txt = "<p> <a style=\"color:0000FF\" ";
                }
                else
                {
                    txt1->time.setTimeFromString(row->text(3));
                    if(*state == 0) txt1->txt = "<p> <a style=\"color:FF0000\" ";
                    else            txt1->txt = "<p> <a style=\"color:880000\" ";
                }

                txt1->txt += "href=\"alarm://";
                txt1->txt += strip(row->text(1));
                txt1->txt += "\">";
                txt1->txt += strip(row->text(1));
                txt1->txt += ": ";
                txt1->txt += row->text(3);
                txt1->txt += "</a></p>\n";

                if(txt2==NULL)
                {
                    txt2=new texto;
                    *txt2=*txt1;
                    txt2->ptr=NULL;
                    txt3=txt2;
                }
                else
                {
                    if(txt1->time >= txt2->time)
                    {
                        txt1->ptr=txt2;
                        txt2=txt1;
                    }
                    else
                    {
                        txt3=txt2;
                        while((txt3 != NULL) && (txt3->ptr !=NULL) && (txt3->time > txt1->time) && (txt3->ptr->time > txt1->time))
                            txt3=txt3->ptr;
                        txt1->ptr=txt3->ptr;
                        txt3->ptr=txt1;
                    }
                    txt1=new texto;
                }
            }
            row = row->getNextRow();
        }

        while(txt2!=NULL)
        {
            html+=txt2->txt;
            txt3=txt2->ptr;
            delete txt2;
            txt2=txt3;
        }
    }

    html += MYHTML_TRAILER;
    pvSetText(p,id,html.text());
//    printf("%s\n",html.text());
    delete txt1;
    return 0;
}


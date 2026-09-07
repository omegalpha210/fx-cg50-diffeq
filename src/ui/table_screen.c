#include "graph.h"
#include "table.h"
#include "storage.h"
#include "ui.h"
#include <stdio.h>
#ifndef DIFFEQ_STORAGE_DIR
#define DIFFEQ_STORAGE_DIR "/"
#endif
void ui_table(App *a)
{
    Document *d=&a->doc;
    if(!d->nic){ui_message("Table","Add an initial condition for numerical output.");return;}
    int family=0,direction=1,column=0;unsigned start=0;
    for(;;) {
        TablePage page;
        ui_frame("Calculating table...","EXIT cancels");dupdate();
        table_page(d,&a->model,family,direction,start,&page,ui_cancel,NULL);
        char title[64];snprintf(title,sizeof(title),"Table / IC%d / %s",family+1,direction>0 ? "+x":"-x");
        ui_frame(title,"Left/Right: columns   UP/DOWN: page");
        int columns[10],count=0;
        for(int i=0;i<=d->dim;i++)if(d->list_mask[family]&(1u<<i))columns[count++]=i;
        if(column>=count)column=0;
        if(!count)ui_text(12,76,UI_MUTED,"No List columns selected in Output.");
        for(int j=0;j<3 && column+j<count;j++) {
            int index=columns[column+j];char label[20];model_variable_label(d,index-1,label,sizeof(label));
            ui_rect(6+j*125,41,123,18,UI_BLUE);ui_text(10+j*125,45,C_WHITE,"%s",label);
            for(unsigned row=0;row<page.count;row++) {
                int y=65+(int)row*17;ui_rect(6+j*125,y-2,123,16,row%2 ? UI_PALE:C_WHITE);
                ui_text(10+j*125,y,UI_INK,"%.8g",page.row[row][index]);
            }
        }
        if(page.result.status!=ODE_OK && page.result.status!=ODE_SAMPLE_STOP)
            ui_text(8,184,C_RED,"Partial: %s",ode_status_text(page.result.status));
        else ui_text(8,184,UI_MUTED,"Rows %lu-%lu | Step %d%s",(unsigned long)(page.count ? start+1:0),
            (unsigned long)(start+page.count),d->solver.step,page.more ? " | more":" | end");
        ui_softkeys("TOP","BTM","DIR","IC+","STAT","GRAPH");dupdate();
        int key=ui_getkey().key;
        if(key==KEY_EXIT || key==KEY_F6)return;
        if(key==KEY_F1)start=0;
        if(key==KEY_UP && start>=TABLE_ROWS)start-=TABLE_ROWS;
        if(key==KEY_DOWN && page.more)start+=TABLE_ROWS;
        if(key==KEY_F2) {
            OdeStatus status;ui_frame("Finding table end...","EXIT cancels");dupdate();
            unsigned bottom=table_last_page(d,&a->model,family,direction,&status,ui_cancel,NULL);
            if(status==ODE_OK)start=bottom;
            else if(status!=ODE_CANCELLED)ui_message("Table end",ode_status_text(status));
        }
        if(key==KEY_LEFT && column>0)column--;
        if(key==KEY_RIGHT && column+3<count)column++;
        if(key==KEY_F3){direction=-direction;start=0;}
        if(key==KEY_F4){family=(family+1)%d->nic;start=0;}
        if(key==KEY_F5) {
            char path[256],message[300];OdeStatus status;
            ui_frame("Preparing STAT data","Current IC/direction. EXIT cancels.");dupdate();
            if(storage_stat_csv(d,&a->model,family,direction,DIFFEQ_STORAGE_DIR,
                path,sizeof(path),&status,ui_cancel,NULL)) {
                snprintf(message,sizeof(message),"Saved %s\nSTAT: List Editor > F6 > F6 > CSV > LOAD.",path);
                ui_message("STAT data saved",message);
            } else {
                if(status==ODE_STEP_LIMIT)snprintf(message,sizeof(message),
                    "More than 998 data rows. Increase Step and retry. Incomplete file removed.");
                else snprintf(message,sizeof(message),"Export failed: %s.",ode_status_text(status));
                ui_message("STAT data not saved",message);
            }
        }
    }
}

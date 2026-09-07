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
    Document *d=&a->doc;TableIndex index;
    ui_frame("Preparing table...","EXIT cancels");dupdate();
    OdeStatus status=table_index_build(d,&a->model,&index,ui_cancel,NULL);
    if(status!=ODE_OK){if(status!=ODE_CANCELLED)ui_message("Table",ode_status_text(status));return;}
    int column=0;unsigned start=index.mid;
    for(;;) {
        TablePage page;ui_frame("Calculating table...","EXIT cancels");dupdate();
        table_read_page(d,&a->model,&index,start,&page,ui_cancel,NULL);
        if(page.result.status==ODE_CANCELLED)return;
        ui_frame(index.solutions && d->nic>1 ? "Table / initial solutions":"Table",
            index.count>2 ? "Left/Right: columns   UP/DOWN: page":"UP/DOWN: page");
        for(int j=0;j<3 && (j==0 || column+j-1<index.count);j++) {
            int selected=j==0 ? -1:column+j-1;char label[20];
            table_column_label(d,&index,selected,label,sizeof(label));
            ui_rect(6+j*125,41,123,18,UI_BLUE);ui_text(10+j*125,45,C_WHITE,"%s",label);
            for(unsigned row=0;row<page.count;row++) {
                int y=65+(int)row*17,data=selected+1;
                ui_rect(6+j*125,y-2,123,16,row%2 ? UI_PALE:C_WHITE);
                if(page.valid[row]&(1u<<data))ui_text(10+j*125,y,UI_INK,"%.8g",page.row[row][data]);
                else ui_text(10+j*125,y,UI_MUTED,"--");
            }
        }
        if((!start && ode_invalid_region(index.low))
            || (start==table_bottom(&index) && ode_invalid_region(index.high)))
            ui_text(8,184,C_RED,"END: Numerical limit");
        else if(page.result.status!=ODE_OK)ui_text(8,184,C_RED,"Partial: %s",ode_status_text(page.result.status));
        else ui_text(8,184,UI_MUTED,"Rows %lu-%lu / %lu | Step %d%s",(unsigned long)(start+1),
            (unsigned long)(start+page.count),(unsigned long)index.total,d->solver.step,
            !start || start==table_bottom(&index) ? " | END":"");
        ui_softkeys("TOP","BTM","MID","","STAT","GRAPH");dupdate();
        int key=ui_getkey().key;
        if(key==KEY_EXIT || key==KEY_F6)return;
        if(key==KEY_F1)start=0;
        if(key==KEY_F2)start=table_bottom(&index);
        if(key==KEY_F3)start=index.mid;
        if(key==KEY_UP)start=start>TABLE_ROWS ? start-TABLE_ROWS:0;
        if(key==KEY_DOWN){start+=TABLE_ROWS;if(start>table_bottom(&index))start=table_bottom(&index);}
        if(key==KEY_LEFT && column>0)column--;
        if(key==KEY_RIGHT && column+2<index.count)column++;
        if(key==KEY_F5) {
            char path[256],message[300];
            ui_frame("Preparing STAT data","Ascending x. EXIT cancels.");dupdate();
            if(storage_stat_csv(d,&a->model,DIFFEQ_STORAGE_DIR,path,sizeof(path),&status,ui_cancel,NULL)) {
                snprintf(message,sizeof(message),"Saved %s\nSTAT: List Editor > F6 > F6 > CSV > LOAD.",path);
                ui_message("STAT data saved",message);
            } else {
                snprintf(message,sizeof(message),status==ODE_STEP_LIMIT ?
                    "More than 998 data rows. Increase Step and retry.":"Export failed: %s.",ode_status_text(status));
                ui_message("STAT data not saved",message);
            }
        }
    }
}

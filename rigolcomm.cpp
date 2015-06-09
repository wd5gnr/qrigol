/*
 *
 *  This program is Copyright (c) 2015 by Al Williams al.williams@awce.com
 *  All rights reserved.
 *
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    */
#include "rigolcomm.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>


// The intent of this file is to just read/write
// the device in a very raw way



static const int max_cmd_len=255;
static const int max_resp_len=2*512*1024+10; // 10 byte header on newer scopes

RigolComm::RigolComm()
{
    _buffer=NULL;
    data_size=0;
}

RigolComm::~RigolComm()
{
    if (buffer) free(_buffer);
}

int RigolComm::open(const char *device)
{
    fd=::open(device,O_RDWR);
    if (fd>0)
    {
        _buffer=(char *)malloc(max_resp_len+1);
        buffer=_buffer;
    }
   return fd>0 && buffer!=NULL;
}

int RigolComm::unlock()
{
    return send(":KEY:LOCK DISABLE");
}

int RigolComm::close(void)
{
    if (fd>=0)
    {
        unlock();
        ::close(fd);
    }
    if (_buffer) free(_buffer);
    _buffer=NULL;
    return 0;
}

int RigolComm::send(const char *command)
{
     int size;
     char buf[max_cmd_len+2];
     if (!connected()) return -1;
     if (strlen(command)>max_cmd_len) return -1;
     strncpy(buf,command,max_cmd_len);
     strcat(buf,"\n");
     buf[max_cmd_len+1]='\0';
     size=::write(fd,buf,strlen(buf));
     return size<0?size:0;
}

// Returns 0 for anything other than data buffers
// Databuffers return size of data in buffer
// There is at least one report that the OLD_PROTOCOL handling in this routine is not working
int RigolComm::get_data_size(int rawsize)
 {
     int rv;
     char csize[9];
     buffer=_buffer;
     if (_buffer[0]!='#' || _buffer[1]!='8' || getenv("RIGOL_USE_OLD_PROTOCOL")) return rawsize<512?0:rawsize;
     strncpy(csize,_buffer+2,sizeof(csize)-1);
     csize[sizeof(csize)-1]='\0';  // comment that used to be here was incorrect ;-)
     rv=atoi(csize);
     return rv;
}

int RigolComm::recv(void)
{
    int siz;
    if (!connected()) return -1;
    siz=::read(fd,_buffer,max_resp_len);
    if (siz>0 && siz<max_resp_len) _buffer[siz]='\0';
    return siz<0?siz:get_data_size(siz);
}

float RigolComm::toFloat(void)
{
#if 0
    float rv=-1.0f;
    if (recv()>=0) sscanf(buffer,"%f",&rv);
    return rv;
#else
    float rv=-1.0f;
       // the problem here is sscanf uses the user's locale
       // But the Rigol ALWAYS uses a '.' for decimal separator
       struct lconv *lc=localeconv();
       char dp=*lc->decimal_point;
       if (recv()<0) return rv;
       if (dp!='.')
       {
           char *p;
           do
           {
               p=strchr(buffer,'.');
               if (p) *p=dp;  // convert to local decimal point
           } while (p);
       }
       sscanf(buffer,"%f",&rv);
       return rv;
#endif

}

float RigolComm::cmdFloat(const char *cmd)
{
    if (send(cmd)<0) return -1.0f;
    return toFloat();
}

float RigolComm::cmdFloatlt(const char *cmd)
{
    float rv=-1.0f;
    if (send(cmd)<0) return -rv;
    if (recv()>=0) sscanf(buffer+1,"%f",&rv);
    return rv;
}


int RigolComm::command(const char *cmd)
{
    if (send(cmd)<=0)
        if (strchr(cmd,'?'))
            return recv();
    return 0;
}

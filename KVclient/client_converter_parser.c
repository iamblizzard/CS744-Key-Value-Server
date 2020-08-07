#include "client.h"
#include <assert.h>

int from_XML_to_out(char * responseXML, char * ans){
    char * message = "<Message>";
    char * t = strstr(responseXML,message);
    strcpy(ans,"");
   int lastpointer = 0;
    if (t==NULL){
        // GET Response
        char * key = "<Key>";
        char * s = strstr(responseXML, key);
        if (s==NULL){
            printf("%.20s\n", responseXML);
            assert(1==2);
            return 0;
        }
        s = s + strlen(key);
        char *keyend = strstr(s, "</Key>");
        if (keyend == NULL){
            printf("%.20s\n", responseXML);
            assert(3==2);
            return 0;
        }
        strncat(ans,s,keyend -s);
        strcat(ans, ",");
        lastpointer = lastpointer +(keyend -s);
        lastpointer ++;//for comma        
        char * val = "<Value>";
        char * ter = strstr(keyend,val);
        ter = ter + strlen(val);
        char * endval = strstr(keyend, "</Value>");
        strncat(ans,ter,endval-ter);
        lastpointer+=(endval-ter);
        ans[lastpointer] = 0;
        return 1;
    }
    else{
        //PUT or DEL response: successful or unsuccesful
        t = t + strlen(message);
        char * messageend = strstr(t,"</Message>");
        strncat(ans,t,messageend-t );
        ans[messageend-t] = 0;
    }
    return 1;
}

int to_xml_client(char * request, FILE * wp, char * ans){
    if (request == NULL) return 0;
    
    char * header = "<?xml version  = \"1.0\" encoding=\"UTF-8\"?>\n<KVMessage type=";
    char* getreq = "\"getreq\">\n<Key>\0";
    char* putreq = "\"putreq\">\n<Key>\0";
    char*delreq= "\"delreq\">\n<Key>\0";
    char*endkey = "</Key>\n\0";
    char* startval = "<Value>\0";
    char*endval = "</Value>\n\0";
    char * endKVMessage = "</KVMessage>\n\0";
    strcpy(ans,header);
    char delim[] = ",\n";
    char * p = strtok(request,delim);
    int typereq;
    if (!strncmp(p, "GET",3)){
        typereq = TGET;
    }
    else if (!strncmp(p, "PUT",3)){
        typereq = TPUT;
    }
    else if (!strncmp(p, "DEL",3)){
        typereq = TDEL;
    }
    else{
        fprintf(wp,"%s","Unknown Error: Invalid Request Type. Trying next request.\n");
        // assert(1==2);
        // free(ans);
        return 0;
    }
    if (typereq == TPUT){
        p = strtok(NULL, delim);
        if (p==NULL){
            fprintf(wp,"%s","Unknown Error: Invalid PUT Request. Trying next request.\n");
            // assert(3==4);
            return 0;
            }
        if (strlen(p) > MAXKEYSIZE){
            fprintf(wp,"%s","Oversized key\n");
            // assert(5==6);
             
             return 0;
        }
        char *q = strtok(NULL, delim);
        if (q==NULL){
            // assert(7==8);
            fprintf(wp,"%s","Unknown Error: Invalid PUT Request. Trying next request.\n");
            return 0;
            }
          if (strlen(q) > MAXVALUESIZE){
            //   assert(9==10);
              fprintf(wp, "%s","Oversized value\n");
             return 0;
        }
        strcat(ans, putreq);
        strcat(ans,p);
        strcat(ans,endkey);
        strcat(ans, startval);
        strcat(ans,q);
        strcat(ans,endval);
        strcat(ans,endKVMessage);
    }
    else{//get or del
        p = strtok(NULL, delim);
        if (p==NULL) {
            fprintf(wp,"%s","Error: Invalid GET or DEL Request. Trying next request.\n");
           
            return 0;
        }
         if (strlen(p) > MAXKEYSIZE){
            fprintf(wp,"%s","Oversized key\n");
             return 0;
        }
        if (typereq == TGET) strcat(ans,getreq);
        else strcat(ans,delreq);
        strcat(ans,p);
        strcat(ans,endkey);
        strcat(ans,endKVMessage);
    }
    return 1;
}

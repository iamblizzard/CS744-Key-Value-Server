#include "../common_headers/main_server.h"

int ServerResponse_to_XML_server(Response* r, char * ans){
    if(r==NULL){
        return 0;
    }
    char * header = "<?xml version  = \"1.0\" encoding=\"UTF-8\"?>\n<KVMessage type=\"resp\">\n";
    char*startkey = "<Key>";
    char*endkey = "</Key>\n";
    char* startval = "<Value>";
    char*endval = "</Value>\n";
    char * endKVMessage = "</KVMessage>\n";
    char* startmessage = "<Message>";
    char * endmessage = "</Message>\n"; 
    ans[0] =0;
    strcpy(ans,header);
    if (r-> success){
         switch(r->typereq){
            case TDEL:
            case TPUT:                
                strcat(ans,startmessage);
                strcat(ans, "Success");
                strcat(ans, endmessage);
                break;
            case TGET:
                strcat(ans,startkey);
                strcat(ans,r->key);
                strcat(ans,endkey);
                strcat(ans,startval);
                strcat(ans,r->val);
                strcat(ans,endval);
                break;
        }
    }
    else{
        strcat(ans,startmessage);
        switch(r->typereq){
            case TPUT:
                strcat(ans, "Unknown Error: Unsuccesful Put");
                break;
            case TGET:
                strcat(ans, "Does not exist");

                break;
            case TDEL:
                strcat(ans, "Does not exist");
                break;
        }
        strcat(ans, endmessage);

    }
    strcat(ans,endKVMessage);
    // printf("%s\n",ans);
    return 1;
}

 void printreqattributes(Request * r){
     printf("Type of Request(2 for PUT, 3 for GET, 5 for DEL: %d\n",r->typereq);
     printf("Request key : %s\n",r->key);
     printf("Request val(only for PUT): %s\n",r->val);
    return;
 }

 int from_clientXML_to_Request(char * RequestXML, Request * ans) {
    if(strstr(RequestXML, "putreq") != NULL) {
        ans->typereq = TPUT;

        char* startKey = strstr(RequestXML, "<Key>");
        startKey = startKey + 5;
        char* endKey = strstr(RequestXML, "</Key>");
        strncpy(ans->key, startKey, endKey - startKey);
        ans->key[endKey - startKey] = 0;
        char* startValue = strstr(RequestXML, "<Value>");
        startValue = startValue + 7;
        char* endValue = strstr(RequestXML, "</Value>");
        strncpy(ans->val, startValue, endValue - startValue);
        ans->val[endValue - startValue] = 0;
        
    } else if(strstr(RequestXML, "getreq") != NULL) {
        ans->typereq = TGET;

        char* startKey = strstr(RequestXML, "<Key>");
        startKey = startKey + 5;
        char* endKey = strstr(RequestXML, "</Key>");
        strncpy(ans->key, startKey, endKey - startKey);
        ans->key[endKey - startKey] = 0;
        memset(ans->val, 0, sizeof(ans->val));

    } else if(strstr(RequestXML, "delreq") != NULL) {
        ans->typereq = TDEL;

        char* startKey = strstr(RequestXML, "<Key>");
        startKey = startKey + 5;
        char* endKey = strstr(RequestXML, "</Key>");
        strncpy(ans->key, startKey, endKey - startKey);
        ans->key[endKey - startKey] = 0;
        memset(ans->val, 0, sizeof(ans->val));

    } else {
        return 0;
    }

    return 1;
}

// char * temp =(char *)malloc(MAXXMLSIZE) ;
    // strcpy(temp,RequestXML);
    // char delim[] = "=";
    // if(temp == NULL) {
    //     printf("%s\n", RequestXML);
    // }
    // char*p = strtok(temp,delim);
    // p = strtok(NULL,delim);
    // p = strtok(NULL,delim);
    // p = strtok(NULL,delim);
    // if (p == NULL){ printf("%s\n", RequestXML);__assert(0, "e1");return 0; }
    // p = p+1;
    // if (!strncmp(p,"put",3)){
    //     ans->typereq = TPUT;
    //     char * key = "<Key>";
    //     char * s = strstr(p, key);
    //     s = s + strlen(key);
    //     char*keyend = strstr(s, "</Key>");;
    //     if (keyend == NULL){ 
    //         printf("%s\n", RequestXML);
    //         return 0;
    //     }
    //     strncpy(ans->key,s,keyend -s);
    //     (ans->key)[keyend-s] = 0;
    //     char * val = "<Value>";
    //     char * temp = strstr(p,val);
    //     temp = temp + strlen(val);
    //     val = temp;
    //     char * endval = strstr(val, "</Value>");
    //     strncpy(ans->val,val,endval-val);
    //     (ans->val)[endval-val] = 0;
    //     return 1;
    // }
    // else{
    //     if (!strncmp(p,"get",3)) ans->typereq = TGET;
    //     else ans->typereq = TDEL;
    //     strcpy(ans->val,"");
    //     char * key = "<Key>";
    //     char * s = strstr(p, key);
    //     s = s + strlen(key);
    //     char*keyend = strstr(p,"</Key>");
    //     if (keyend == NULL){ 
    //         printf("%s\n", RequestXML);
    //         return 0;
    //     }
    //     strncpy(ans->key,s,keyend -s);
    //     (ans->key)[keyend-s] = 0;
    //     return 1 ;
    // }
 

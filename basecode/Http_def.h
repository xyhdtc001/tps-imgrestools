#ifndef HTTP_DEF_H
#define  HTTP_DEF_H


static const char* const Http_def_h_Version =
    "$Id: Http_def.h,v 1.1.1.1 2007/04/20 09:18:07 develop Exp $";

#define HTTP_VERSION    "VPP/1.0" // Vovida Provisioning Protocol
#define CONT_LENGTH     "Content-Length:"
#define VPPREQ_BUF_LEN  56
#define URL_LENGTH       100
#define CONT_STR_LENGTH  56
#define VPPVER_LENGTH   56
#define GET_REQ          "GET"         // get an item
#define PUT_REQ          "PUT"         // put an item
#define DIRLIST_REQ      "LIST"        // list items in a group
#define DIRNLIST_REQ     "NLIST"       // list items with content-length
#define DELETE_REQ       "DELETE"      // delete an item
#define REMOVE_REQ       "REMOVE"      // remove a user
#define REGISTER_REQ     "REGISTER"
#define UNREGISTER_REQ   "UNREGISTER"
#define UPDATE_REQ       "UPDATE"
#define UPDATEDIR_REQ    "UPDATEDIR"
#define DELDIR_REQ       "DELDIR"

#define SYNC_GRLIST_REQ  "SYNCGRLIST"
#define SYNC_PRLIST_REQ  "SYNCPRLIST"
#define SYNC_GOOD_REQ    "SYNCGOOD"
#define SYNC_GET_REQ     "SYNCGET"
#define SYNC_PUT_REQ     "SYNCPUT"
#define SYNC_REGISTER_REQ     "SYNCREGISTER"
#define SYNC_UNREGISTER_REQ   "SYNCUNREGISTER"
#define SYNC_START_REQ   "SYNCSTART"
#define SYNC_STAT_REQ    "SYNCSTAT"


#endif  //HTTP_DEF_H

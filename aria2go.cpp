
#define TO_GID(gid_to_convert) aria2::A2Gid gid = (aria2::A2Gid) gid_to_convert;
#define TO_FILEDATA_POINTER(fileData_to_convert) aria2::FileData* fileData = (aria2::FileData*) fileData_to_convert;
#define TO_GLOBALSTATE_POINTER(globalState_to_convert) aria2::GlobalStat* globalStat = (aria2::GlobalStat*) globalState_to_convert;
#define TO_BTMI_POINTER(BTMI_to_convert) aria2::BtMetaInfoData* btMetaInfo = (aria2::BtMetaInfoData*) BTMI_to_convert;
#define TO_HANDLE_POINTER(handle_to_convert) aria2::DownloadHandle* handle = (aria2::DownloadHandle*) handle_to_convert;
#define ERROR_MESSAGE(message,code) {std::string error_message = message; error_message += std::to_string(code); throw error_message;}

#include "aria2go.h"
#include "aria2.h"
#include <string.h>

#include <iostream>

// C wrapper for go

aria2::A2Gid* current_gid_array = nullptr;
int current_gid_array_length = -1;
aria2::FileData* current_file_array = nullptr;
int current_file_array_length = -1;
char** current_uri_array = nullptr;
int current_uri_array_length = -1;
aria2::Session* session = nullptr;
std::vector<std::string> uris;

int downloadEventCallback(aria2::Session* s, aria2::DownloadEvent e,
                          aria2::A2Gid gid, void* userData){
    if(session == NULL || s != session) return 0;
    DownloadEvent event;
    switch (e) {
        case aria2::EVENT_ON_DOWNLOAD_START:
            event = EVENT_ON_DOWNLOAD_START;
            break;
        case aria2::EVENT_ON_DOWNLOAD_PAUSE:
            event = EVENT_ON_DOWNLOAD_PAUSE;
            break;
        case aria2::EVENT_ON_DOWNLOAD_STOP:
            event = EVENT_ON_DOWNLOAD_STOP;
            break;
        case aria2::EVENT_ON_DOWNLOAD_COMPLETE:
            event = EVENT_ON_DOWNLOAD_COMPLETE;
            break;
        case aria2::EVENT_ON_BT_DOWNLOAD_COMPLETE:
            event = EVENT_ON_BT_DOWNLOAD_COMPLETE;
            break;
        default:
            event = EVENT_ON_DOWNLOAD_ERROR;
    }
    runGoCallBack(event,(void*)gid);
    return 0;
}

void init_aria2go(){
    aria2::libraryInit();
}

void init_aria2go_session (int keep_running){
    aria2::SessionConfig config;
    config.downloadEventCallback = downloadEventCallback;
    config.keepRunning = keep_running;
    session = aria2::sessionNew(aria2::KeyVals(),config);
}

int run_aria2go(int run_mode){
    if(session == nullptr) return -1;
    if(run_mode){
        return aria2::run(session,aria2::RUN_ONCE);
    }else{
        return aria2::run(session,aria2::RUN_DEFAULT);
    }
}

void keepruning_aria2go(){
    if(session == nullptr) return;
    int i=1;
    while(i){
        i = aria2::run(session,aria2::RUN_ONCE);
    }
}

char* gidToHex_aria2go(void* g){
    if(!g) return nullptr;
    TO_GID(g)
    std::string h = aria2::gidToHex(gid);
    char* hex = new char[h.length()];
    strcpy(hex,h.c_str());
    return hex;
}

void* hexToGid_aria2go(char * s){
    if(s==nullptr){ return nullptr; /* Undefined String for Hex To Gid transform */ }
    return (void *) aria2::hexToGid(std::string (s));
}

int isNull_aria2go( void* g){
    if(!g) return true;
    return aria2::isNull( (aria2::A2Gid) g);
}

void* addUri_aria2go( char* uri, int position=-1){
    //TODO implement options
    if(uri==nullptr || !session){ return nullptr; /*throw "Undefined String for adding uri";*/}
    uris.push_back(std::string (uri));
    aria2::A2Gid gid;
    int error_code = aria2::addUri(session,&gid,uris,aria2::KeyVals(),position);
    if (error_code || aria2::isNull(gid)) ERROR_MESSAGE("Failed to add download uri",error_code)
    clear_uris();
    return (void *) gid;
}

int addMetalink_aria2go(char* file_location,int position=-1){
    if(current_gid_array!=NULL) delete current_gid_array;
    std::vector<aria2::A2Gid>* gids;
    int error_code = aria2::addMetalink(session,gids,std::string (file_location),aria2::KeyVals(),position);
    if(error_code || gids==NULL) ERROR_MESSAGE("Unable to add metalink",error_code)
    current_gid_array_length = gids->size();
    current_gid_array = gids->data();
    return current_gid_array_length;
}

void* get_element_gid(int index){
    if(index>=current_gid_array_length) throw "Out Of Index";
    return (void*)current_gid_array[index];
}

void add_uri(char* uri){
    uris.push_back(std::string (uri));   
}

void clear_uris(){
    uris.clear();
}

void* add_all_from_cache(int position=-1){
    //TODO implement options
    aria2::A2Gid gid;
    if(!session) return nullptr;
    int error_code = aria2::addUri(session,&gid,uris,aria2::KeyVals(),position);
    if (error_code || aria2::isNull(gid)) ERROR_MESSAGE("Failed to add download uri",error_code)
    clear_uris();
    return (void *) gid;    
}

void* addTorrent_aria2go(char* file,int position=-1){
    aria2::A2Gid gid;
    int error_code;
    if(!session) return nullptr;
    if(uris.size()>0){
        error_code = aria2::addTorrent(session,&gid,std::string (file),uris,aria2::KeyVals(),position);
    }else{
        error_code = aria2::addTorrent(session,&gid,std::string (file),aria2::KeyVals(),position);
    }
    if (error_code || aria2::isNull(gid)) ERROR_MESSAGE("Failed to add download uri",error_code)
    clear_uris();
    return (void *) gid;
}

int getActiveDownload_aria2go(){
    if(current_gid_array!=NULL) delete current_gid_array;
    if(!session) return -1;
    std::vector<aria2::A2Gid> gids = aria2::getActiveDownload(session);
    current_gid_array_length = gids.size();
    current_gid_array = gids.data();
    return current_gid_array_length;
}

int removeDownload_aria2go(void* g, int force){
    if(g==nullptr) return -1;
    TO_GID(g)
    if(aria2::isNull(gid)) return -1;
    int error_code = aria2::removeDownload(session,gid,force);
    return error_code;
}

int pauseDownload_aria2go(void* g, int force){
    if(g==nullptr || !session) return -1;
    TO_GID(g)
    if(aria2::isNull(gid)) return -1;
    int error_code = aria2::pauseDownload(session,gid,force);
    return error_code;
}

int unpauseDownload_aria2go(void* g){
    if(g==nullptr || !session) return -1;
    TO_GID(g)
    if(aria2::isNull(gid)) return -1;
    int error_code = aria2::unpauseDownload(session,gid);
    return error_code;
}

int finalize_aria2go(){
    if(session){
        aria2::shutdown(session);
        int r = aria2::sessionFinal(session);
        return r;
    }
    return -1;
}

int deinit_aria2go(){
    return aria2::libraryDeinit();
}

void* getDownloadHandle_aria2go(void* g){
    if(!g) return nullptr;
    TO_GID(g)
    if (aria2::isNull(gid)) return nullptr;
    aria2::DownloadHandle* h = aria2::getDownloadHandle(session,gid);
    return (void*) h;
}

enum DownloadStatus getStatus_gid(void* g){
    if(!g) return DOWNLOAD_ERROR;
    TO_GID(g)
    aria2::DownloadHandle* handle = aria2::getDownloadHandle(session,gid);
    aria2::DownloadStatus s = handle->getStatus();
    switch (s) {
        case aria2::DOWNLOAD_ACTIVE:
            return DOWNLOAD_ACTIVE;
        case aria2::DOWNLOAD_WAITING:
            return DOWNLOAD_WAITING;
        case aria2::DOWNLOAD_PAUSED:
            return DOWNLOAD_PAUSED;
        case aria2::DOWNLOAD_COMPLETE:
            return DOWNLOAD_COMPLETE;
        case aria2::DOWNLOAD_REMOVED:
            return DOWNLOAD_REMOVED;
        default:
            return DOWNLOAD_ERROR;
    }
    aria2::deleteDownloadHandle(handle);
}

long int getTotalLength_gid(void* g){
    if(!g) return -1;
    TO_GID(g)
    aria2::DownloadHandle* handle = aria2::getDownloadHandle(session,gid);
    long int r = handle->getTotalLength();
    aria2::deleteDownloadHandle(handle);
    return r;
}

long int getCompletedLength_gid(void* g){
    if(!g) return -1; 
    TO_GID(g)
    aria2::DownloadHandle* handle = aria2::getDownloadHandle(session,gid);
    long int r = handle->getCompletedLength();
    aria2::deleteDownloadHandle(handle);
    return r;
}

long int getUploadLength_gid(void* g){
    if(!g) return -1; 
    TO_GID(g)
    aria2::DownloadHandle* handle = aria2::getDownloadHandle(session,gid);
    long int r = handle->getUploadLength();
    aria2::deleteDownloadHandle(handle);
    return r;
}

char* getBitfield_gid(void* g){
    if(!g) return nullptr;
    TO_GID(g)
    aria2::DownloadHandle* handle = aria2::getDownloadHandle(session,gid);
    std::string b  = handle->getBitfield();
    char* bit_field = new char[b.length()];
    strcpy(bit_field,b.c_str());
    return bit_field;
}

int getDownloadSpeed_gid(void* g){
    if(!g) return -1; 
    TO_GID(g)
    aria2::DownloadHandle* handle = aria2::getDownloadHandle(session,gid);
    int r = handle->getDownloadSpeed();
    aria2::deleteDownloadHandle(handle);
    return r;
}

int getUploadSpeed_gid(void* g){
    if(!g) return -1; 
    TO_GID(g)
    aria2::DownloadHandle* handle = aria2::getDownloadHandle(session,gid);
    int r = handle->getUploadSpeed();
    aria2::deleteDownloadHandle(handle);
    return r;
}

char* getInfoHash_gid(void* g){
    if(!g) return nullptr;
    TO_GID(g)
    aria2::DownloadHandle* handle = aria2::getDownloadHandle(session,gid);
    const std::string s  = handle->getBitfield();
    char* str = new char[s.length()];
    strcpy(str,s.c_str());
    return str;
}

char* getDir_gid(void* g){
    if(!g) return nullptr;
    TO_GID(g)
    aria2::DownloadHandle* handle = aria2::getDownloadHandle(session,gid);
    const std::string s  = handle->getDir();
    char* str = new char[s.length()];
    strcpy(str,s.c_str());
    return str;
}


int getPieceLength_gid(void* g){
    if(!g) return -1; 
    TO_GID(g)
    aria2::DownloadHandle* handle = aria2::getDownloadHandle(session,gid);
    int r = (int)handle->getPieceLength();
    aria2::deleteDownloadHandle(handle);
    return r;
}

int getNumPieces_gid(void* g){
    if(!g) return -1; 
    TO_GID(g)
    aria2::DownloadHandle* handle = aria2::getDownloadHandle(session,gid);
    int r = (int)handle->getNumPieces();
    aria2::deleteDownloadHandle(handle);
    return r;
}

int getConnections_gid(void* g){
    if(!g) return -1; 
    TO_GID(g)
    aria2::DownloadHandle* handle = aria2::getDownloadHandle(session,gid);
    int r = (int)handle->getConnections();
    aria2::deleteDownloadHandle(handle);
    return r;
}

int getErrorCode_gid(void* g){
    if(!g) return -1; 
    TO_GID(g)
    aria2::DownloadHandle* handle = aria2::getDownloadHandle(session,gid);
    int r = (int)handle->getErrorCode();
    aria2::deleteDownloadHandle(handle);
    return r;
}

int getNumFiles_gid(void* g){
    if(!g) return -1; 
    TO_GID(g)
    aria2::DownloadHandle* handle = aria2::getDownloadHandle(session,gid);
    int r = (int)handle->getNumFiles();
    aria2::deleteDownloadHandle(handle);
    return r;
}

void* getBtMetaInfo_gid(void* g){
    if(!g) return nullptr; 
    TO_GID(g)
    aria2::DownloadHandle* handle = aria2::getDownloadHandle(session,gid);
    aria2::BtMetaInfoData* torrent_data = new aria2::BtMetaInfoData;
    try {
        *torrent_data = handle->getBtMetaInfo();
    }catch(const std::exception& e){
        delete torrent_data;
        aria2::deleteDownloadHandle(handle);
        return nullptr;
    }
    aria2::deleteDownloadHandle(handle);
    return (void*) torrent_data;
}

char* get_comment_BtMetaInfo(void* btmi){
    TO_BTMI_POINTER(btmi)
    char* comment = new char[btMetaInfo->comment.length()];
    strcpy(comment,btMetaInfo->comment.c_str());
    return comment;
}

long int get_creationDate_BtMetaInfo(void* btmi){
    TO_BTMI_POINTER(btmi)
    return static_cast<long int>(btMetaInfo->creationDate);
}

int get_mode_BtMetaInfo(void* btmi){
    TO_BTMI_POINTER(btmi)
    switch(btMetaInfo->mode){
        case aria2::BT_FILE_MODE_NONE:
            return -1;
        case aria2::BT_FILE_MODE_SINGLE:
            return 0;
        case aria2::BT_FILE_MODE_MULTI:
            return 1;
        default:
            return -2;
    }
}

char* get_name_BtMetaInfo(void* btmi){
    TO_BTMI_POINTER(btmi)
    char* name = new char[btMetaInfo->name.length()];
    strcpy(name,btMetaInfo->name.c_str());
    return name;
}

void* get_element_fileData(int index){
    if(index>=current_file_array_length) throw "Out Of Index";
    return (void*) &(current_file_array[index]);
}

int getFiles_gid(void* g){
    if(!g) return -1; 
    TO_GID(g)
    aria2::DownloadHandle* handle = aria2::getDownloadHandle(session,gid);
    if(current_file_array!=NULL) delete current_file_array;
    std::vector<aria2::FileData> files = handle->getFiles();
    deleteDownloadHandle(handle);
    current_file_array_length = files.size();
    current_file_array = files.data();
    return current_file_array_length;
}

int get_index_fileData(void* f){
    TO_FILEDATA_POINTER(f)
    return fileData->index;
}


char* get_path_fileData(void* f){
    TO_FILEDATA_POINTER(f)
    char* path = new char[fileData->path.length()];
    strcpy(path,fileData->path.c_str());
    return path;
}


long int get_length_fileData(void* f){
    TO_FILEDATA_POINTER(f)
    return (long int) fileData->length;
}

long int get_completedLength_fileData(void* f){
    TO_FILEDATA_POINTER(f)
    return (long int) fileData->completedLength;
}

int get_selected_fileData(void* f){
    TO_FILEDATA_POINTER(f)
    return fileData->selected;
}

void* getGlobalStat_aria2go(){
    aria2::GlobalStat* gs = new aria2::GlobalStat;
    *gs = aria2::getGlobalStat(session);
    return (void*)gs;
}

int get_downloadSpeed_globalStat(void* gs){
    TO_GLOBALSTATE_POINTER(gs)
    return globalStat->downloadSpeed;
}

int get_uploadSpeed_globalStat(void* gs){
    TO_GLOBALSTATE_POINTER(gs)
    return globalStat->uploadSpeed;
}

int get_numActive_globalStat(void* gs){
    TO_GLOBALSTATE_POINTER(gs)
    return globalStat->numActive;
}

int get_numWaiting_globalStat(void* gs){
    TO_GLOBALSTATE_POINTER(gs)
    return globalStat->numWaiting;
}

int get_numStopped_globalStat(void* gs){
    TO_GLOBALSTATE_POINTER(gs)
    return globalStat->numStopped;
}
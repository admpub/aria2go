#ifndef ARIA2_BRIDGE
#define ARIA2_BRIDGE

    #ifdef __cplusplus
    extern "C" {
    #endif
        void init_aria2go();
        void* init_aria2go_session ();
        int run_aria2go(void*);
        char* gidToHex_aria2go(void*);
        void* hexToGid_aria2go(char*);
        int isNull_aria2go(void*);
        void* addUri_aria2go(char*,int);
        int addMetalink_aria2go(char*,int);
        void* get_element_gid(int);
        void add_uri(char*);
        void clear_uris();
        void* add_all_from_cache(int);
        void* addTorrent_aria2go(char*,int);
        int getActiveDownload_aria2go();
        void removeDownload_aria2go(void*, int);
    #ifdef __cplusplus
    }
    #endif

#endif

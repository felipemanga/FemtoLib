#pragma once

#include <cstddef>
#include <LibLog>

#include "./ChaN/ffconf.h"
#include "./ChaN/fileff.h"

void initYAPFS();

struct FileInfo : public YAPFS::FILINFO {
    const char *name() const {
        return lfsize ? lfname : fname;
    }
};

class File {
    YAPFS::FIL handle;
public:
    inline static int error;

    File(){
        handle.fs = nullptr;
        initYAPFS();
    }

    File(const File&) = delete;

    File(File&& other) : handle(other.handle) {
        other.handle.fs = nullptr;
    }

    File(const char* name, bool create = false, bool append = false) {
        handle.fs = nullptr;
        openRW(name, create, append);
    }

    ~File(){
        close();
    }

    void close(){
        if(handle.fs){
            f_close(&handle);
            handle.fs = nullptr;
        }
    }

    operator bool(){
        return handle.fs;
    }

    static FileInfo stat(const char *name) {
        FileInfo info;
        error = f_stat(name, &info);
        return info;
    }

    File& openRO(const char *name) {
        close();
        error = f_open(&handle, name, FA_READ);
        return *this;
    }

    File& openRW(const char *name, bool create, bool append) {
        close();
        error = f_open(&handle, name, FA_READ|FA_WRITE|(create?FA_CREATE_ALWAYS:FA_OPEN_ALWAYS));
        if( !error && append){
            f_lseek(&handle, handle.fsize);
        }
        return *this;
    }

    uint32_t size(){
        return handle.fsize;
    }

    uint32_t tell(){
        return f_tell(&handle);
    }

    File& seek(uint32_t offset){
        f_lseek(&handle, offset);
        return *this;
    }

    uint32_t read( void *ptr, uint32_t count ){
        if(!*this) return 0;

        if( ((volatile uint32_t *) 0xE000ED00)[0] != 1947 ){
            YAPFS::UINT n = 0;
            error = f_read(&handle, ptr, count, &n);
            return n;
        }

        uint32_t total = 0;

        while(count){
            YAPFS::UINT n = 0;
            uint32_t chunk = count;
            if(chunk > 256)
                chunk = 256;
            count -= chunk;
            error = f_read(&handle, ptr, chunk, &n);
            total += n;
            if(error || n < chunk){
                LOG("Read Error\n");
            }
            ptr += n;
        }

        if(error)
            total = 0;
        return total;
    }

    uint32_t write( const void *ptr, uint32_t count ){
        if(!*this) return 0;
        uint32_t total = 0;

        if( ((volatile uint32_t *) 0xE000ED00)[0] != 1947 ){
            YAPFS::UINT n = 0;
            error = f_write(&handle, ptr, count, &n);
            return n;
        }

        while(count){
            YAPFS::UINT n;
            uint32_t chunk = count;
            if(chunk > 256)
                chunk = 256;
            count -= chunk;
            error = f_write(&handle, const_cast<void*>(ptr), chunk, &n);
            total += n;
            if(error){
                LOG("Write Error\n");
            }
            ptr += chunk;
        }

        if(error)
            total = 0;
        else if(total >= 256)
            f_sync(&handle);
        return total;
    }

    template< typename T, size_t S > uint32_t read( T (&data)[S] ){
	    return read( data, sizeof(data) );
    }

    template< typename T, size_t S > uint32_t write( const T (&data)[S] ){
	    return write( data, sizeof(data) );
    }

    template< typename T >
    T read(){
        T tmp = {};
        read( (void*) &tmp, sizeof(T) );
        return tmp;
    }

    template< typename T >
    File & operator >> (T &i){
    	read(&i, sizeof(T));
    	return *this;
    }

    template< typename T >
    File & operator << (const T& i){
    	write(&i, sizeof(T));
    	return *this;
    }

    File & operator << (const char *str ){
        uint32_t len = 0;
        while(str[len]) len++;
        write(str, len);
    	return *this;
    }
};

class Directory {
    YAPFS::FATFS_DIR handle;
    FileInfo fileInfo;

public:
    Directory(){
        handle.fs = nullptr;
        initYAPFS();
    }

    Directory(const Directory&) = delete;

    operator bool(){
        return handle.fs;
    }

    Directory &open(const char *path){
        File::error = f_opendir(&handle, path);
        return *this;
    }

    FileInfo *read() {
        fileInfo.fname[0] = 0;
        fileInfo.lfsize = 0;
        File::error = f_readdir(&handle, &fileInfo);
        return File::error != YAPFS::FR_OK || fileInfo.fname[0] == 0 ? nullptr : &fileInfo;
    }

    template <typename Callback>
    FileInfo *read(Callback filter) {
        FileInfo *ret;
        do{
            ret = read();
        }while(ret && !filter(fileInfo));
        return ret;
    }
};

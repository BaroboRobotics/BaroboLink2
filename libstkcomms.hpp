#ifndef _LIBSTKCOMMS_H_
#define _LIBSTKCOMMS_H_
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include "thread_macros.h"


class CHexFile
{
  public:
    CHexFile();
    CHexFile(const char* filename);
    int loadFile(const char* filename);
    inline int len() {
      return _len;
    }
    ~CHexFile();
    uint8_t getByte(int index);
  private:
    uint8_t *_data;
    int _dataAllocSize;
    int _len;
    void realloc();
    void parseLine(const char* line);
};

class CStkComms
{
  public:
  CStkComms();
  ~CStkComms();
  int connect(const char addr[]);
  int setSocket(int socket);
  int programAll(const char* hexFileName);
  int programAll(const char* hexFileName, int hwRev);
  int programAllAsync(const char* hexFileName);
  int programAllAsync(const char* hexFileName, int hwRev);
  double getProgress();
  inline int isProgramComplete() {
    return _programComplete;
  }
  int disconnect();
  int handshake();
  int setDevice(
      uint8_t     DeviceCode=  0x86,
      uint8_t     Revision=     0x00,
      uint8_t     progtype=     0x00,
      uint8_t     parmode=      0x01,
      uint8_t     polling=      0x01,
      uint8_t     selftimed=    0x01,
      uint8_t     lockbytes=    0x01,
      uint8_t     fusebytes=    0x03,
      uint8_t     flashpollval1=0xff,
      uint8_t     flashpollval2=0xff,
      uint8_t     eeprompollval1=0xff,
      uint8_t     eeprompollval2=0xff,
      uint8_t     pagesizehigh= 0x00,
      uint8_t     pagesizelow=  0x80,
      uint8_t     eepromsizehigh=0x04,
      uint8_t     eepromsizelow=0x00,
      uint8_t     flashsize4=   0x00,
      uint8_t     flashsize3=   0x00,
      uint8_t     flashsize2=   0x80,
      uint8_t     flashsize1=   0x00);
  int setDeviceExt(
      uint8_t    commandsize=  0x05,
      uint8_t    eeprompagesize= 0x04,
      uint8_t    signalpagel=  0xd7,
      uint8_t    signalbs2=    0xc2,
      uint8_t    resetdisable= 0x00);
  int enterProgMode();
  int leaveProgMode();
  int checkSignature();
  int loadAddress(uint16_t address);
  int progHexFile(const char* filename);
  int progPage(uint8_t* data, uint16_t size);
  int progFuses();
  int readData(uint16_t address, uint8_t *byte);
  int writeData(uint16_t address, uint8_t byte);
  int checkFlash(const char* filename);
  int checkPage(CHexFile* hexfile, uint16_t address, uint16_t size = 0x80);
  int universal(uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4);

  private:
  int _socket;
  bool _isConnected;
  int _programComplete;
#ifndef _WIN32
  struct sockaddr_rc _addr;
#else
  SOCKADDR_BTH _addr;
#endif
  uint8_t _signature[3];

  MUTEX_T* _progressLock;
  COND_T* _progressCond;
  double _progress;

  int sendBytes(void* buf, size_t len);
  int recvBytes(uint8_t* buf, size_t expectedBytes, size_t size);
  int recvBytes(uint8_t* buf, size_t size);

  int setdtr(int on);
};

enum hexLineType_e
{
  HEXLINE_DATA,
  HEXLINE_EOF,
  HEXLINE_EXADDR,
  HEXLINE_STARTSEGMENTADDR,
  HEXLINE_EXTENDEDLINEARADDR,
  HEXLINE_STARTLINEARADDR
};
#endif

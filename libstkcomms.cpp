#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "libstkcomms.hpp"
#include "command.h"
#include "thread_macros.h"

#define DEBUG

#ifdef DEBUG
#define THROW throw
#else
#define THROW
#endif

CStkComms::CStkComms()
{
  _isConnected = false;
  _programComplete = 0;
  MUTEX_NEW(_progressLock);
  MUTEX_INIT(_progressLock);
  COND_NEW(_progressCond);
  COND_INIT(_progressCond);
}

CStkComms::~CStkComms()
{
  free(_progressLock);
  free(_progressCond);
}

int CStkComms::connect(const char addr[])
{
  int err = 0;
  int flags;
#ifndef _WIN32
  _socket = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
#else
  _socket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
#endif

#ifdef _WIN32
  if(comms->socket == INVALID_SOCKET) {
    err = WSAGetLastError();
    printf("Could not bind to socket. Error %d\n", err);
    if(err == 10047) {
      return -5;
    } else {
      return -1;
    }
  }
#else
  if(_socket == -1) {
    fprintf(stderr, "Could not bind to socket. %d\n", errno);
    return -1;
  }
#endif
  // set the connection parameters (who to connect to)
#ifndef _WIN32
  _addr.rc_family = AF_BLUETOOTH;
  _addr.rc_channel = (uint8_t) 1;
  str2ba( addr, &_addr.rc_bdaddr );
#else
  _addr.addressFamily = AF_BTH;
  str2ba( address, (bdaddr_t*)&_addr.btAddr);
  _addr.port = channel;
#endif

  // connect to server
  int status;
  status = -1;
  int tries = 0;
  while(status < 0) {
    if(tries > 2) {
      break;
    }
    status = ::connect(_socket, (const struct sockaddr *)&_addr, sizeof(_addr));
    if(status == 0) {
      _isConnected = 1;
    } 
    tries++;
  }
  if(status < 0) {
#ifndef _WIN32
    perror("Error connecting.");
#else
	  LPVOID lpMsgBuf;
	  FormatMessage( 
		  FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		  FORMAT_MESSAGE_FROM_SYSTEM | 
		  FORMAT_MESSAGE_IGNORE_INSERTS,
		  NULL,
		  GetLastError(),
		  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		  (LPTSTR) &lpMsgBuf,
		  0,
		  NULL 
		  );
	  // Process any inserts in lpMsgBuf.
	  // ...
	  // Display the string.
	  //MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );
	  fprintf(stderr, "Error Connecting: %s", lpMsgBuf);
    int wsaerror = WSAGetLastError();
	  if(wsaerror == 10048) {
		  fprintf(stderr, "Make sure there are no other programs currently connected to the Mobot.\n");
	  } else if (wsaerror == 10047 || wsaerror == 10050) {
      fprintf(stderr, "A bluetooth device could not be found on this computer. You may need to attach\nan external Bluetooth dongle to continue.\n");
      err = -5;
    }
	  // Free the buffer.
	  LocalFree( lpMsgBuf );
#endif
    return err;
  }
  /* Make the socket non-blocking */
  flags = fcntl(_socket, F_GETFL, 0);
  fcntl(_socket, F_SETFL, flags | O_NONBLOCK);
  setdtr(1);
  sleep(1);
  setdtr(0);
  return 0;
}

int CStkComms::disconnect()
{
  close(_socket);
}

int CStkComms::setSocket(int socket)
{
  int flags;
  _socket = socket;
  /* Make the socket non-blocking */
  flags = fcntl(_socket, F_GETFL, 0);
  fcntl(_socket, F_SETFL, flags | O_NONBLOCK);
  _isConnected = 1;
  return 0;
}

int CStkComms::programAll(const char* hexFileName)
{
  if(handshake()) {
    THROW;
    return -1;
  }
  if(setDevice()) {
    THROW;
    return -1;
  }
  if(setDeviceExt()) {
    THROW;
    return -1;
  }
  if(enterProgMode()) {
    THROW;
    return -1;
  }
  if(checkSignature()) {
    THROW;
    return -1;
  }
  if(progFuses()) {
    THROW;
    return -1;
  }
  if(progHexFile(hexFileName)) {
    THROW;
    return -1;
  }
  if(checkFlash(hexFileName)) {
    THROW;
    return -1;
  }
  if(leaveProgMode()) {
    THROW;
    return -1;
  }
  _progress = 1.1;
  _programComplete = 1;
  return 0;  
}

int CStkComms::programAll(const char* hexFileName, int hwRev)
{
  if(handshake()) {
    THROW;
    return -1;
  }
  if(setDevice()) {
    THROW;
    return -1;
  }
  if(setDeviceExt()) {
    THROW;
    return -1;
  }
  if(enterProgMode()) {
    THROW;
    return -1;
  }
  if(checkSignature()) {
    THROW;
    return -1;
  }
  /*
  if(writeData(0x11, hwRev)) {
    THROW;
    return -1;
  }
  */
  if(progFuses()) {
    THROW;
    return -1;
  }
  if(progHexFile(hexFileName)) {
    THROW;
    return -1;
  }
  if(checkFlash(hexFileName)) {
    THROW;
    return -1;
  }
  if(leaveProgMode()) {
    THROW;
    return -1;
  }
  _progress = 1.1;
  _programComplete = 1;
  return 0;  
}

struct programAllThreadArg{
  const char* hexFileName;
  CStkComms* stkComms;
  int hwRev;
};

void* programAllThread(void* arg)
{
  programAllThreadArg *a = (programAllThreadArg*)arg;
  if(a->hwRev == 0) {
    a->stkComms->programAll(a->hexFileName);
  } else {
    a->stkComms->programAll(a->hexFileName, a->hwRev);
  }
  return NULL;
}

int CStkComms::programAllAsync(const char* hexFileName)
{
  THREAD_T thread;
  struct programAllThreadArg *a;
  a = (struct programAllThreadArg*)malloc(sizeof(struct programAllThreadArg));
  a->hexFileName = hexFileName;
  a->stkComms = this;
  a->hwRev = 0;
  _progress = 0.01;
  THREAD_CREATE(&thread, programAllThread, a);
  return 0;
}

int CStkComms::programAllAsync(const char* hexFileName, int hwRev)
{
  THREAD_T thread;
  struct programAllThreadArg *a;
  a = (struct programAllThreadArg*)malloc(sizeof(struct programAllThreadArg));
  a->hexFileName = hexFileName;
  a->stkComms = this;
  a->hwRev = hwRev;
  _progress = 0.01;
  THREAD_CREATE(&thread, programAllThread, a);
  return 0;
}

double CStkComms::getProgress()
{
  double progress;
  MUTEX_LOCK(_progressLock);
  progress = _progress;
  MUTEX_UNLOCK(_progressLock);
  return progress;
}

int CStkComms::handshake()
{
  uint8_t buf[10];
  int len;
  while(1) {
    buf[0] = Cmnd_STK_GET_SYNC;
    buf[1] = Sync_CRC_EOP;
    sendBytes(buf, 2);
    /* Wait a second and try to read */
    sleep(1);
    len = recvBytes(buf, 10);
    if(len == 2) {break;}
  }
  if(len == 2 && buf[0] == Resp_STK_INSYNC )
    return 0;
  perror("Recv error");
  return -1;
}

int CStkComms::setDevice(
      uint8_t     DeviceCode,
      uint8_t     Revision,
      uint8_t     progtype,
      uint8_t     parmode,
      uint8_t     polling,
      uint8_t     selftimed,
      uint8_t     lockbytes,
      uint8_t     fusebytes,
      uint8_t     flashpollval1,
      uint8_t     flashpollval2,
      uint8_t     eeprompollval1,
      uint8_t     eeprompollval2,
      uint8_t     pagesizehigh,
      uint8_t     pagesizelow,
      uint8_t     eepromsizehigh,
      uint8_t     eepromsizelow,
      uint8_t     flashsize4,
      uint8_t     flashsize3,
      uint8_t     flashsize2,
      uint8_t     flashsize1)
{
  uint8_t buf[30];
  buf[0] = Cmnd_STK_SET_DEVICE;
  buf[1] =      DeviceCode;
  buf[2] =      Revision;
  buf[3] =      progtype;
  buf[4] =      parmode;
  buf[5] =      polling;
  buf[6] =      selftimed;
  buf[7] =      lockbytes;
  buf[8] =      fusebytes;
  buf[9] =      flashpollval1;
  buf[10] =      flashpollval2;
  buf[11] =      eeprompollval1;
  buf[12] =      eeprompollval2;
  buf[13] =      pagesizehigh;
  buf[14] =      pagesizelow;
  buf[15] =      eepromsizehigh;
  buf[16] =      eepromsizelow;
  buf[17] =      flashsize4;
  buf[18] =      flashsize3;
  buf[19] =      flashsize2;
  buf[20] =      flashsize1;
  buf[21] = Sync_CRC_EOP;
  int rc;
  if(rc = sendBytes(buf, 22)) {
    THROW;
  }
  rc = recvBytes(buf, 2, 25);
  if(rc != 2) {
    THROW;
  }
  if(buf[0] != Resp_STK_INSYNC) {
    THROW;
  }
  if(buf[1] != Resp_STK_OK) {
    THROW;
  }
  return 0;
}

int CStkComms::setDeviceExt(
      uint8_t    commandsize,
      uint8_t    eeprompagesize,
      uint8_t    signalpagel,
      uint8_t    signalbs2,
      uint8_t    resetdisable)
{
  uint8_t buf[10];
  buf[0] = Cmnd_STK_SET_DEVICE_EXT;
  buf[1] = commandsize;
  buf[2] = eeprompagesize;
  buf[3] = signalpagel;
  buf[4] = signalbs2;
  buf[5] = resetdisable;
  buf[6] = Sync_CRC_EOP;
  int rc;
  if(rc = sendBytes(buf, 7)) {
    THROW;
    return rc;
  }
  rc = recvBytes(buf, 2, 10);
  if(rc != 2) {
    THROW;
    return -1;
  }
  if(buf[0] != Resp_STK_INSYNC) {
    THROW;
    return -1;
  }
  if(buf[1] != Resp_STK_OK) {
    THROW;
    return -1;
  }
  return 0;
}

int CStkComms::enterProgMode()
{
  uint8_t buf[10];
  buf[0] = Cmnd_STK_ENTER_PROGMODE;
  buf[1] = Sync_CRC_EOP;
  int rc;
  if(rc = sendBytes(buf, 2)) {
    return rc;
  }
  rc = recvBytes(buf, 2, 10);
  if(rc != 2) {
    return -1;
  }
  if(buf[0] != Resp_STK_INSYNC) {
    return -1;
  }
  if(buf[1] != Resp_STK_OK) {
    return -1;
  }
  return 0;
}

int CStkComms::leaveProgMode()
{
  uint8_t buf[10];
  buf[0] = Cmnd_STK_LEAVE_PROGMODE;
  buf[1] = Sync_CRC_EOP;
  int rc;
  if(rc = sendBytes(buf, 2)) {
    return rc;
  }
  rc = recvBytes(buf, 2, 10);
  if(rc != 2) {
    return -1;
  }
  if(buf[0] != Resp_STK_INSYNC) {
    return -1;
  }
  if(buf[1] != Resp_STK_OK) {
    return -1;
  }
  return 0;
}

int CStkComms::checkSignature()
{
  uint8_t buf[10];
  buf[0] = Cmnd_STK_READ_SIGN;
  buf[1] = Sync_CRC_EOP;
  int rc;
  if(rc = sendBytes(buf, 2)) {
    return rc;
  }
  rc = recvBytes(buf, 5, 10);
  if(rc != 5) {
    return -1;
  }
  if(buf[0] != Resp_STK_INSYNC) {
    return -1;
  }
  if(buf[4] != Resp_STK_OK) {
    return -1;
  }
  memcpy(&_signature[0], &buf[1], 3);
  buf[0] = 0x1e;
  buf[1] = 0x95;
  buf[2] = 0x0f;
  if(memcmp(_signature, buf, 3)) {
    fprintf(stderr, "Device Signature incorrect.\n");
    return -1;
  }
  return 0;
}

int CStkComms::loadAddress(uint16_t address)
{
  uint8_t buf[10];
  buf[0] = Cmnd_STK_LOAD_ADDRESS;
  buf[1] = (address&0xFF);
  buf[2] = address>>8;
  buf[3] = Sync_CRC_EOP;
  int rc;
  if(rc = sendBytes(buf, 4))
  {
    return -1;
  }
  rc = recvBytes(buf, 2, 4);
  if(rc != 2)
  {
    THROW;
    return -1;
  }
  if(buf[0] != Resp_STK_INSYNC) {
    THROW;
    return -1;
  }
  if(buf[1] != Resp_STK_OK) {
    THROW;
    return -1;
  }
  return 0;
}

int CStkComms::progHexFile(const char* filename)
{
  CHexFile *file = new CHexFile(filename);
  uint16_t pageSize = 128;
  int i;
  /* Program the file one 128-byte page at a time */
  uint8_t* buf = new uint8_t[pageSize + 10];
  uint16_t address = 0; // The address adresses 2-byte locations
  while(address*2 < file->len())
  {
    loadAddress(address);
    for(
        i = 0; 
        (i < pageSize) && ((address*2 + i) < file->len()); 
        i++) 
    {
      buf[i] = file->getByte(address*2 + i);
    }
    progPage(buf, i);
    address += pageSize/2;
    /* Update the progress tracker */
    MUTEX_LOCK(_progressLock);
    _progress = 0.5 * ((double)address*2) / (double)file->len();
    COND_SIGNAL(_progressCond);
    MUTEX_UNLOCK(_progressLock);
  }
  delete file;
  delete[] buf;
  return 0;
}

int CStkComms::checkFlash(const char* filename)
{
  CHexFile *hf = new CHexFile(filename);
  int i;
  uint16_t addrIncr = 0x40;
  uint16_t pageSize = 0x80;
  for(i = 0; i*2 < hf->len(); i += addrIncr)
  {
    if(checkPage(hf, i, i*2+pageSize > hf->len()? hf->len() - i*2 : pageSize))
    {
      THROW;
      return -1;
    }
    /* Update the progress tracker */
    MUTEX_LOCK(_progressLock);
    _progress = 0.5 + 0.5 * ((double)i*2) / (double)hf->len();
    COND_SIGNAL(_progressCond);
    MUTEX_UNLOCK(_progressLock);
  }
  MUTEX_LOCK(_progressLock);
  _progress = 1;
  COND_SIGNAL(_progressCond);
  MUTEX_UNLOCK(_progressLock);
  return 0;
}

int CStkComms::checkPage(CHexFile* hexfile, uint16_t address, uint16_t size)
{
  /* First, load the address */
  int rc;
  if(rc = loadAddress(address)) {
    THROW;
    return rc;
  }
  uint8_t* buf = new uint8_t[size+10];
  /* Send the "Get Page" command */
  buf[0] = Cmnd_STK_READ_PAGE;
  buf[1] = size >> 8;
  buf[2] = size & 0xFF;
  buf[3] = 'F';
  buf[4] = Sync_CRC_EOP;
  sendBytes(buf, 5);
  /* Now, get the data */
  rc = recvBytes(buf, size+2, size+10);
  if(rc != size+2) {
    THROW;
    delete[] buf;
    return -1;
  }
  /* Compare with the hex file */
  int i, startIndex;
  startIndex = address*2;
  for(i = 0; i < size; i++) {
    if(hexfile->getByte(startIndex+i) != buf[i+1]) {
      THROW;
      delete[] buf;
      return -1;
    }
  }
  delete[] buf;
  return 0;
}

int CStkComms::progPage(uint8_t* data, uint16_t size)
{
  uint8_t *buf = new uint8_t[size + 10];
  buf[0] = Cmnd_STK_PROG_PAGE;
  buf[1] = size >> 8;
  buf[2] = size & 0xFF;
  buf[3] = 'F';
  memcpy(&buf[4], data, size);
  buf[size+4] = Sync_CRC_EOP;
  int rc;
  if(rc = sendBytes(buf, size+5)) {
    return rc;
  }
  rc = recvBytes(buf, 2, size);
  if(rc != 2) {
    return -1;
  }
  if(buf[0] != Resp_STK_INSYNC) {
    return -1;
  } 
  if(buf[1] != Resp_STK_OK) {
    return -1;
  }
  return 0;
}

int CStkComms::progFuses()
{
  universal( 0xa0 , 0x03 , 0xfc , 0x00 );
  universal( 0xa0 , 0x03 , 0xfd , 0x00 );
  universal( 0xa0 , 0x03 , 0xfe , 0x00 );
  universal( 0xa0 , 0x03 , 0xff , 0x00 );
  return 0;
}

int CStkComms::readData(uint16_t address, uint8_t *byte)
{
  int rc;
  if(rc = loadAddress(address)) {
    THROW;
    return -1;
  }
  uint8_t buf[10];
  buf[0] = Cmnd_STK_READ_DATA;
  buf[1] = Sync_CRC_EOP;
  if(rc = sendBytes(buf, 2)) {
    THROW;
    return -1;
  }
  rc = recvBytes(buf, 3, 10);
  if(rc != 3) {
    THROW;
    return -1;
  }
  *byte = buf[1];
  return 0;
}

int CStkComms::writeData(uint16_t address, uint8_t byte)
{
  int rc;
  if(rc = loadAddress(address)) {
    THROW;
    return -1;
  }
  uint8_t buf[10];
  buf[0] = Cmnd_STK_PROG_DATA;
  buf[1] = byte;
  buf[2] = Sync_CRC_EOP;
  if(rc = sendBytes(buf, 3)) {
    THROW;
    return -1;
  }
  rc = recvBytes(buf, 2, 10);
  if(rc != 2) {
    THROW;
    return -1;
  }
  return 0;
}

int CStkComms::universal(uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4)
{
  uint8_t buf[10];
  buf[0] = Cmnd_STK_UNIVERSAL;
  buf[1] = byte1;
  buf[2] = byte2;
  buf[3] = byte3;
  buf[4] = byte4;
  buf[5] = Sync_CRC_EOP;
  int rc;
  if(rc = sendBytes(buf, 6)) {
    return rc;
  }
  rc = recvBytes(buf, 3, 10);
  if(rc != 3) {
    THROW;
    return rc;
  }
  return 0;
}

int CStkComms::sendBytes(void* buf, size_t len)
{
  if(!_isConnected) {
    return -1;
  }

#ifdef VERBOSE
  printf("SEND: ");
  for(int i = 0; i < len; i++) {
    printf("0x%02X ", ((uint8_t*)buf)[i]);
  }
  printf("\n");
#endif
  if(write(_socket, buf, len) == -1) {
    perror("Write error");
    return -1;
  } 
  return 0;
}

int CStkComms::recvBytes(uint8_t* buf, size_t expectedBytes, size_t size)
{
  int len = 0;
  int rc;
  uint8_t *mybuf = new uint8_t[size];
  
  while(len < expectedBytes) {
    rc = read(_socket, mybuf, size);
    if(rc > 0) {
      memcpy(&buf[len], mybuf, rc);
      len += rc;
      if(buf[0] != Resp_STK_INSYNC) {
        THROW;
      }
    }
    //usleep(100000);
  }
#ifdef VERBOSE
  printf("Recv: ");
  for(int i = 0; i < len; i++) {
    printf("0x%02X ", ((uint8_t*)buf)[i]);
  }
  printf("\n");
#endif
  delete[] mybuf;
  return len;
}

int CStkComms::recvBytes(uint8_t* buf, size_t size)
{
  int len = 0;
  
  len = read(_socket, buf, size);
#ifdef DEBUG
  printf("Recv: ");
  for(int i = 0; i < len; i++) {
    printf("0x%2X ", ((uint8_t*)buf)[i]);
  }
  printf("\n");
#endif
  return len;
}

int CStkComms::setdtr (int on)
{
  int controlbits = TIOCM_DTR;
  if(on)
    return(ioctl(_socket, TIOCMBIC, &controlbits));
  else
    return(ioctl(_socket, TIOCMBIS, &controlbits));
} 

CHexFile::CHexFile()
{
  _data = NULL;
  _dataAllocSize = 0;
  _len = 0;
}

CHexFile::CHexFile(const char* filename)
{
  _data = NULL;
  _dataAllocSize = 0;
  _len = 0;
  loadFile(filename);
}

CHexFile::~CHexFile()
{
  if(_data) {
    delete[] _data;
  }
  _dataAllocSize = 0;
  _len = 0;
}

uint8_t CHexFile::getByte(int index)
{
  if((index < 0) || (index >= _len)) {
    THROW;
  }
  return _data[index];
}

int CHexFile::loadFile(const char* filename)
{
  FILE* fp = fopen(filename, "r");
  if(fp == NULL) {
    return -1;
  }
  char buf[128];
  char *str;
  str = fgets(buf, 128, fp);
  while(str != NULL) {
    parseLine(str);
    str = fgets(buf, 128, fp);
  }
  fclose(fp);
  return 0;
}

void CHexFile::realloc()
{
  int incrSize = 256;
  if(_dataAllocSize == 0) {
    _data = new uint8_t[incrSize];
  } else {
    uint8_t* newData = new uint8_t[incrSize + _dataAllocSize];
    memcpy(newData, _data, _len);
    delete[] _data;
    _data = newData;
  }
  _dataAllocSize += incrSize;
}

void CHexFile::parseLine(const char* line)
{
  int byteCount = 0;
  unsigned int address;
  int type;
  unsigned int value;
  unsigned int checksum;
  const char* str;
  int i;
  // Data format taken from http://en.wikipedia.org/wiki/Intel_HEX
  // Ensure that the first character is a ':'
  if(line[0] != ':') {
    THROW;
  }
  char buf[10];
  // Get the first two hex chars
  memset(buf, 0, sizeof(char)*10);
  strncpy(buf, &line[1], 2);
  sscanf(buf, "%x", &byteCount);
  // Get the next four hex chars 
  memset(buf, 0, sizeof(char)*10);
  strncpy(buf, &line[3], 4);
  sscanf(buf, "%x", &address);
  // Next two chars are the type
  memset(buf, 0, sizeof(char)*10);
  strncpy(buf, &line[7], 2);
  sscanf(buf, "%x", &type);

  /* Now we need to check the type. If the type is data, make sure we have
   * enough space in our buffer and read the data */
  if(type != HEXLINE_DATA) {
    return;
  }

  /* Check size */
  while(_len + byteCount >= _dataAllocSize) {
    realloc();
  }
  uint8_t checktest = 0;
  checktest = byteCount + (uint8_t)(address>>8) + (uint8_t)(address&0xFF) + (uint8_t)type;
  memset(buf, 0, sizeof(char)*10);
  str = &line[9];
  for(i = 0; i < byteCount; i++) {
    strncpy(buf, str, 2);
    sscanf(buf, "%x", &value);
    _data[_len] = value;
    checktest += value;
    _len++;
    str += 2;
  }
  sscanf(str, "%x", &checksum);
  // 2's complement the checktest 
  checktest = (checktest ^ 0xFF) + 0x01;
  if(checktest != checksum) 
    THROW;
}

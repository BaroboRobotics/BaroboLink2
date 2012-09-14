#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#ifndef _WIN32
#include <sys/ioctl.h>
#else
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <Ws2bth.h>
#endif
#include "libstkcomms.hpp"
#include "command.h"
#include "../thread_macros.h"

//#define DEBUG

#ifdef DEBUG
#define THROW throw
#else
#define THROW
#endif

stkComms_t* stkComms_new()
{
	stkComms_t* comms;
	comms = (stkComms_t*)malloc(sizeof(stkComms_t));
	return comms;
}

int stkComms_init(stkComms_t* comms)
{
  comms->isConnected = false;
  comms->programComplete = 0;
  MUTEX_NEW(comms->progressLock);
  MUTEX_INIT(comms->progressLock);
  COND_NEW(comms->progressCond);
  COND_INIT(comms->progressCond);
  return 0;
}

int stkComms_destroy(stkComms_t* comms)
{
  free(comms->progressLock);
  free(comms->progressCond);
  return 0;
}

int stkComms_connect(stkComms_t* comms, const char addr[])
{
  int err = 0;
  int flags;
#ifndef _WIN32
  comms->socket = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
#else
  comms->socket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
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
  if(comms->socket == -1) {
    fprintf(stderr, "Could not bind to socket. %d\n", errno);
    return -1;
  }
#endif
  // set the connection parameters (who to connect to)
#ifndef _WIN32
  comms->addr.rc_family = AF_BLUETOOTH;
  comms->addr.rc_channel = (uint8_t) 1;
  str2ba( addr, &comms->addr.rc_bdaddr );
#else
  comms->addr.addressFamily = AF_BTH;
  str2ba( addr, (bdaddr_t*)&comms->addr.btAddr);
  comms->addr.port = 1;
#endif

  // connect to server
  int status;
  status = -1;
  int tries = 0;
  while(status < 0) {
    if(tries > 2) {
      break;
    }
    status = ::connect(comms->socket, (const struct sockaddr *)&comms->addr, sizeof(comms->addr));
    if(status == 0) {
      comms->isConnected = 1;
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
#ifndef _WIN32
  flags = fcntl(comms->socket, F_GETFL, 0);
  fcntl(comms->socket, F_SETFL, flags | O_NONBLOCK);
  stkComms_setdtr(comms, 1);
  sleep(1);
  stkComms_setdtr(comms, 0);
#else
  ioctlsocket(comms->socket, FIONBIO, (u_long*)1);
#endif
  return 0;
}

int stkComms_disconnect(stkComms_t* comms)
{
#ifndef _WIN32
  return close(comms->socket);
#else
  return closesocket(comms->socket);
#endif
}

int stkComms_setSocket(stkComms_t* comms, int socket)
{
  int flags;
  comms->socket = socket;
  /* Make the socket non-blocking */
#ifndef _WIN32
  flags = fcntl(comms->socket, F_GETFL, 0);
  fcntl(comms->socket, F_SETFL, flags | O_NONBLOCK);
#else
  ioctlsocket(comms->socket, FIONBIO, (u_long*)1);
#endif
  comms->isConnected = 1;

  return 0;
}

double stkComms_getProgress(stkComms_t* comms)
{
  double progress;
  MUTEX_LOCK(comms->progressLock);
  progress = comms->progress;
  MUTEX_UNLOCK(comms->progressLock);
  return progress;
}

void stkComms_setProgress(stkComms_t* comms, double progress)
{
	comms->progress = progress;
}

int stkComms_isProgramComplete(stkComms_t* comms) 
{
  return comms->programComplete;
}

void stkComms_setProgramComplete(stkComms_t* comms, int complete)
{
	comms->programComplete = complete;
}

int stkComms_handshake(stkComms_t* comms)
{
  uint8_t buf[10];
  int len;
  while(1) {
    buf[0] = Cmnd_STK_GET_SYNC;
    buf[1] = Sync_CRC_EOP;
    stkComms_sendBytes(comms, buf, 2);
    /* Wait a second and try to read */
#ifndef _WIN32
    sleep(1);
#else
    Sleep(1000);
#endif
    len = stkComms_recvBytes2(comms, buf, 10);
    if(len == 2) {break;}
  }
  if(len == 2 && buf[0] == Resp_STK_INSYNC )
    return 0;
  perror("Recv error");
  return -1;
}

int stkComms_setDevice(
      stkComms_t* comms,
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
  if(rc = stkComms_sendBytes(comms, buf, 22)) {
    THROW;
  }
  rc = stkComms_recvBytes(comms, buf, 2, 25);
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

int stkComms_setDeviceExt(
      stkComms_t* comms,
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
  if(rc = stkComms_sendBytes(comms, buf, 7)) {
    THROW;
    return rc;
  }
  rc = stkComms_recvBytes(comms, buf, 2, 10);
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

int stkComms_enterProgMode(stkComms_t* comms)
{
  uint8_t buf[10];
  buf[0] = Cmnd_STK_ENTER_PROGMODE;
  buf[1] = Sync_CRC_EOP;
  int rc;
  if(rc = stkComms_sendBytes(comms, buf, 2)) {
    return rc;
  }
  rc = stkComms_recvBytes(comms, buf, 2, 10);
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

int stkComms_leaveProgMode(stkComms_t* comms)
{
  uint8_t buf[10];
  buf[0] = Cmnd_STK_LEAVE_PROGMODE;
  buf[1] = Sync_CRC_EOP;
  int rc;
  if(rc = stkComms_sendBytes(comms, buf, 2)) {
    return rc;
  }
  rc = stkComms_recvBytes(comms, buf, 2, 10);
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

int stkComms_checkSignature(stkComms_t* comms)
{
  uint8_t buf[10];
  buf[0] = Cmnd_STK_READ_SIGN;
  buf[1] = Sync_CRC_EOP;
  int rc;
  if(rc = stkComms_sendBytes(comms, buf, 2)) {
    return rc;
  }
  rc = stkComms_recvBytes(comms, buf, 5, 10);
  if(rc != 5) {
    return -1;
  }
  if(buf[0] != Resp_STK_INSYNC) {
    return -1;
  }
  if(buf[4] != Resp_STK_OK) {
    return -1;
  }
  memcpy(&comms->signature[0], &buf[1], 3);
  buf[0] = 0x1e;
  buf[1] = 0x95;
  buf[2] = 0x0f;
  if(memcmp(comms->signature, buf, 3)) {
    fprintf(stderr, "Device Signature incorrect.\n");
    return -1;
  }
  return 0;
}

int stkComms_loadAddress(stkComms_t* comms, uint16_t address)
{
  uint8_t buf[10];
  buf[0] = Cmnd_STK_LOAD_ADDRESS;
  buf[1] = (address&0xFF);
  buf[2] = address>>8;
  buf[3] = Sync_CRC_EOP;
  int rc;
  if(rc = stkComms_sendBytes(comms, buf, 4))
  {
    return -1;
  }
  rc = stkComms_recvBytes(comms, buf, 2, 4);
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

int stkComms_progHexFile(stkComms_t* comms, const char* filename)
{
  hexFile_t* file = hexFile_new();
  hexFile_init(file);
  uint16_t pageSize = 128;
  int i;
  /* Program the file one 128-byte page at a time */
  uint8_t* buf = (uint8_t*)malloc(sizeof(uint8_t)*(pageSize + 10));
  uint16_t address = 0; // The address adresses 2-byte locations
  while(address*2 < hexFile_len(file))
  {
    stkComms_loadAddress(comms, address);
    for(
        i = 0; 
        (i < pageSize) && ((address*2 + i) < hexFile_len(file)); 
        i++) 
    {
      buf[i] = hexFile_getByte(file, (address*2 + i));
    }
    stkComms_progPage(comms, buf, i);
    address += pageSize/2;
    /* Update the progress tracker */
    MUTEX_LOCK(comms->progressLock);
    comms->progress = 0.5 * ((double)address*2) / (double)hexFile_len(file);
    COND_SIGNAL(comms->progressCond);
    MUTEX_UNLOCK(comms->progressLock);
  }
  hexFile_destroy(file);
  free(file);
  free(buf);
  return 0;
}

int stkComms_checkFlash(stkComms_t* comms, const char* filename)
{
  hexFile_t* hf = hexFile_new();
  hexFile_init(hf);
  int i;
  uint16_t addrIncr = 0x40;
  uint16_t pageSize = 0x80;
  for(i = 0; i*2 < hexFile_len(hf); i += addrIncr)
  {
    if(stkComms_checkPage(comms, hf, i, i*2+pageSize > hexFile_len(hf)? hexFile_len(hf) - i*2 : pageSize))
    {
      THROW;
      return -1;
    }
    /* Update the progress tracker */
    MUTEX_LOCK(comms->progressLock);
    comms->progress = 0.5 + 0.5 * ((double)i*2) / (double)hexFile_len(hf);
    COND_SIGNAL(comms->progressCond);
    MUTEX_UNLOCK(comms->progressLock);
  }
  MUTEX_LOCK(comms->progressLock);
  comms->progress = 1;
  COND_SIGNAL(comms->progressCond);
  MUTEX_UNLOCK(comms->progressLock);
  return 0;
}

int stkComms_checkPage(stkComms_t* comms, hexFile_t* hexfile, uint16_t address, uint16_t size)
{
  /* First, load the address */
  int rc;
  if(rc = stkComms_loadAddress(comms, address)) {
    THROW;
    return rc;
  }
  uint8_t* buf = (uint8_t*)malloc(sizeof(uint8_t)*(size+10));
  /* Send the "Get Page" command */
  buf[0] = Cmnd_STK_READ_PAGE;
  buf[1] = size >> 8;
  buf[2] = size & 0xFF;
  buf[3] = 'F';
  buf[4] = Sync_CRC_EOP;
  stkComms_sendBytes(comms, buf, 5);
  /* Now, get the data */
  rc = stkComms_recvBytes(comms, buf, size+2, size+10);
  if(rc != size+2) {
    THROW;
    free(buf);
    return -1;
  }
  /* Compare with the hex file */
  int i, startIndex;
  startIndex = address*2;
  for(i = 0; i < size; i++) {
    if(hexFile_getByte(hexfile, (startIndex+i)) != buf[i+1]) {
      THROW;
      free(buf);
      return -1;
    }
  }
  free(buf);
  return 0;
}

int stkComms_progPage(stkComms_t* comms, uint8_t* data, uint16_t size)
{
  uint8_t *buf = (uint8_t*)malloc(sizeof(uint8_t)*(size + 10));
  buf[0] = Cmnd_STK_PROG_PAGE;
  buf[1] = size >> 8;
  buf[2] = size & 0xFF;
  buf[3] = 'F';
  memcpy(&buf[4], data, size);
  buf[size+4] = Sync_CRC_EOP;
  int rc;
  if(rc = stkComms_sendBytes(comms, buf, size+5)) {
    return rc;
  }
  rc = stkComms_recvBytes(comms, buf, 2, size);
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

int stkComms_progFuses(stkComms_t* comms)
{
  stkComms_universal( comms, 0xa0 , 0x03 , 0xfc , 0x00 );
  stkComms_universal( comms, 0xa0 , 0x03 , 0xfd , 0x00 );
  stkComms_universal( comms, 0xa0 , 0x03 , 0xfe , 0x00 );
  stkComms_universal( comms, 0xa0 , 0x03 , 0xff , 0x00 );
  return 0;
}

int stkComms_readData(stkComms_t* comms, uint16_t address, uint8_t *byte)
{
  int rc;
  if(rc = stkComms_loadAddress(comms, address)) {
    THROW;
    return -1;
  }
  uint8_t buf[10];
  buf[0] = Cmnd_STK_READ_DATA;
  buf[1] = Sync_CRC_EOP;
  if(rc = stkComms_sendBytes(comms, buf, 2)) {
    THROW;
    return -1;
  }
  rc = stkComms_recvBytes(comms, buf, 3, 10);
  if(rc != 3) {
    THROW;
    return -1;
  }
  *byte = buf[1];
  return 0;
}

int stkComms_writeData(stkComms_t* comms, uint16_t address, uint8_t byte)
{
  int rc;
  if(rc = stkComms_loadAddress(comms, address)) {
    THROW;
    return -1;
  }
  uint8_t buf[10];
  buf[0] = Cmnd_STK_PROG_DATA;
  buf[1] = byte;
  buf[2] = Sync_CRC_EOP;
  if(rc = stkComms_sendBytes(comms, buf, 3)) {
    THROW;
    return -1;
  }
  rc = stkComms_recvBytes(comms, buf, 2, 10);
  if(rc != 2) {
    THROW;
    return -1;
  }
  return 0;
}

int stkComms_universal(stkComms_t* comms, uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4)
{
  uint8_t buf[10];
  buf[0] = Cmnd_STK_UNIVERSAL;
  buf[1] = byte1;
  buf[2] = byte2;
  buf[3] = byte3;
  buf[4] = byte4;
  buf[5] = Sync_CRC_EOP;
  int rc;
  if(rc = stkComms_sendBytes(comms, buf, 6)) {
    return rc;
  }
  rc = stkComms_recvBytes(comms, buf, 3, 10);
  if(rc != 3) {
    THROW;
    return rc;
  }
  return 0;
}

int stkComms_sendBytes(stkComms_t* comms, void* buf, size_t len)
{
  if(!comms->isConnected) {
    return -1;
  }

#ifdef VERBOSE
  printf("SEND: ");
  for(int i = 0; i < len; i++) {
    printf("0x%02X ", ((uint8_t*)buf)[i]);
  }
  printf("\n");
#endif
#ifndef _WIN32
  if(write(comms->socket, buf, len) == -1) {
    perror("Write error");
    return -1;
  } 
#else
  if(send(comms->socket, (const char*)buf, len, 0) == -1) {
    perror("Write error");
    return -1;
  } 
#endif
  return 0;
}

int stkComms_recvBytes(stkComms_t* comms, uint8_t* buf, size_t expectedBytes, size_t size)
{
  int len = 0;
  int rc;
  uint8_t *mybuf = (uint8_t*)malloc(size*sizeof(uint8_t));
  
  while(len < expectedBytes) {
#ifndef _WIN32
    rc = read(comms->socket, mybuf, size);
#else
    rc = recvfrom(comms->socket, (char*)mybuf, 1, 0, (struct sockaddr*)0, 0);
#endif
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
  free (mybuf);
  return len;
}

int stkComms_recvBytes2(stkComms_t* comms, uint8_t* buf, size_t size)
{
  int len = 0;
  
#ifndef _WIN32
  len = read(comms->socket, buf, size);
#else

#endif
#ifdef DEBUG
  printf("Recv: ");
  for(int i = 0; i < len; i++) {
    printf("0x%2X ", ((uint8_t*)buf)[i]);
  }
  printf("\n");
#endif
  return len;
}

int stkComms_setdtr (stkComms_t* comms, int on)
{
#ifndef _WIN32
  int controlbits = TIOCM_DTR;
  if(on)
    return(ioctl(comms->socket, TIOCMBIC, &controlbits));
  else
    return(ioctl(comms->socket, TIOCMBIS, &controlbits));
#else
  return 0;
#endif
} 

#ifdef _WIN32
void baswap(bdaddr_t *dst, const bdaddr_t *src)
{
	register unsigned char *d = (unsigned char *) dst;
	register const unsigned char *s = (const unsigned char *) src;
	register int i;

	for (i = 0; i < 6; i++)
		d[i] = s[5-i];
}

int str2ba(const char *str, bdaddr_t *ba)
{
	UINT8 b[6];
	const char *ptr = str;
	int i;

	for (i = 0; i < 6; i++) {
		b[i] = (UINT8) strtol(ptr, NULL, 16);
		if (i != 5 && !(ptr = strchr(ptr, ':')))
			ptr = ":00:00:00:00:00";
		ptr++;
	}

	baswap(ba, (bdaddr_t *) b);

	return 0;
}
#endif

hexFile_t* hexFile_new()
{
  hexFile_t* hf;
  hf = (hexFile_t*)malloc(sizeof(hexFile_t));
  return hf;
}

int hexFile_init(hexFile_t* hf)
{
  hf->data = NULL;
  hf->dataAllocSize = 0;
  hf->len = 0;
  return 0;
}

int hexFile_init2(hexFile_t* hf, const char* filename)
{
  hf->data = NULL;
  hf->dataAllocSize = 0;
  hf->len = 0;
  hexFile_loadFile(hf, filename);
  return 0;
}

int hexFile_destroy(hexFile_t* hf)
{
  if(hf->data) {
    free(hf->data);
  }
  hf->dataAllocSize = 0;
  hf->len = 0;
  return 0;
}

uint8_t hexFile_getByte(hexFile_t* hf, int index)
{
  if((index < 0) || (index >= hf->len)) {
    THROW;
  }
  return hf->data[index];
}

int hexFile_loadFile(hexFile_t* hf, const char* filename)
{
  FILE* fp = fopen(filename, "r");
  if(fp == NULL) {
    return -1;
  }
  char buf[128];
  char *str;
  str = fgets(buf, 128, fp);
  while(str != NULL) {
    hexFile_parseLine(hf, str);
    str = fgets(buf, 128, fp);
  }
  fclose(fp);
  return 0;
}

int hexFile_len(hexFile_t* hf) 
{
  return hf->len;
}

void hexFile_realloc(hexFile_t* hf)
{
  int incrSize = 256;
  if(hf->dataAllocSize == 0) {
    hf->data = (uint8_t*)malloc(sizeof(uint8_t)*incrSize);
  } else {
    uint8_t* newData = (uint8_t*)malloc(sizeof(uint8_t)*(incrSize + hf->dataAllocSize));
    memcpy(newData, hf->data, hf->len);
    free(hf->data);
    hf->data = newData;
  }
  hf->dataAllocSize += incrSize;
}

void hexFile_parseLine(hexFile_t* hf, const char* line)
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
  while(hf->len + byteCount >= hf->dataAllocSize) {
    hexFile_realloc(hf);
  }
  uint8_t checktest = 0;
  checktest = byteCount + (uint8_t)(address>>8) + (uint8_t)(address&0xFF) + (uint8_t)type;
  memset(buf, 0, sizeof(char)*10);
  str = &line[9];
  for(i = 0; i < byteCount; i++) {
    strncpy(buf, str, 2);
    sscanf(buf, "%x", &value);
    hf->data[hf->len] = value;
    checktest += value;
    hf->len++;
    str += 2;
  }
  sscanf(str, "%x", &checksum);
  // 2's complement the checktest 
  checktest = (checktest ^ 0xFF) + 0x01;
  if(checktest != checksum) 
    THROW;
}

#define P2E_MAJORVERSION 3
#define P2E_MINORVERSION 70
#define P2E_VERSIONSUFFIX  

#define P2E_xstr(s) P2E_str(s)
#define P2E_str(s) #s
#define P_PRODUCTVERSION_PS P2E_xstr(P2E_MAJORVERSION.P2E_MINORVERSION P2E_VERSIONSUFFIX)

#ifndef __version_h
#define __version_h
  static const char version[] = P_PRODUCTVERSION_PS;
#endif
 
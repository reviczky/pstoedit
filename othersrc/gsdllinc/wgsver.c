/* Copyright (C) 2001-2003, Ghostgum Software Pty Ltd.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* 
 * This file is part of GSview.  The above notice applies only 
 * to this file.  It does NOT apply to any other file in GSview
 * unless that file includes the above copyright notice.
 */

/* $Id: wgsver.c,v 1.1.2.4 2003/07/14 08:03:09 ghostgum Exp $ */
/* Obtain details of copies of Ghostscript installed under Windows */

/* To compile as a demo program, define DUMP_GSVER */
/* #define DUMP_GSVER */

#include <windows.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include "wgsver.h"

/* Ghostscript may be known in the Windows Registry by
 * the following names.
 */
#define GS_PRODUCT_AFPL "AFPL Ghostscript"
#define GS_PRODUCT_ALADDIN "Aladdin Ghostscript"
#define GS_PRODUCT_GPL "GPL Ghostscript"
#define GS_PRODUCT_GNU "GNU Ghostscript"

/* Get Ghostscript versions for given product.
 * Store results starting at pver + 1 + offset.
 * Returns total number of versions in pver.
 */
static int get_gs_versions_product(int *pver, int offset, 
  const char *gs_productfamily, const char *gsregbase)
{
    HKEY hkey;
    DWORD cbData;
    HKEY hkeyroot;
    char key[256];
    int ver;
    char *p;
    int n = 0;

	if (strlen(gsregbase))
	  sprintf_s(TARGETWITHLEN(key,256) , "Software\\%s\\%s", gsregbase, gs_productfamily);
	else
	  sprintf_s(TARGETWITHLEN(key,256) ,"Software\\%s", gs_productfamily);

    hkeyroot = HKEY_LOCAL_MACHINE;
    if (RegOpenKeyExA(hkeyroot, key, 0, KEY_READ, &hkey) == ERROR_SUCCESS) {
	/* Now enumerate the keys */
	cbData = sizeof(key) / sizeof(char);
	while (RegEnumKeyA(hkey, n, key, cbData) == ERROR_SUCCESS) {
	    n++;
	    ver = 0;
	    p = key;
	    while (*p && (*p!='.')) {
		ver = (ver * 10) + (*p - '0')*100;
		p++;
	    }
	    if (*p == '.')
		p++;
	    if (*p) {
		ver += (*p - '0') * 10;
		p++;
	    }
	    if (*p)
		ver += (*p - '0');
	    if (n + offset < pver[0])
		pver[n+offset] = ver;
	}
    }
    return n+offset;
}

/* Query registry to find which versions of Ghostscript are installed.
 * Return version numbers in an integer array.   
 * On entry, the first element in the array must be the array size 
 * in elements.
 * If all is well, TRUE is returned.
 * On exit, the first element is set to the number of Ghostscript
 * versions installed, and subsequent elements to the version
 * numbers of Ghostscript.
 * e.g. on entry {5, 0, 0, 0, 0}, on exit {3, 550, 600, 596, 0}
 * Returned version numbers may not be sorted.
 *
 * If Ghostscript is not installed at all, return FALSE
 * and set pver[0] to 0.
 * If the array is not large enough, return FALSE 
 * and set pver[0] to the number of Ghostscript versions installed.
 */
BOOL get_gs_versions(int *pver, const char *gsregbase)
{
    int n;
    if (pver == (int *)NULL)
	    return FALSE;

    n = get_gs_versions_product(pver, 0, GS_PRODUCT_AFPL, gsregbase);
    n = get_gs_versions_product(pver, n, GS_PRODUCT_ALADDIN, gsregbase);
    n = get_gs_versions_product(pver, n, GS_PRODUCT_GPL, gsregbase);
    n = get_gs_versions_product(pver, n, GS_PRODUCT_GNU, gsregbase);

    if (n >= pver[0]) {
	pver[0] = n;
	return FALSE;	/* too small */
    }

    if (n == 0) {
	pver[0] = 0;
	return FALSE;	/* not installed */
    }
    pver[0] = n;
    return TRUE;
}

 
/*
 * Get a named registry value.
 * Key = hkeyroot\\key, named value = name.
 * name, ptr, plen and return values are the same as in gp_getenv();
 */

int 
gp_getenv_registry(HKEY hkeyroot, const char *key, const char *name, 
    char *ptr, int *plen)
{
    HKEY hkey;
    DWORD cbData, keytype;
    BYTE b;
    LONG rc;
    BYTE *bptr = (BYTE *)ptr;

    if (RegOpenKeyExA(hkeyroot, key, 0, KEY_READ, &hkey)
	== ERROR_SUCCESS) {
	keytype = REG_SZ;
	cbData = *plen;
	if (bptr == (BYTE *)NULL)
	    bptr = &b;	/* Registry API won't return ERROR_MORE_DATA */
			/* if ptr is NULL */
	rc = RegQueryValueExA(hkey, (char *)name, 0, &keytype, bptr, &cbData);
	(void)RegCloseKey(hkey);
	if (rc == ERROR_SUCCESS) {
	    *plen = cbData;
	    return 0;	/* found environment variable and copied it */
	} else if (rc == ERROR_MORE_DATA) {
	    /* buffer wasn't large enough */
	    *plen = cbData;
	    return -1;
	}
    }
    return 1;	/* not found */
}


static BOOL get_gs_string_product(int gs_revision, const char *name, 
    char *ptr, int len, const char *gs_productfamily, const char *gsregbase)
{
    /* If using Win32, look in the registry for a value with
     * the given name.  The registry value will be under the key
     * HKEY_CURRENT_USER\Software\AFPL Ghostscript\N.NN
     * or if that fails under the key
     * HKEY_LOCAL_MACHINE\Software\AFPL Ghostscript\N.NN
     * where "AFPL Ghostscript" is actually gs_productfamily
     * and N.NN is obtained from gs_revision.
     */

    int code;
    char key[256];
    char dotversion[16];
    int length;
    DWORD version = GetVersion();

    if (((HIWORD(version) & 0x8000) != 0)
	  && ((HIWORD(version) & 0x4000) == 0)) {
	/* Win32s */
	return FALSE;
    }


    sprintf_s(TARGETWITHLEN(dotversion,16), "%d.%02d", 
	    (int)(gs_revision / 100), (int)(gs_revision % 100));
	
	if (strlen(gsregbase))
	  sprintf_s(TARGETWITHLEN(key,256), "Software\\%s\\%s\\%s", gsregbase, gs_productfamily, dotversion);
	else
	  sprintf_s(TARGETWITHLEN(key,256), "Software\\%s\\%s", gs_productfamily, dotversion);

    length = len;
    code = gp_getenv_registry(HKEY_CURRENT_USER, key, name, ptr, &length);
    if ( code == 0 )
	return TRUE;	/* found it */

    length = len;
    code = gp_getenv_registry(HKEY_LOCAL_MACHINE, key, name, ptr, &length);

    if ( code == 0 )
	return TRUE;	/* found it */

    return FALSE;
}

BOOL get_gs_string(int gs_revision, const char *name, char *ptr, int len, 
  const char *gsregbase)
{
    if (get_gs_string_product(gs_revision, name, ptr, len, GS_PRODUCT_AFPL, gsregbase))
	return TRUE;
    if (get_gs_string_product(gs_revision, name, ptr, len, GS_PRODUCT_ALADDIN, gsregbase))
	return TRUE;
    if (get_gs_string_product(gs_revision, name, ptr, len, GS_PRODUCT_GPL, gsregbase))
	return TRUE;
    if (get_gs_string_product(gs_revision, name, ptr, len, GS_PRODUCT_GNU, gsregbase))
	return TRUE;
    return FALSE;
}



/* Set the latest Ghostscript EXE or DLL from the registry */
BOOL
find_gs(char *gspath, int len, int minver, BOOL bDLL, const char *gsregbase)
{
    int count;
    int *ver;
    int gsver;
    char buf[256];
    char *p;
    int i;

    DWORD version = GetVersion();
    if ( ((HIWORD(version) & 0x8000)!=0) && ((HIWORD(version) & 0x4000)==0) )
	return FALSE;  // win32s

    count = 1;
    (void)get_gs_versions(&count, gsregbase);
    if (count < 1)
	  return FALSE;

    ver = (int *)malloc((count+1)*sizeof(int));
    if (ver == (int *)NULL)
	return FALSE;
    ver[0] = count+1;
    if (!get_gs_versions(ver, gsregbase)) {
	free(ver);
	return FALSE;
    }
    gsver = 0;
    for (i=1; i<=ver[0]; i++) {
	if (ver[i] > gsver)
	    gsver = ver[i];
    }
    free(ver);
    if (gsver < minver)	// minimum version (e.g. for gsprint)
	return FALSE;
    
    if (!get_gs_string(gsver, "GS_DLL", buf, sizeof(buf), gsregbase))
	return FALSE;

    if (bDLL) {
		strncpy_s(gspath, len, buf, len-1);
		return TRUE;
	} else {
		p = strrchr(buf, '\\');
		if (p) {
			p++;
			*p = 0;
			strncpy_s(p,sizeof(buf)-1-strlen(buf), "gswin32c.exe", sizeof(buf)-1-strlen(buf));
			strncpy_s(gspath,len, buf, len-1);
			return TRUE;
		}
		return FALSE;
	}
}


#ifdef DUMP_GSVER
#define ENTRYPOINT main(int argc, char *argv[], char *gsregbase)
#else
#define ENTRYPOINT dumpgsvers(const char *gsregbase)
#endif

/* This is an example of how you can use the above functions */
int ENTRYPOINT
{
    BOOL flag;
    int ver[10];
    int i;
    char buf[256];

    if (find_gs(buf, sizeof(buf), 550, TRUE, gsregbase))
	fprintf(stderr,"Latest GS DLL is %s\n", buf);
    if (find_gs(buf, sizeof(buf), 550, FALSE, gsregbase))
	fprintf(stderr,"Latest GS EXE is %s\n", buf);

    ver[0] = sizeof(ver) / sizeof(int);
    flag = get_gs_versions(ver, gsregbase);
    fprintf(stderr,"Versions: %d\n", ver[0]);

    if (flag == FALSE) {
	fprintf(stderr,"get_gs_versions failed, need %d\n", ver[0]);
	return 1;
    }

    for (i=1; i <= ver[0]; i++) {
	fprintf(stderr," %d\n", ver[i]);
	if (get_gs_string(ver[i], "GS_DLL", buf, sizeof(buf), gsregbase))
	    fprintf(stderr,"   GS_DLL=%s\n", buf);
	if (get_gs_string(ver[i], "GS_LIB", buf, sizeof(buf), gsregbase))
	    fprintf(stderr,"   GS_LIB=%s\n", buf);
    }
    return 0;
}


/****************************************************************************
**
** Copyright (C) 2017 Denis Shienkov <denis.shienkov@gmail.com>
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
** Copyright (C) 2023 Gediminas Simanskis <gediminas@rusoku.com>
** Copyright (C) 2023 Rusoku technologijos UAB.
**
** This file is part of the QtSerialBus module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef RUSOKU_SYMBOLS_P_H
#define RUSOKU_SYMBOLS_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qlibrary.h>
#include <QtCore/qstring.h>
#include <QtCore/qdebug.h>
#include "CANAPI_Types.h"
#include "TouCAN_Defines.h"

#ifdef Q_OS_WIN32
    #include <windows.h>
    #define DRV_CALLBACK_TYPE WINAPI
#else
    #define DRV_CALLBACK_TYPE
#endif

/*  -----------  defines  ------------------------------------------------
 */
/*
#ifndef CAN_MAX_HANDLES
#define CAN_MAX_HANDLES         (8)     // maximum number of open handles
#endif
#ifndef DLC2LEN
#define DLC2LEN(x)              dlc_table[(x) & 0xF]
#endif
#ifndef LEN2DLC
#define LEN2DLC(x)              ((x) > 48) ? 0xF : \
                                ((x) > 32) ? 0xE : \
                                ((x) > 24) ? 0xD : \
                                ((x) > 20) ? 0xC : \
                                ((x) > 16) ? 0xB : \
                                ((x) > 12) ? 0xA : \
                                ((x) > 8) ?  0x9 : (x)
#endif
*/

typedef struct structCanalMsg {
    unsigned long flags;			    // CAN message flags
    unsigned long obid;			    	// Used by driver for channel info etc.
    unsigned long id;			      	// CAN id (11-bit or 29-bit)
    unsigned char sizeData;		  	    // Data size 0-8
    unsigned char data[8];		        // CAN Data
    unsigned long timestamp;		    // Relative time stamp for package in microseconds
} canalMsg;

typedef struct structCanalStatistics {
    unsigned long cntReceiveFrames;				// # of receive frames
    unsigned long cntTransmitFrames;			// # of transmitted frames
    unsigned long cntReceiveData;			  	// # of received data bytes
    unsigned long cntTransmitData;			    // # of transmitted data bytes
    unsigned long cntOverruns;				    // # of overruns
    unsigned long cntBusWarnings;			  	// # of bys warnings
    unsigned long cntBusOff;					// # of bus off's
} canalStatistics;

typedef struct structCanalStatus {
    unsigned long channel_status;	  	// Current state for channel
    unsigned long lasterrorcode;		// Last error code
    unsigned long lasterrorsubcode;		// Last error subcode
    char lasterrorstr[80];	    		// Last error string
} canalStatus;

typedef enum {
    FILTER_ACCEPT_ALL   = 0,
    FILTER_REJECT_ALL,
    FILTER_VALUE,
}Filter_Type_TypeDef;

#define CANAL_DEVLIST_SIZE_MAX 64

typedef struct struct_CANAL_DEV_INFO {
    unsigned int    DeviceId;
    unsigned int    vid;
    unsigned int    pid;
    char            SerialNumber[10];
} canal_dev_info, *pcanal_dev_info;


typedef struct struct_CANAL_DEV_LIST{
    canal_dev_info canDevInfo[CANAL_DEVLIST_SIZE_MAX];
    unsigned int   canDevCount;
} canal_dev_list, *pcanal_dev_list;

#define GENERATE_SYMBOL_VARIABLE(returnType, symbolName, ...) \
    typedef returnType (DRV_CALLBACK_TYPE *fp_##symbolName)(__VA_ARGS__); \
    static fp_##symbolName symbolName;

#define RESOLVE_SYMBOL(symbolName) \
    symbolName = reinterpret_cast<fp_##symbolName>(rusokuLibrary->resolve(#symbolName)); \
    if (!symbolName) \
        return false;

GENERATE_SYMBOL_VARIABLE(long, CanalOpen, const char*, unsigned long)
GENERATE_SYMBOL_VARIABLE(long, CanalClose, long)
GENERATE_SYMBOL_VARIABLE(long, CanalGetLevel, long)
GENERATE_SYMBOL_VARIABLE(int, CanalSend, canalMsg*)
GENERATE_SYMBOL_VARIABLE(int, CanalBlockingSend, long, canalMsg*, unsigned long)
GENERATE_SYMBOL_VARIABLE(int, CanalReceive, canalMsg*)
GENERATE_SYMBOL_VARIABLE(int, CanalBlockingReceive, long, canalMsg*, unsigned long)
GENERATE_SYMBOL_VARIABLE(int, CanalDataAvailable, long)
GENERATE_SYMBOL_VARIABLE(int, CanalGetStatus, long, canalStatus*)
GENERATE_SYMBOL_VARIABLE(int, CanalGetStatistics, long, canalStatistics*)
GENERATE_SYMBOL_VARIABLE(int, CanalSetFilter, long , unsigned long)
GENERATE_SYMBOL_VARIABLE(int, CanalSetMask, long , unsigned long)
GENERATE_SYMBOL_VARIABLE(int, CanalSetBaudrate, long, unsigned long)
GENERATE_SYMBOL_VARIABLE(unsigned long, CanalGetVersion, void)
GENERATE_SYMBOL_VARIABLE(unsigned long, CanalGetDllVersion, void)
GENERATE_SYMBOL_VARIABLE(const char *, CanalGetVendorString, void)
GENERATE_SYMBOL_VARIABLE(const char *, CanalGetDriverInfo, void)
GENERATE_SYMBOL_VARIABLE(int, CanalSetFilter11bit, long, Filter_Type_TypeDef,  unsigned long, unsigned long)
GENERATE_SYMBOL_VARIABLE(int, CanalSetFilter29bit, long, Filter_Type_TypeDef,  unsigned long, unsigned long)
GENERATE_SYMBOL_VARIABLE(int, CanalGetBootloaderVersion, long, unsigned long*)
GENERATE_SYMBOL_VARIABLE(int, CanalGetHardwareVersion, long, unsigned long*)
GENERATE_SYMBOL_VARIABLE(int, CanalGetFirmwareVersion, long, unsigned long*)
GENERATE_SYMBOL_VARIABLE(int, CanalGetSerialNumber,long, unsigned long*)
GENERATE_SYMBOL_VARIABLE(int, CanalGetVidPid, long, unsigned long*)
GENERATE_SYMBOL_VARIABLE(int, CanalGetDeviceId, long, unsigned long*)
GENERATE_SYMBOL_VARIABLE(int, CanalGetVendor, long, unsigned int, char*)
GENERATE_SYMBOL_VARIABLE(int, CanalInterfaceStart, long)
GENERATE_SYMBOL_VARIABLE(int, CanalInterfaceStop, long)
GENERATE_SYMBOL_VARIABLE(int, CanalGetDeviceList, canal_dev_list*, int)

inline bool resolveRusokuCanSymbols(QLibrary *rusokuLibrary)
{
    if (!rusokuLibrary->isLoaded()) {
        #ifdef Q_OS_MACOS
            rusokuLibrary->setFileName(QStringLiteral("libUVCANTOU"));
        #else
            rusokuLibrary->setFileName(QStringLiteral("canal.dll"));
        #endif
        if (!rusokuLibrary->load())
            return false;
    }

    RESOLVE_SYMBOL(CanalOpen)
    RESOLVE_SYMBOL(CanalClose)
    RESOLVE_SYMBOL(CanalGetLevel)
    RESOLVE_SYMBOL(CanalSend)
    RESOLVE_SYMBOL(CanalBlockingSend)
    RESOLVE_SYMBOL(CanalReceive)
    RESOLVE_SYMBOL(CanalBlockingReceive)
    RESOLVE_SYMBOL(CanalDataAvailable)
    RESOLVE_SYMBOL(CanalGetStatus)
    RESOLVE_SYMBOL(CanalGetStatistics)
    RESOLVE_SYMBOL(CanalSetFilter)
    RESOLVE_SYMBOL(CanalSetMask)
    RESOLVE_SYMBOL(CanalSetBaudrate)
    RESOLVE_SYMBOL(CanalGetVersion)
    RESOLVE_SYMBOL(CanalGetDllVersion)
    RESOLVE_SYMBOL(CanalGetVendorString)
    RESOLVE_SYMBOL(CanalGetDriverInfo)
    RESOLVE_SYMBOL(CanalSetFilter11bit)
    RESOLVE_SYMBOL(CanalSetFilter29bit)
    RESOLVE_SYMBOL(CanalGetBootloaderVersion)
    RESOLVE_SYMBOL(CanalGetHardwareVersion)
    RESOLVE_SYMBOL(CanalGetFirmwareVersion)
    RESOLVE_SYMBOL(CanalGetSerialNumber)
    RESOLVE_SYMBOL(CanalGetVidPid)
    RESOLVE_SYMBOL(CanalGetDeviceId)
    RESOLVE_SYMBOL(CanalGetVendor)
    RESOLVE_SYMBOL(CanalInterfaceStart)
    RESOLVE_SYMBOL(CanalInterfaceStop)
    RESOLVE_SYMBOL(CanalGetDeviceList)

    return true;
}

#endif // RUSOKU_SYMBOLS_P_H

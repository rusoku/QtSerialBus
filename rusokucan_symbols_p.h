/****************************************************************************
**
** Copyright (C) 2017 Denis Shienkov <denis.shienkov@gmail.com>
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
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
#   include <windows.h>
#   define DRV_CALLBACK_TYPE WINAPI
#else
#   define DRV_CALLBACK_TYPE
#endif


#define GENERATE_SYMBOL_VARIABLE(returnType, symbolName, ...) \
    typedef returnType (DRV_CALLBACK_TYPE *fp_##symbolName)(__VA_ARGS__); \
    static fp_##symbolName symbolName;

#define RESOLVE_SYMBOL(symbolName) \
    symbolName = reinterpret_cast<fp_##symbolName>(rusokuLibrary->resolve(#symbolName)); \
    if (!symbolName) \
        return false;

GENERATE_SYMBOL_VARIABLE(int, can_test, int32_t, uint8_t, const void *, int *)
GENERATE_SYMBOL_VARIABLE(int, can_init, int32_t, uint8_t, const void *)
GENERATE_SYMBOL_VARIABLE(int, can_exit, int)
GENERATE_SYMBOL_VARIABLE(int, can_kill, int)
GENERATE_SYMBOL_VARIABLE(int, can_start,int, const can_bitrate_t *)
GENERATE_SYMBOL_VARIABLE(int, can_reset, int)
GENERATE_SYMBOL_VARIABLE(int, can_write, int, const can_message_t, uint16_t)
GENERATE_SYMBOL_VARIABLE(int, can_read, int, const can_message_t, uint16_t)
GENERATE_SYMBOL_VARIABLE(int, can_status, int, uint8_t *)
GENERATE_SYMBOL_VARIABLE(int, can_busload, int, uint8_t *, uint8_t *)
GENERATE_SYMBOL_VARIABLE(int, can_bitrate, int, can_bitrate_t *, can_speed_t *)
GENERATE_SYMBOL_VARIABLE(int, can_property, int, uint16_t, void *, uint32_t)
GENERATE_SYMBOL_VARIABLE(char *, can_hardware, int)
GENERATE_SYMBOL_VARIABLE(char *, can_firmware, int)
GENERATE_SYMBOL_VARIABLE(char *, can_version, int)

inline bool resolveRusokuCanSymbols(QLibrary *rusokuLibrary)
{
    if (!rusokuLibrary->isLoaded()) {
        #ifdef Q_OS_MACOS
            rusokuLibrary->setFileName(QStringLiteral("libUVCANTOU"));
        #else
            pcanLibrary->setFileName(QStringLiteral("uvcantou"));
        #endif
        if (!rusokuLibrary->load())
            return false;
    }

    RESOLVE_SYMBOL(can_test)
    RESOLVE_SYMBOL(can_init)
    RESOLVE_SYMBOL(can_exit)
    RESOLVE_SYMBOL(can_kill)
    RESOLVE_SYMBOL(can_start)
    RESOLVE_SYMBOL(can_reset)
    RESOLVE_SYMBOL(can_write)
    RESOLVE_SYMBOL(can_read)
    RESOLVE_SYMBOL(can_status)
    RESOLVE_SYMBOL(can_busload)
    RESOLVE_SYMBOL(can_bitrate)
    RESOLVE_SYMBOL(can_property)
    RESOLVE_SYMBOL(can_hardware)
    RESOLVE_SYMBOL(can_firmware)
    RESOLVE_SYMBOL(can_version)

    return true;
}

#endif // RUSOKU_SYMBOLS_P_H

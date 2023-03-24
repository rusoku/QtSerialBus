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

#ifndef RUSOKUCANBACKEND_P_H
#define RUSOKUCANBACKEND_P_H

#include <QtSerialBus/qcanbusframe.h>
#include <QtSerialBus/qcanbusdevice.h>
#include <QtSerialBus/qcanbusdeviceinfo.h>

#include <QtCore/qvariant.h>
#include <QtCore/qvector.h>
#include <QtCore/qlist.h>

#if defined(Q_OS_WIN32)
    #include <qt_windows.h>
#else
    //other stuff
#endif

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

QT_BEGIN_NAMESPACE

class QSocketNotifier;
class QWinEventNotifier;
class QTimer;

class RusokuCanBackendPrivate {
    Q_DECLARE_PUBLIC(RusokuCanBackend)

public:
    RusokuCanBackendPrivate(RusokuCanBackend *q);

    bool open();
    void close();
    bool setConfigurationParameter(int key, const QVariant &value);
    void setupChannel(const QByteArray &interfaceName);
    void setupDefaultConfigurations();
    QString systemErrorString(/*TPCANStatus errorCode*/);
    void startWrite();
    void startRead();
    bool verifyBitRate(int bitrate);

    RusokuCanBackend *const q_ptr;

    bool isFlexibleDatarateEnabled = false;
    bool isOpen = false;
    int32_t channelIndex = -1;
    int handle = -1;
    QTimer *writeNotifier = nullptr;

#if defined(Q_OS_WIN32)
    //QWinEventNotifier *readNotifier = nullptr;
    //HANDLE readHandle  = INVALID_HANDLE_VALUE;
#else
    //QSocketNotifier *readNotifier = nullptr;
    QTimer *readNotifier = nullptr;
    int readHandle = -1;
#endif

};

QT_END_NAMESPACE

#endif // RUSOKUCANBACKEND_P_H

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

#include "rusokucanbackend.h"
#include "rusokucanbackend_p.h"
#include "rusokucan_symbols_p.h"

#include <QtSerialBus/qcanbusdevice.h>

#include <QtCore/qtimer.h>
#include <QtCore/qcoreevent.h>
#include <QtCore/qloggingcategory.h>

#include <algorithm>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_CANBUS_PLUGINS_RUSOKUCAN)

RusokuCanBackendPrivate::RusokuCanBackendPrivate(RusokuCanBackend *q)
        : q_ptr(q)
{
}

bool RusokuCanBackendPrivate::open() {
    Q_Q(RusokuCanBackend);

    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "RusokuCanBackendPrivate::open()");

    const int nominalBitrate = q->configurationParameter(QCanBusDevice::BitRateKey).toInt();
    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "--- nominal bitrate = %d", nominalBitrate);

    //can_init(channelIndex, nominalBitrate);
    //if ((handle = can_init(channelIndex, CANMODE_DEFAULT, 0)) < CANERR_NOERROR){
    //    qCCritical(QT_CANBUS_PLUGINS_RUSOKUCAN, "Cannot initialize hardware");
    //    q->setError("Cannot initialize hardware", QCanBusDevice::ConnectionError);
    //    return false;
   //}

    //can_init(0, 0, NULL);
    //can_exit(0);

    isOpen = true;
    return true;
}

void RusokuCanBackendPrivate::close()
{
    Q_Q(RusokuCanBackend);



    isOpen = false;
}

bool RusokuCanBackendPrivate::setConfigurationParameter(int key, const QVariant &value)
{
    Q_Q(RusokuCanBackend);



    return true;
}

struct TCanDevice {
    int32_t adapter;
    char *name;
};

static const TCanDevice m_CanDevices[] = {
        {TOUCAN_USB_CHANNEL0, (char *)"TouCAN-USB1" },
        {TOUCAN_USB_CHANNEL1, (char *)"TouCAN-USB2" },
        {TOUCAN_USB_CHANNEL2, (char *)"TouCAN-USB3" },
        {TOUCAN_USB_CHANNEL3, (char *)"TouCAN-USB4" },
        {TOUCAN_USB_CHANNEL4, (char *)"TouCAN-USB5" },
        {TOUCAN_USB_CHANNEL5, (char *)"TouCAN-USB6" },
        {TOUCAN_USB_CHANNEL6, (char *)"TouCAN-USB7" },
        {TOUCAN_USB_CHANNEL7, (char *)"TouCAN-USB8" },
        {EOF, NULL}
};

void RusokuCanBackendPrivate::setupChannel(const QByteArray &interfaceName)
{
    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "RusokuCanBackendPrivate::setupChannel()");

    const TCanDevice *channel = m_CanDevices;
    while (channel->adapter != EOF && channel->name != interfaceName)
        ++channel;
    channelIndex = channel->adapter;

    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "--- channel index %d", channelIndex);
}

// Calls only when the device is closed
void RusokuCanBackendPrivate::setupDefaultConfigurations()
{
    Q_Q(RusokuCanBackend);

    q->setConfigurationParameter(QCanBusDevice::BitRateKey, 500000);
}

QString RusokuCanBackendPrivate::systemErrorString(/*TPCANStatus errorCode*/)
{
    Q_Q(RusokuCanBackend);

    return RusokuCanBackend::tr("TODO:");
}

void RusokuCanBackendPrivate::startWrite()
{
    Q_Q(RusokuCanBackend);
}

void RusokuCanBackendPrivate::startRead()
{
    Q_Q(RusokuCanBackend);
}

bool RusokuCanBackendPrivate::verifyBitRate(int bitrate)
{
    Q_Q(RusokuCanBackend);

    return true;
}

QT_END_NAMESPACE


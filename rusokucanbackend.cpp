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

#include "rusokucanbackend.h"
#include "rusokucanbackend_p.h"
//#include "rusokucan_symbols_p.h"

#include <QtSerialBus/qcanbusdevice.h>

#include <QtCore/qtimer.h>
#include <QtCore/qcoreevent.h>
#include <QtCore/qloggingcategory.h>

#include <algorithm>


#ifdef Q_OS_WIN32
//   include <QtCore/qwineventnotifier.h>
#else
//   include <QtCore/qsocketnotifier.h>
#endif


QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_CANBUS_PLUGINS_RUSOKUCAN)

bool RusokuCanBackend::canCreate(QString *errorReason)
{
    //qCCritical(QT_CANBUS_PLUGINS_RUSOKUCAN, "Cannot load library: ");
    //QMessageLogger();
    qDebug() << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";

    return true;
}

QList<QCanBusDeviceInfo> RusokuCanBackend::interfaces()
{
    QList<QCanBusDeviceInfo> result;
/*
    static QCanBusDeviceInfo createDeviceInfo(const QString &name,
                                              bool isVirtual = false,
                                              bool isFlexibleDataRateCapable = false);

    static QCanBusDeviceInfo createDeviceInfo(const QString &name, const QString &serialNumber,
                                              const QString &description, int channel,
                                              bool isVirtual, bool isFlexibleDataRateCapable);
*/

    result.append(std::move(createDeviceInfo(QLatin1String("usb0"),
                                             "00000001", QLatin1String("TouCAN adapter"),
                                             1, false, false)));

    result.append(std::move(createDeviceInfo(QLatin1String("usb1"),
                                             "00000002", QLatin1String("TouCAN adapter"),
                                             1, false, false)));

    return result;
}

RusokuCanBackend::RusokuCanBackend(const QString &name, QObject *parent)
        : QCanBusDevice(parent)
        , d_ptr(new RusokuCanBackendPrivate(this))
{
    Q_D(RusokuCanBackend);

//#define Q_D(Class) Class##Private * const d = d_func()
//RusokuCanBackendPrivate *const d = d_func()

    d->setupChannel(name.toLatin1());
    d->setupDefaultConfigurations();

    std::function<void()> f = std::bind(&RusokuCanBackend::resetController, this);
    setResetControllerFunction(f);

    std::function<CanBusStatus()> g = std::bind(&RusokuCanBackend::busStatus, this);
    setCanBusStatusGetter(g);
}

RusokuCanBackend::~RusokuCanBackend()
{
    Q_D(RusokuCanBackend);

    if (d->isOpen)
        close();

    delete d_ptr;
}

bool RusokuCanBackend::open()
{
    Q_D(RusokuCanBackend);

    if (!d->isOpen) {
        if (Q_UNLIKELY(!d->open()))
            return false;

        // Apply all stored configurations except bitrate, because
        // the bitrate cannot be changed after opening the device
        const auto keys = configurationKeys();
        for (int key : keys) {
            if (key == QCanBusDevice::BitRateKey || key == QCanBusDevice::DataBitRateKey)
                continue;
            const QVariant param = configurationParameter(key);
            const bool success = d->setConfigurationParameter(key, param);
            if (Q_UNLIKELY(!success)) {
                qCWarning(QT_CANBUS_PLUGINS_RUSOKUCAN, "Cannot apply parameter: %d with value: %ls.",
                          key, qUtf16Printable(param.toString()));
            }
        }
    }

    setState(QCanBusDevice::ConnectedState);
    return true;
}

void RusokuCanBackend::close()
{
    Q_D(RusokuCanBackend);

    d->close();

    setState(QCanBusDevice::UnconnectedState);
}

void RusokuCanBackend::setConfigurationParameter(int key, const QVariant &value)
{
    Q_D(RusokuCanBackend);

    if (d->setConfigurationParameter(key, value))
        QCanBusDevice::setConfigurationParameter(key, value);
}

bool RusokuCanBackend::writeFrame(const QCanBusFrame &newData)
{
    Q_D(RusokuCanBackend);

    if (Q_UNLIKELY(state() != QCanBusDevice::ConnectedState))
        return false;

    if (Q_UNLIKELY(!newData.isValid())) {
        setError(tr("Cannot write invalid QCanBusFrame"), QCanBusDevice::WriteError);
        return false;
    }

    if (Q_UNLIKELY(newData.frameType() != QCanBusFrame::DataFrame
                   && newData.frameType() != QCanBusFrame::RemoteRequestFrame)) {
        setError(tr("Unable to write a frame with unacceptable type"),
                 QCanBusDevice::WriteError);
        return false;
    }

    enqueueOutgoingFrame(newData);

    if (!d->writeNotifier->isActive())
        d->writeNotifier->start();

    return true;
}

// TODO: Implement me
QString RusokuCanBackend::interpretErrorFrame(const QCanBusFrame &errorFrame)
{
    Q_UNUSED(errorFrame);

    return QString();
}

void RusokuCanBackend::resetController()
{
    close();
    open();
}

QCanBusDevice::CanBusStatus RusokuCanBackend::busStatus() const
{
/*
    const TPCANStatus status = ::CAN_GetStatus(d_ptr->channelIndex);

    switch (status & PCAN_ERROR_ANYBUSERR) {
        case PCAN_ERROR_OK:
            return QCanBusDevice::CanBusStatus::Good;
        case PCAN_ERROR_BUSWARNING:
            return QCanBusDevice::CanBusStatus::Warning;
        case PCAN_ERROR_BUSPASSIVE:
            return QCanBusDevice::CanBusStatus::Error;
        case PCAN_ERROR_BUSOFF:
            return QCanBusDevice::CanBusStatus::BusOff;
        default:
            qCWarning(QT_CANBUS_PLUGINS_PEAKCAN, "Unknown CAN bus status: %lu.", ulong(status));
            return QCanBusDevice::CanBusStatus::Unknown;
    }
*/
    return QCanBusDevice::CanBusStatus::Good;
}

QT_END_NAMESPACE

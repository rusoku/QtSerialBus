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

// NSVersionOfLinkTimeLibrary()

#include "rusokucanbackend.h"
#include "rusokucanbackend_p.h"
#include "rusokucan_symbols_p.h"

#include <QtSerialBus/qcanbusdevice.h>

#include <QtCore/qtimer.h>
#include <QtCore/qcoreevent.h>
#include <QtCore/qloggingcategory.h>

#include <algorithm>


//#ifdef Q_OS_WIN32
//   include <QtCore/qwineventnotifier.h>
//#else
//   include <QtCore/qsocketnotifier.h>
//#endif

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_CANBUS_PLUGINS_RUSOKUCAN)

#ifndef LINK_LIBRTSTATIC
Q_GLOBAL_STATIC(QLibrary, rusokucanLibrary)
#endif

bool RusokuCanBackend::canCreate(QString *errorReason)
{
#ifdef LINK_LIBRTSTATIC
    return true;
#else

    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "RusokuCanBackend::canCreate() - load dll library");

    static bool symbolsResolved = resolveRusokuCanSymbols(rusokucanLibrary());
    if (Q_UNLIKELY(!symbolsResolved)) {
        qCCritical(QT_CANBUS_PLUGINS_RUSOKUCAN, "Cannot load library: %ls",
                   qUtf16Printable(rusokucanLibrary()->errorString()));
        *errorReason = "Cannot load CANAL dynamic library";

        return false;
    }
    return true;
#endif
}

QList<QCanBusDeviceInfo> RusokuCanBackend::interfaces()
{
    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "RusokuCanBackend::interfaces()");

    QList<QCanBusDeviceInfo> result;
    canal_dev_list  can_device_list = {};
    quint16 canal_total_devices;

    CanalGetDeviceList(&can_device_list, 8);
    canal_total_devices = can_device_list.canDevCount;

    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "Found total devices - %lu", canal_total_devices);

    for(quint16 x = 0; x < canal_total_devices; x++){

        result.append(std::move(createDeviceInfo(QLatin1String(can_device_list.canDevInfo[x].SerialNumber),
                                                 QLatin1String("TouCAN adapter"),
                                                 QLatin1String("RUSOKU"), x,
                                                 false, false)));
    }

    return result;
}

RusokuCanBackend::RusokuCanBackend(const QString &name, QObject *parent)
        : QCanBusDevice(parent)
        , d_ptr(new RusokuCanBackendPrivate(this))
{
    Q_D(RusokuCanBackend);

    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "RusokuCanBackend::RusokuCanBackend() - %ls",
                                                                qUtf16Printable(name));

    //d->setupChannel(name.toLatin1());
    d->setupChannel(name);
    d->setupDefaultConfigurations();

    std::function<void()> f = std::bind(&RusokuCanBackend::resetController, this);
    setResetControllerFunction(f);

    std::function<CanBusStatus()> g = std::bind(&RusokuCanBackend::busStatus, this);
    setCanBusStatusGetter(g);
}

RusokuCanBackend::~RusokuCanBackend()
{
    Q_D(RusokuCanBackend);

    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "RusokuCanBackend::~RusokuCanBackend()");

    if (d->isOpen)
        close();

    delete d_ptr;
}

bool RusokuCanBackend::open()
{
    Q_D(RusokuCanBackend);

    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "RusokuCanBackend::open()");

    if (!d->isOpen) {
        if (Q_UNLIKELY(!d->open()))
            return false;

        // Apply all stored configurations except bitrate, because
        // the bitrate cannot be changed after opening the device
        const auto keys = configurationKeys();

        qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "--- total keys: %d", keys.count());

        for (int key : keys) {
            if (key == QCanBusDevice::BitRateKey || key == QCanBusDevice::DataBitRateKey)
                continue;
            const QVariant param = configurationParameter(key);

            qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "--- key = %d, value = %ls", key, qUtf16Printable(param.toString()));

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

    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "RusokuCanBackend::close()");

    d->close();

    setState(QCanBusDevice::UnconnectedState);
}

void RusokuCanBackend::setConfigurationParameter(int key, const QVariant &value)
{
    Q_D(RusokuCanBackend);

    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "RusokuCanBackend::setConfigurationParameter()");
    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "--- key = %d, value = %ls", key, qUtf16Printable(value.toString()));

    if (d->setConfigurationParameter(key, value))
        QCanBusDevice::setConfigurationParameter(key, value);
}

bool RusokuCanBackend::writeFrame(const QCanBusFrame &newData)
{
    Q_D(RusokuCanBackend);

    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "RusokuCanBackend::writeFrame()");

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

    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "RusokuCanBackend::interpretErrorFrame()");

    return QString();
}

void RusokuCanBackend::resetController()
{
    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "RusokuCanBackend::resetController()");

    close();
    open();
}

QCanBusDevice::CanBusStatus RusokuCanBackend::busStatus() const
{
    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "RusokuCanBackend::busStatus()");

    quint8 rc = CANAL_ERROR_GENERIC;
    canalStatus canStatus = {};

    //rc = ::can_status(d_ptr->handle, &status);
    rc = CanalGetStatus(d_ptr->handle, &canStatus);

    if(rc != CANAL_ERROR_SUCCESS) {
        qCWarning(QT_CANBUS_PLUGINS_RUSOKUCAN, "Unknown CAN bus status: %lu.", ulong(canStatus.channel_status));
        return QCanBusDevice::CanBusStatus::Unknown;
    }

    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "handle: %d", d_ptr->handle);
    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "CAN_STATUS: %lu", canStatus.channel_status);

    switch (canStatus.channel_status & 0xF0) {
        case CANAL_STATUSMSG_OK:
            return QCanBusDevice::CanBusStatus::Good;
        case CANAL_STATUSMSG_OVERRUN:
            return QCanBusDevice::CanBusStatus::Warning;
        case CANAL_STATUSMSG_BUSLIGHT:
            return QCanBusDevice::CanBusStatus::Warning;
        case CANAL_STATUSMSG_BUSHEAVY:
            return QCanBusDevice::CanBusStatus::Warning;
        case CANAL_STATUSMSG_BUSOFF:
            return QCanBusDevice::CanBusStatus::BusOff;
        default:
            qCWarning(QT_CANBUS_PLUGINS_RUSOKUCAN, "Unknown CAN bus status: %lu.", ulong(canStatus.channel_status));
            return QCanBusDevice::CanBusStatus::Unknown;
    }
}

////////////////////////////////////////////////////////////////////////////////////////

class RusokuCanWriteNotifier : public QTimer
{
    // no Q_OBJECT macro!
public:
    RusokuCanWriteNotifier(RusokuCanBackendPrivate *d, QObject *parent)
            : QTimer(parent)
            , dptr(d)
    {
    }

protected:
    void timerEvent(QTimerEvent *e) override
    {
        if (e->timerId() == timerId()) {
            dptr->startWrite();
            return;
        }
        QTimer::timerEvent(e);
    }

private:
    RusokuCanBackendPrivate * const dptr;
};


class RusokuCanReadNotifier : public QTimer
{
    // no Q_OBJECT macro!
public:
    RusokuCanReadNotifier(RusokuCanBackendPrivate *d, QObject *parent)
            : QTimer(parent)
            , dptr(d)
    {
    }

protected:
    void timerEvent(QTimerEvent *e) override
    {
        if (e->timerId() == timerId()) {
            dptr->startRead();
            return;
        }
        QTimer::timerEvent(e);
    }

private:
    RusokuCanBackendPrivate * const dptr;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//       RusokuCanBackendPrivate
//

RusokuCanBackendPrivate::RusokuCanBackendPrivate(RusokuCanBackend *q)
        : q_ptr(q)
{
}

bool RusokuCanBackendPrivate::open() {
    Q_Q(RusokuCanBackend);

    QVariant param;
    QString DeviceInitString;

    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "RusokuCanBackendPrivate::open()");

    const int nominalBitrate = q->configurationParameter(QCanBusDevice::BitRateKey).toInt();
    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "--- nominal bitrate = %d", nominalBitrate);

    const int sbusconfig = q->configurationParameter(QCanBusDevice::UserKey).toInt();
    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "--- sbusconfig = %#010x", sbusconfig);

    param = q->configurationParameter(QCanBusDevice::BitRateKey);

    DeviceInitString.append("0;");
    DeviceInitString.append(m_DeviceName);
    DeviceInitString.append(";");
    DeviceInitString.append(qUtf16Printable(param.toString()));
    DeviceInitString.chop(3);
    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN,"TouCAN init string: %ls", DeviceInitString.utf16());

    handle = CanalOpen(DeviceInitString.toLatin1(),sbusconfig & 6);
    if(handle <= 0)
        return false;

    writeNotifier = new RusokuCanWriteNotifier(this, q);
    writeNotifier->setInterval(0);

    readNotifier = new RusokuCanReadNotifier(this, q);
    readNotifier->setInterval(0);
    readNotifier->start(0);

    isOpen = true;
    return true;
}

void RusokuCanBackendPrivate::close()
{
    Q_Q(RusokuCanBackend);

    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "RusokuCanBackendPrivate::close()");
    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "--- handle = %d", handle);

    readNotifier->stop();
    writeNotifier->stop();

    delete writeNotifier;
    writeNotifier = nullptr;

    delete readNotifier;
    readNotifier = nullptr;

    CanalClose(handle);

    isOpen = false;
}

bool RusokuCanBackendPrivate::setConfigurationParameter(int key, const QVariant &value)
{
    Q_Q(RusokuCanBackend);

    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "RusokuCanBackendPrivate::setConfigurationParameter()");

    return true;
}

//void RusokuCanBackendPrivate::setupChannel(const QByteArray &interfaceName)
void RusokuCanBackendPrivate::setupChannel(const QString &interfaceName)
{
    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "RusokuCanBackendPrivate::setupChannel()");
    m_DeviceName = interfaceName;

    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "--- channel index %d", channelIndex);
}

// Calls only when the device is closed
void RusokuCanBackendPrivate::setupDefaultConfigurations()
{
    Q_Q(RusokuCanBackend);

    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "RusokuCanBackendPrivate::setupDefaultConfigurations()");

    q->setConfigurationParameter(QCanBusDevice::BitRateKey, 500000);
}

QString RusokuCanBackendPrivate::systemErrorString(int errorCode)
{
    Q_Q(RusokuCanBackend);

    return RusokuCanBackend::tr("TODO:");
}

void RusokuCanBackendPrivate::startWrite()
{
    Q_Q(RusokuCanBackend);
    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "RusokuCanBackendPrivate::startWrite()");

    quint16 st = CANAL_ERROR_GENERIC;

    if (!q->hasOutgoingFrames()) {
        writeNotifier->stop();
        qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, " writeNotifier->stop()");
        return;
    }

    const QCanBusFrame frame = q->dequeueOutgoingFrame();
    const QByteArray payload = frame.payload();
    canalMsg CanalMsg = {};

    CanalMsg.id = frame.frameId();
    CanalMsg.sizeData = static_cast<quint8>(payload.size());

    if(frame.hasExtendedFrameFormat()){
        CanalMsg.flags |=  CANAL_IDFLAG_EXTENDED;
    }

    if (frame.frameType() == QCanBusFrame::RemoteRequestFrame){
        CanalMsg.flags |=  CANAL_IDFLAG_RTR;
    }else{
        ::memcpy(CanalMsg.data, payload.constData(), sizeof(CanalMsg.data)/sizeof(CanalMsg.data[0]));
    }

    CanalMsg.timestamp = 0;

    st = CanalSend(handle, &CanalMsg);

    if (Q_UNLIKELY(st != CANAL_ERROR_SUCCESS)) {
        qCWarning(QT_CANBUS_PLUGINS_RUSOKUCAN, "Cannot write frame errcode = %d", st);
    }else{
        emit q->framesWritten(qint64(1));
    }

    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "--- can_write");
    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "--- handle = %d", handle);

    if (q->hasOutgoingFrames() && !writeNotifier->isActive())
        writeNotifier->start();
}

void RusokuCanBackendPrivate::startRead()
{
    Q_Q(RusokuCanBackend);
    //qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "RusokuCanBackendPrivate::startRead()");

    QVector<QCanBusFrame>   newFrames;
    canalMsg    CanalMsg = {};
    quint16     st = CANAL_ERROR_GENERIC;
    quint16     DataAvailableCount = 0;

    DataAvailableCount = CanalDataAvailable(handle);

    if(DataAvailableCount <= 0)
        return;

    for(quint16 x = 0; x < DataAvailableCount; x++ ) {

        st = CanalReceive(handle, &CanalMsg);

        if (st != CANAL_ERROR_SUCCESS) {
            //q->setError(systemErrorString(st), QCanBusDevice::ReadError);
            //qCWarning(QT_CANBUS_PLUGINS_RUSOKUCAN, "Cannot read frame, err_code = %d", st);
            return;
        }

        const quint8 size = CanalMsg.sizeData;

        QCanBusFrame frame(CanalMsg.id,
            QByteArray(reinterpret_cast<const char *>(CanalMsg.data), size));

        if (CanalMsg.flags & CANAL_IDFLAG_EXTENDED) {
            frame.setExtendedFrameFormat(true);
        }

        if (CanalMsg.flags & CANAL_IDFLAG_RTR) {
            frame.setFrameType(QCanBusFrame::RemoteRequestFrame);
        }

        frame.setTimeStamp(QCanBusFrame::TimeStamp::fromMicroSeconds(static_cast<qint64>(CanalMsg.timestamp)));

        newFrames.append(std::move(frame));
        q->enqueueReceivedFrames(newFrames);

        //qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "RX frame list received()");
    }
}

bool RusokuCanBackendPrivate::verifyBitRate(int bitrate)
{
    Q_Q(RusokuCanBackend);

    return true;
}

QT_END_NAMESPACE

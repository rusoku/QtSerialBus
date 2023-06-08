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

    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "RusokuCanBackend::canCreate()");

    static bool symbolsResolved = resolveRusokuCanSymbols(rusokucanLibrary());
    if (Q_UNLIKELY(!symbolsResolved)) {
        qCCritical(QT_CANBUS_PLUGINS_RUSOKUCAN, "Cannot load library: %ls",
                   qUtf16Printable(rusokucanLibrary()->errorString()));
        *errorReason = "Cannot load libUVCANTOU dynamic library";

        return false;
    }
    return true;
#endif
}

/// \brief  CAN channel information
struct SChannelInfo {
    int32_t m_nChannelNo;  ///< channel no. at actual index in the interface list
    char m_szDeviceName[CANPROP_MAX_BUFFER_SIZE];  ///< device name at actual index in the interface list
    char m_szDeviceDllName[CANPROP_MAX_BUFFER_SIZE];  ///< file name of the DLL at actual index in the interface list
    int32_t m_nLibraryId;  ///< library id at actual index in the interface list
    char m_szVendorName[CANPROP_MAX_BUFFER_SIZE];  ///< vendor name at actual index in the interface list
    uint32_t m_nSerialNumber;
};

QList<QCanBusDeviceInfo> RusokuCanBackend::interfaces()
{
    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "RusokuCanBackend::interfaces()");

    QList<QCanBusDeviceInfo> result;




    SChannelInfo info = {};

    //memset(&info, 0, sizeof(SChannelInfo));
    info.m_nChannelNo = (-1);
    int state, rc;
    can_mode_t opMode = {};
    opMode.byte = CANMODE_DEFAULT;
    int m_handle = -1;

        for(int x = 0; x < CAN_MAX_HANDLES; x++)
        {
            if(x == 0)
                rc = ::can_property((-1), CANPROP_SET_FIRST_CHANNEL, NULL, 0U);
            else
                rc = ::can_property((-1), CANPROP_SET_NEXT_CHANNEL, NULL, 0U);

            if (CANERR_NOERROR == rc) {
                ::can_property((-1), CANPROP_GET_CHANNEL_NO, (void *) &info.m_nChannelNo,
                                                                                sizeof(int32_t));
                ::can_property((-1), CANPROP_GET_CHANNEL_NAME, (void *) &info.m_szDeviceName,
                                                                                CANPROP_MAX_BUFFER_SIZE);
                ::can_property((-1), CANPROP_GET_CHANNEL_VENDOR_NAME, (void *) &info.m_szVendorName,
                                                                                CANPROP_MAX_BUFFER_SIZE);
                rc = ::can_test(info.m_nChannelNo, opMode.byte, NULL, &state);
                //m_serial = can_hardware(info.m_nChannelNo);

                if ((0 <= rc) && (state == CANBRD_PRESENT))
                {
                    m_handle = ::can_init(info.m_nChannelNo, 0, NULL);
                    ::can_property(m_handle, TOUCAN_GET_SERIAL_NUMBER, &info.m_nSerialNumber, sizeof(int32_t));
                    char string[CANPROP_MAX_BUFFER_SIZE] = "(unknown)";
                    snprintf(string, 10, "%08X",info.m_nSerialNumber);
                    ::can_exit(m_handle);

                    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "TOUCAN_GET_SERIAL_NUMBER: %s", string);
                    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "CANPROP_GET_CHANNEL_NO: %d", info.m_nChannelNo);
                    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "CANPROP_GET_CHANNEL_NAME: %s", info.m_szDeviceName);
                    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "CANPROP_GET_CHANNEL_VENDOR_NAME: %s", info.m_szVendorName);

                    result.append(std::move(createDeviceInfo(QLatin1String(info.m_szDeviceName), QLatin1String(string),
                                                          QLatin1String(info.m_szVendorName), info.m_nChannelNo,
                                                          false, false)));
                }
            }
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

    uint8_t status;
    int rc;
    rc = ::can_status(d_ptr->handle, &status);

    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "handle: %d", d_ptr->handle);
    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "CAN_STATUS: %d", status);

    if(rc != CANERR_NOERROR) {
        qCWarning(QT_CANBUS_PLUGINS_RUSOKUCAN, "Unknown CAN bus status: %lu.", ulong(status));
        return QCanBusDevice::CanBusStatus::Unknown;
    }

    switch (status & 0xF0) {
        case 0:
            return QCanBusDevice::CanBusStatus::Good;
        case CANSTAT_EWRN:
            return QCanBusDevice::CanBusStatus::Warning;
        case CANSTAT_BERR:
            return QCanBusDevice::CanBusStatus::Error;
        case CANSTAT_BOFF:
            return QCanBusDevice::CanBusStatus::BusOff;
        default:
            qCWarning(QT_CANBUS_PLUGINS_RUSOKUCAN, "Unknown CAN bus status: %lu.", ulong(status));
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

    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "RusokuCanBackendPrivate::open()");

    const int nominalBitrate = q->configurationParameter(QCanBusDevice::BitRateKey).toInt();
    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "--- nominal bitrate = %d", nominalBitrate);

    if ((handle = ::can_init(channelIndex, CANMODE_DEFAULT, NULL)) < CANERR_NOERROR){
        qCCritical(QT_CANBUS_PLUGINS_RUSOKUCAN, "Cannot init hardware");
        q->setError("Cannot init hardware", QCanBusDevice::ConnectionError);
        return false;
    }

    can_bitrate_t bitrate;
    switch (nominalBitrate) {
        case 1000000: bitrate.index = (int32_t)CANBTR_INDEX_1M; break;
        case 800000:  bitrate.index = (int32_t)CANBTR_INDEX_800K; break;
        case 500000:  bitrate.index = (int32_t)CANBTR_INDEX_500K; break;
        case 250000:  bitrate.index = (int32_t)CANBTR_INDEX_250K; break;
        case 125000:  bitrate.index = (int32_t)CANBTR_INDEX_125K; break;
        case 100000:  bitrate.index = (int32_t)CANBTR_INDEX_100K; break;
        case 50000:   bitrate.index = (int32_t)CANBTR_INDEX_50K; break;
        case 20000:   bitrate.index = (int32_t)CANBTR_INDEX_20K; break;
        case 10000:   bitrate.index = (int32_t)CANBTR_INDEX_10K; break;
        default:      bitrate.index = (int32_t)CANBTR_INDEX_500K; break;
    }

    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "--- handle = %d", handle);
    if ((::can_start(handle, &bitrate)) < CANERR_NOERROR) {
        qCCritical(QT_CANBUS_PLUGINS_RUSOKUCAN, "Cannot init hardware");
        q->setError("Cannot start hardware", QCanBusDevice::ConnectionError);
        return false;
    }

    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "--- handle = %d", handle);

    writeNotifier = new RusokuCanWriteNotifier(this, q);
    writeNotifier->setInterval(1);

    readNotifier = new RusokuCanReadNotifier(this, q);
    readNotifier->setInterval(1);
    readNotifier->start();

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

    ::can_exit(handle);

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

static const can_board_t can_boards[CAN_MAX_HANDLES+1] = {  // list of supported CAN Interfaces
        {TOUCAN_USB_CHANNEL0, (char *)"TouCAN-USB1"},
        {TOUCAN_USB_CHANNEL1, (char *)"TouCAN-USB2"},
        {TOUCAN_USB_CHANNEL2, (char *)"TouCAN-USB3"},
        {TOUCAN_USB_CHANNEL3, (char *)"TouCAN-USB4"},
        {TOUCAN_USB_CHANNEL4, (char *)"TouCAN-USB5"},
        {TOUCAN_USB_CHANNEL5, (char *)"TouCAN-USB6"},
        {TOUCAN_USB_CHANNEL6, (char *)"TouCAN-USB7"},
        {TOUCAN_USB_CHANNEL7, (char *)"TouCAN-USB8"},
        {EOF, NULL}
};

void RusokuCanBackendPrivate::setupChannel(const QByteArray &interfaceName)
{
    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "RusokuCanBackendPrivate::setupChannel()");

    const can_board_t *channel = can_boards;
    while (channel->type != EOF && channel->name != interfaceName)
        ++channel;
    channelIndex = channel->type;

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

    if (!q->hasOutgoingFrames()) {
        writeNotifier->stop();
        qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, " writeNotifier->stop()");
        return;
    }

    const QCanBusFrame frame = q->dequeueOutgoingFrame();
    const QByteArray payload = frame.payload();

    can_message_t message = {};
    message.id = frame.frameId();
    message.dlc = static_cast<quint8>(payload.size());
    message.xtd = frame.hasExtendedFrameFormat();
    message.timestamp.tv_sec = 0;

    if (frame.frameType() == QCanBusFrame::RemoteRequestFrame){
        message.rtr = true;
    }else{
        ::memcpy(message.data, payload.constData(), sizeof(message.data));
    }

    int st = CANERR_ONLINE;
    st = ::can_write(handle, &message, 0);
    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "--- can_write");
    qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "--- handle = %d", handle);

    if (Q_UNLIKELY(st != CANERR_NOERROR)) {
        qCWarning(QT_CANBUS_PLUGINS_RUSOKUCAN, "Cannot write frame errcode = %d", st);
    }else{
        emit q->framesWritten(qint64(1));
    }

    if (q->hasOutgoingFrames() && !writeNotifier->isActive())
        writeNotifier->start();
}

void RusokuCanBackendPrivate::startRead()
{
    Q_Q(RusokuCanBackend);

    //qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "RusokuCanBackendPrivate::startRead()");

    QVector<QCanBusFrame> newFrames;
    int st = CANERR_ONLINE;
    can_message_t message = {};

    for(;;) {
        st = ::can_read(handle, &message, 0);

        if (st != CANERR_NOERROR) {
            //q->setError(systemErrorString(st), QCanBusDevice::ReadError);
            //qCWarning(QT_CANBUS_PLUGINS_RUSOKUCAN, "Cannot read frame, err_code = %d", st);
            //return;
            break;
        }

        if(message.esi)
            break;

        const int size = message.dlc;
        QCanBusFrame frame(message.id,
                           QByteArray(reinterpret_cast<const char *>(message.data), size));
        frame.setExtendedFrameFormat(message.xtd);
        frame.setFrameType((message.rtr)
                           ? QCanBusFrame::RemoteRequestFrame : QCanBusFrame::DataFrame);

        frame.setTimeStamp(QCanBusFrame::TimeStamp::fromMicroSeconds(static_cast<qint64>(message.timestamp.tv_nsec)));
        newFrames.append(std::move(frame));
    }
    q->enqueueReceivedFrames(newFrames);
    //qCInfo(QT_CANBUS_PLUGINS_RUSOKUCAN, "RX frame list received()");
}

bool RusokuCanBackendPrivate::verifyBitRate(int bitrate)
{
    Q_Q(RusokuCanBackend);

    return true;
}

QT_END_NAMESPACE

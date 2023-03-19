
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


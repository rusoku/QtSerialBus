
#include "rusokucanbackend_p.h"
#include <QtCore/qloggingcategory.h>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_CANBUS_PLUGINS_RUSOKUCAN)

RusokuCanBackendPrivate::RusokuCanBackendPrivate(RusokuCanBackend *q)
        : q_ptr(q)
{
}

bool RusokuCanBackendPrivate::open() {
    Q_Q(RusokuCanBackend);

    return true;
}

void RusokuCanBackendPrivate::close()
{
    Q_Q(RusokuCanBackend);
}

bool RusokuCanBackendPrivate::setConfigurationParameter(int key, const QVariant &value)
{
    Q_Q(RusokuCanBackend);

    return true;
}

void RusokuCanBackendPrivate::setupChannel(const QByteArray &interfaceName)
{

}

// Calls only when the device is closed
void RusokuCanBackendPrivate::setupDefaultConfigurations()
{
    Q_Q(RusokuCanBackend);

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


/*
    Copyright (C) 2014 Alexandr Akulich <akulichalexander@gmail.com>

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
    LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
    OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include "textchannel.h"

#include <TelepathyQt/Constants>
#include <TelepathyQt/RequestableChannelClassSpec>
#include <TelepathyQt/RequestableChannelClassSpecList>
#include <TelepathyQt/Types>

#include <QLatin1String>
#include <QVariantMap>

#include <QDebug>

SimpleTextChannel::SimpleTextChannel(Tp::BaseChannel *baseChannel)
    : Tp::BaseChannelTextType(baseChannel),
      m_targetHandle(baseChannel->targetHandle()),
      m_targetID(baseChannel->targetID())
{
    QStringList supportedContentTypes = QStringList() << QLatin1String("text/plain");
    Tp::UIntList messageTypes = Tp::UIntList() << Tp::ChannelTextMessageTypeNormal;

    uint messagePartSupportFlags = 0;
    uint deliveryReportingSupport = 0;

    m_messagesIface = Tp::BaseChannelMessagesInterface::create(this,
                                                               supportedContentTypes,
                                                               messageTypes,
                                                               messagePartSupportFlags,
                                                               deliveryReportingSupport);

    baseChannel->plugInterface(Tp::AbstractChannelInterfacePtr::dynamicCast(m_messagesIface));

    m_messagesIface->setSendMessageCallback(Tp::memFun(this, &SimpleTextChannel::sendMessageCallback));
}

SimpleTextChannelPtr SimpleTextChannel::create(Tp::BaseChannel *baseChannel)
{
    return SimpleTextChannelPtr(new SimpleTextChannel(baseChannel));
}

SimpleTextChannel::~SimpleTextChannel()
{
}

QString SimpleTextChannel::sendMessageCallback(const Tp::MessagePartList &messageParts, uint flags, Tp::DBusError *error)
{
    QString content;
    for (Tp::MessagePartList::const_iterator i = messageParts.begin()+1; i != messageParts.end(); ++i) {
        if(i->count(QLatin1String("content-type"))
            && i->value(QLatin1String("content-type")).variant().toString() == QLatin1String("text/plain")
            && i->count(QLatin1String("content")))
        {
            content = i->value(QLatin1String("content")).variant().toString();
            break;
        }
    }

    emit sendMessage(m_targetID, content);

    return QString();
}

void SimpleTextChannel::addIncomingMessage(const QString &message)
{
    uint timestamp = QDateTime::currentMSecsSinceEpoch() / 1000;

    Tp::MessagePartList body;
    Tp::MessagePart text;
    text[QLatin1String("content-type")] = QDBusVariant(QLatin1String("text/plain"));
    text[QLatin1String("content")]      = QDBusVariant(message);
    body << text;

    Tp::MessagePartList partList;
    Tp::MessagePart header;
    header[QLatin1String("message-received")]  = QDBusVariant(timestamp);
    header[QLatin1String("message-sender")]    = QDBusVariant(m_targetHandle);
    header[QLatin1String("message-sender-id")] = QDBusVariant(m_targetID);
    header[QLatin1String("message-type")]      = QDBusVariant(Tp::ChannelTextMessageTypeNormal);

    partList << header << body;
    addReceivedMessage(partList);
}

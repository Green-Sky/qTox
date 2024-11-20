/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "src/core/icoreconferencemessagesender.h"
#include "src/core/icoreidhandler.h"
#include "src/model/conference.h"
#include "src/model/imessagedispatcher.h"
#include "src/model/message.h"

#include <QObject>
#include <QString>

#include <cstdint>

class IConferenceSettings;

class ConferenceMessageDispatcher : public IMessageDispatcher
{
    Q_OBJECT
public:
    ConferenceMessageDispatcher(Conference& g_, MessageProcessor processor, ICoreIdHandler& idHandler,
                                ICoreConferenceMessageSender& messageSender,
                                const IConferenceSettings& conferenceSettings);

    std::pair<DispatchedMessageId, DispatchedMessageId> sendMessage(bool isAction,
                                                                    QString const& content) override;

    std::pair<DispatchedMessageId, DispatchedMessageId>
    sendExtendedMessage(const QString& content, ExtensionSet extensions) override;
    void onMessageReceived(ToxPk const& sender, bool isAction, QString const& content);

private:
    Conference& conference;
    MessageProcessor processor;
    ICoreIdHandler& idHandler;
    ICoreConferenceMessageSender& messageSender;
    const IConferenceSettings& conferenceSettings;
    DispatchedMessageId nextMessageId{0};
};
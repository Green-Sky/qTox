/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "src/core/icoreconferencemessagesender.h"
#include "src/friendlist.h"
#include "src/model/conference.h"
#include "src/model/conferencemessagedispatcher.h"
#include "src/model/message.h"
#include "src/persistence/iconferencesettings.h"
#include "util/interface.h"

#include "mock/mockconferencequery.h"
#include "mock/mockcoreidhandler.h"

#include <QObject>
#include <QtTest/QtTest>

#include <deque>
#include <memory>
#include <set>

#include <tox/tox.h> // tox_max_message_length

class MockConferenceMessageSender : public ICoreConferenceMessageSender
{
public:
    void sendConferenceAction(int conferenceId, const QString& action) override;

    void sendConferenceMessage(int conferenceId, const QString& message) override;

    size_t numSentActions = 0;
    size_t numSentMessages = 0;
};

void MockConferenceMessageSender::sendConferenceAction(int conferenceId, const QString& action)
{
    std::ignore = conferenceId;
    std::ignore = action;
    numSentActions++;
}

void MockConferenceMessageSender::sendConferenceMessage(int conferenceId, const QString& message)
{
    std::ignore = conferenceId;
    std::ignore = message;
    numSentMessages++;
}

class MockConferenceSettings : public QObject, public IConferenceSettings
{
    Q_OBJECT

public:
    QStringList getBlockList() const override;
    void setBlockList(const QStringList& blist) override;
    SIGNAL_IMPL(MockConferenceSettings, blockListChanged, const QStringList& blist)

    bool getShowConferenceJoinLeaveMessages() const override
    {
        return true;
    }
    void setShowConferenceJoinLeaveMessages(bool newValue) override
    {
        std::ignore = newValue;
    }
    SIGNAL_IMPL(MockConferenceSettings, showConferenceJoinLeaveMessagesChanged, bool show)

private:
    QStringList blockList;
};

QStringList MockConferenceSettings::getBlockList() const
{
    return blockList;
}

void MockConferenceSettings::setBlockList(const QStringList& blist)
{
    blockList = blist;
}

class TestConferenceMessageDispatcher : public QObject
{
    Q_OBJECT

public:
    TestConferenceMessageDispatcher();

private slots:
    void init();
    void testSignals();
    void testMessageSending();
    void testEmptyConference();
    void testSelfReceive();
    void testBlockList();

    void onMessageSent(DispatchedMessageId id, Message message)
    {
        auto it = outgoingMessages.find(id);
        QVERIFY(it == outgoingMessages.end());
        outgoingMessages.emplace(id);
        sentMessages.push_back(std::move(message));
    }

    void onMessageComplete(DispatchedMessageId id)
    {
        auto it = outgoingMessages.find(id);
        QVERIFY(it != outgoingMessages.end());
        outgoingMessages.erase(it);
    }

    void onMessageReceived(const ToxPk& sender, Message message)
    {
        std::ignore = sender;
        receivedMessages.push_back(std::move(message));
    }

private:
    // All unique_ptrs to make construction/init() easier to manage
    std::unique_ptr<MockConferenceSettings> conferenceSettings;
    std::unique_ptr<MockConferenceQuery> conferenceQuery;
    std::unique_ptr<MockCoreIdHandler> coreIdHandler;
    std::unique_ptr<Conference> g;
    std::unique_ptr<MockConferenceMessageSender> messageSender;
    std::unique_ptr<MessageProcessor::SharedParams> sharedProcessorParams;
    std::unique_ptr<MessageProcessor> messageProcessor;
    std::unique_ptr<ConferenceMessageDispatcher> conferenceMessageDispatcher;
    std::set<DispatchedMessageId> outgoingMessages;
    std::deque<Message> sentMessages;
    std::deque<Message> receivedMessages;
    std::unique_ptr<FriendList> friendList;
};

TestConferenceMessageDispatcher::TestConferenceMessageDispatcher() = default;

/**
 * @brief Test initialization. Resets all members to initial state
 */
void TestConferenceMessageDispatcher::init()
{
    friendList = std::make_unique<FriendList>();
    conferenceSettings = std::make_unique<MockConferenceSettings>();
    conferenceQuery = std::make_unique<MockConferenceQuery>();
    coreIdHandler = std::make_unique<MockCoreIdHandler>();
    g = std::make_unique<Conference>(0, ConferenceId(), "TestConference", false, "me",
                                     *conferenceQuery, *coreIdHandler, *friendList);
    messageSender = std::make_unique<MockConferenceMessageSender>();
    sharedProcessorParams = std::make_unique<MessageProcessor::SharedParams>(tox_max_message_length());
    messageProcessor = std::make_unique<MessageProcessor>(*sharedProcessorParams);
    conferenceMessageDispatcher =
        std::make_unique<ConferenceMessageDispatcher>(*g, *messageProcessor, *coreIdHandler,
                                                      *messageSender, *conferenceSettings);

    connect(conferenceMessageDispatcher.get(), &ConferenceMessageDispatcher::messageSent, this,
            &TestConferenceMessageDispatcher::onMessageSent);
    connect(conferenceMessageDispatcher.get(), &ConferenceMessageDispatcher::messageComplete, this,
            &TestConferenceMessageDispatcher::onMessageComplete);
    connect(conferenceMessageDispatcher.get(), &ConferenceMessageDispatcher::messageReceived, this,
            &TestConferenceMessageDispatcher::onMessageReceived);

    outgoingMessages = std::set<DispatchedMessageId>();
    sentMessages = std::deque<Message>();
    receivedMessages = std::deque<Message>();
}

/**
 * @brief Tests that the signals emitted by the dispatcher are all emitted at the correct times
 */
void TestConferenceMessageDispatcher::testSignals()
{
    conferenceMessageDispatcher->sendMessage(false, "test");

    // For conferences we pair our sent and completed signals since we have no receiver reports
    QVERIFY(outgoingMessages.empty());
    QVERIFY(!sentMessages.empty());
    QVERIFY(sentMessages.front().isAction == false);
    QVERIFY(sentMessages.front().content == "test");

    // If signals are emitted correctly we should have one message in our received message buffer
    QVERIFY(receivedMessages.empty());
    conferenceMessageDispatcher->onMessageReceived(ToxPk(), false, "test2");

    QVERIFY(!receivedMessages.empty());
    QVERIFY(receivedMessages.front().isAction == false);
    QVERIFY(receivedMessages.front().content == "test2");
}

/**
 * @brief Tests that sent messages actually go through to core
 */
void TestConferenceMessageDispatcher::testMessageSending()
{
    conferenceMessageDispatcher->sendMessage(false, "Test");

    QVERIFY(messageSender->numSentMessages == 1);
    QVERIFY(messageSender->numSentActions == 0);

    conferenceMessageDispatcher->sendMessage(true, "Test");

    QVERIFY(messageSender->numSentMessages == 1);
    QVERIFY(messageSender->numSentActions == 1);
}

/**
 * @brief Tests that if we are the only member in a conference we do _not_ send messages to core.
 * Toxcore isn't too happy if we send messages and we're the only one in the conference
 */
void TestConferenceMessageDispatcher::testEmptyConference()
{
    conferenceQuery->setAsEmptyConference();
    g->regeneratePeerList();

    conferenceMessageDispatcher->sendMessage(false, "Test");
    conferenceMessageDispatcher->sendMessage(true, "Test");

    QVERIFY(messageSender->numSentMessages == 0);
    QVERIFY(messageSender->numSentActions == 0);
}

/**
 * @brief Tests that we do not emit any signals if we receive a message from ourself. Toxcore will send us back messages we sent
 */
void TestConferenceMessageDispatcher::testSelfReceive()
{
    uint8_t selfId[ToxPk::size] = {0};
    conferenceMessageDispatcher->onMessageReceived(ToxPk(selfId), false, "Test");
    QVERIFY(receivedMessages.empty());

    uint8_t id[ToxPk::size] = {1};
    conferenceMessageDispatcher->onMessageReceived(ToxPk(id), false, "Test");
    QVERIFY(receivedMessages.size() == 1);
}

/**
 * @brief Tests that messages from block-listed peers do not get propagated from the dispatcher
 */
void TestConferenceMessageDispatcher::testBlockList()
{
    uint8_t id[ToxPk::size] = {1};
    auto otherPk = ToxPk(id);
    conferenceMessageDispatcher->onMessageReceived(otherPk, false, "Test");
    QVERIFY(receivedMessages.size() == 1);

    conferenceSettings->setBlockList({otherPk.toString()});
    conferenceMessageDispatcher->onMessageReceived(otherPk, false, "Test");
    QVERIFY(receivedMessages.size() == 1);
}

// Cannot be guiless due to a settings instance in ConferenceMessageDispatcher
QTEST_GUILESS_MAIN(TestConferenceMessageDispatcher)
#include "conferencemessagedispatcher_test.moc"

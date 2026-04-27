/**
 * @file InMemoryEventBusTest.cpp
 * @brief Unit tests for InMemoryEventBus
 *
 * Covers: publish/subscribe lifecycle, multiple handlers, exception policies,
 * message recording, start/stop semantics.
 */

#include <gtest/gtest.h>
#include "adapters/secondary/InMemoryEventBus.hpp"

class InMemoryEventBusTest : public ::testing::Test {
protected:
    InMemoryEventBus bus;
};

// ============================================================================
// Publish / Subscribe basics
// ============================================================================

TEST_F(InMemoryEventBusTest, Publish_AfterSubscribe_HandlerReceivesMessage) {
    std::string receivedKey;
    std::string receivedMsg;
    bus.subscribe({"order.create"}, [&](const std::string& key, const std::string& msg) {
        receivedKey = key;
        receivedMsg = msg;
    });
    bus.start();
    bus.publish("order.create", R"({"order_id":"ord-1"})");

    EXPECT_EQ(receivedKey, "order.create");
    EXPECT_EQ(receivedMsg, R"({"order_id":"ord-1"})");
}

TEST_F(InMemoryEventBusTest, Publish_MultipleHandlers_SameKey_AllReceive) {
    int callCount = 0;
    bus.subscribe({"order.create"}, [&](const std::string&, const std::string&) { callCount++; });
    bus.subscribe({"order.create"}, [&](const std::string&, const std::string&) { callCount += 10; });
    bus.start();
    bus.publish("order.create", "test");

    EXPECT_EQ(callCount, 11);
}

TEST_F(InMemoryEventBusTest, Publish_MultipleRoutingKeys_AllHandlersReceive) {
    std::vector<std::string> receivedKeys;
    bus.subscribe({"order.create", "order.cancel"}, [&](const std::string& key, const std::string&) {
        receivedKeys.push_back(key);
    });
    bus.start();
    bus.publish("order.create", "msg1");
    bus.publish("order.cancel", "msg2");

    EXPECT_EQ(receivedKeys.size(), 2u);
    EXPECT_EQ(receivedKeys[0], "order.create");
    EXPECT_EQ(receivedKeys[1], "order.cancel");
}

TEST_F(InMemoryEventBusTest, Publish_NoSubscribers_MessageRecorded) {
    bus.start();
    bus.publish("order.create", R"({"order_id":"ord-1"})");

    EXPECT_EQ(bus.publishedCount(), 1u);
    EXPECT_EQ(bus.publishedMessages()[0].routingKey, "order.create");
}

TEST_F(InMemoryEventBusTest, Publish_UnknownRoutingKey_NoHandlerCalled) {
    int callCount = 0;
    bus.subscribe({"order.create"}, [&](const std::string&, const std::string&) { callCount++; });
    bus.start();
    bus.publish("quote.updated", "test");

    EXPECT_EQ(callCount, 0);
    EXPECT_EQ(bus.publishedCount(), 1u);
}

// ============================================================================
// Lifecycle: start / stop
// ============================================================================

TEST_F(InMemoryEventBusTest, Subscribe_BeforeStart_SubscriptionsAccumulate) {
    EXPECT_EQ(bus.subscriptionCount(), 0u);
    bus.subscribe({"order.create"}, [](const std::string&, const std::string&) {});
    EXPECT_EQ(bus.subscriptionCount(), 1u);
    bus.subscribe({"order.cancel"}, [](const std::string&, const std::string&) {});
    EXPECT_EQ(bus.subscriptionCount(), 2u);
}

TEST_F(InMemoryEventBusTest, Publish_BeforeStart_MessageRecordedNoDispatch) {
    int callCount = 0;
    bus.subscribe({"order.create"}, [&](const std::string&, const std::string&) { callCount++; });
    bus.publish("order.create", "test");

    EXPECT_EQ(callCount, 0);
    EXPECT_EQ(bus.publishedCount(), 1u);
}

TEST_F(InMemoryEventBusTest, Publish_AfterStart_DispatchesToHandlers) {
    int callCount = 0;
    bus.subscribe({"order.create"}, [&](const std::string&, const std::string&) { callCount++; });
    bus.start();
    bus.publish("order.create", "test");

    EXPECT_EQ(callCount, 1);
}

TEST_F(InMemoryEventBusTest, Stop_PreventsDispatch) {
    int callCount = 0;
    bus.subscribe({"order.create"}, [&](const std::string&, const std::string&) { callCount++; });
    bus.start();
    bus.publish("order.create", "msg1");
    EXPECT_EQ(callCount, 1);

    bus.stop();
    bus.publish("order.create", "msg2");
    EXPECT_EQ(callCount, 1);
    EXPECT_EQ(bus.publishedCount(), 2u);
}

TEST_F(InMemoryEventBusTest, Start_Idempotent) {
    int callCount = 0;
    bus.subscribe({"test"}, [&](const std::string&, const std::string&) { callCount++; });
    bus.start();
    bus.start();
    bus.publish("test", "msg");

    EXPECT_EQ(callCount, 1);
}

TEST_F(InMemoryEventBusTest, Stop_WhenNotStarted_Noop) {
    bus.stop();
    EXPECT_FALSE(bus.isRunning());
}

// ============================================================================
// Message recording
// ============================================================================

TEST_F(InMemoryEventBusTest, PublishedMessages_RecordedInOrder) {
    bus.start();
    bus.publish("order.create", "msg1");
    bus.publish("order.filled", "msg2");
    bus.publish("order.cancelled", "msg3");

    EXPECT_EQ(bus.publishedCount(), 3u);
    EXPECT_EQ(bus.publishedMessages()[0].routingKey, "order.create");
    EXPECT_EQ(bus.publishedMessages()[1].routingKey, "order.filled");
    EXPECT_EQ(bus.publishedMessages()[2].routingKey, "order.cancelled");
}

TEST_F(InMemoryEventBusTest, PublishedCount_ReturnsCorrectCount) {
    bus.start();
    EXPECT_EQ(bus.publishedCount(), 0u);
    bus.publish("test", "a");
    EXPECT_EQ(bus.publishedCount(), 1u);
    bus.publish("test", "b");
    bus.publish("test", "c");
    EXPECT_EQ(bus.publishedCount(), 3u);
}

TEST_F(InMemoryEventBusTest, Clear_ResetsAllState) {
    bus.subscribe({"test"}, [](const std::string&, const std::string&) {});
    bus.start();
    bus.publish("test", "msg");
    EXPECT_EQ(bus.publishedCount(), 1u);
    EXPECT_EQ(bus.subscriptionCount(), 1u);

    bus.clear();

    EXPECT_EQ(bus.publishedCount(), 0u);
    EXPECT_EQ(bus.subscriptionCount(), 0u);
    EXPECT_FALSE(bus.isRunning());
}

// ============================================================================
// Exception handling
// ============================================================================

TEST_F(InMemoryEventBusTest, ExceptionPolicy_Catch_StoresError) {
    bus.setExceptionPolicy(InMemoryEventBus::ExceptionPolicy::Catch);
    bus.subscribe({"test"}, [](const std::string&, const std::string&) {
        throw std::runtime_error("handler error");
    });
    bus.start();
    bus.publish("test", "msg");

    EXPECT_EQ(bus.errors().size(), 1u);
    EXPECT_EQ(bus.errors()[0].first, "test");
    EXPECT_EQ(bus.errors()[0].second, "handler error");
}

TEST_F(InMemoryEventBusTest, ExceptionPolicy_Catch_OtherHandlersStillRun) {
    int secondHandlerCalled = 0;
    bus.setExceptionPolicy(InMemoryEventBus::ExceptionPolicy::Catch);
    bus.subscribe({"test"}, [](const std::string&, const std::string&) {
        throw std::runtime_error("fail");
    });
    bus.subscribe({"test"}, [&](const std::string&, const std::string&) {
        secondHandlerCalled++;
    });
    bus.start();
    bus.publish("test", "msg");

    EXPECT_EQ(secondHandlerCalled, 1);
    EXPECT_EQ(bus.errors().size(), 1u);
}

TEST_F(InMemoryEventBusTest, ExceptionPolicy_Propagate_ExceptionReachesCaller) {
    bus.setExceptionPolicy(InMemoryEventBus::ExceptionPolicy::Propagate);
    bus.subscribe({"test"}, [](const std::string&, const std::string&) {
        throw std::runtime_error("propagated error");
    });
    bus.start();

    EXPECT_THROW(bus.publish("test", "msg"), std::runtime_error);
}

// ============================================================================
// Test helpers
// ============================================================================

TEST_F(InMemoryEventBusTest, HandlerCount_ReturnsCorrectCount) {
    bus.subscribe({"order.create"}, [](const std::string&, const std::string&) {});
    bus.subscribe({"order.create"}, [](const std::string&, const std::string&) {});
    bus.subscribe({"order.cancel"}, [](const std::string&, const std::string&) {});

    EXPECT_EQ(bus.handlerCount("order.create"), 2u);
    EXPECT_EQ(bus.handlerCount("order.cancel"), 1u);
    EXPECT_EQ(bus.handlerCount("unknown"), 0u);
}

TEST_F(InMemoryEventBusTest, SubscriptionCount_ReturnsCorrectCount) {
    bus.subscribe({"order.create"}, [](const std::string&, const std::string&) {});
    EXPECT_EQ(bus.subscriptionCount(), 1u);
    bus.subscribe({"order.cancel"}, [](const std::string&, const std::string&) {});
    EXPECT_EQ(bus.subscriptionCount(), 2u);
}

TEST_F(InMemoryEventBusTest, Subscribe_EmptyRoutingKeyList_NoSubscription) {
    bus.subscribe({}, [](const std::string&, const std::string&) {});
    EXPECT_EQ(bus.subscriptionCount(), 0u);
}

TEST_F(InMemoryEventBusTest, IsRunning_AfterStart_ReturnsTrue) {
    EXPECT_FALSE(bus.isRunning());
    bus.start();
    EXPECT_TRUE(bus.isRunning());
}

TEST_F(InMemoryEventBusTest, IsRunning_AfterStop_ReturnsFalse) {
    bus.start();
    EXPECT_TRUE(bus.isRunning());
    bus.stop();
    EXPECT_FALSE(bus.isRunning());
}
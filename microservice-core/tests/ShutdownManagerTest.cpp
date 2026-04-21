#include <gtest/gtest.h>
#include "application/ShutdownManager.hpp"
#include "ports/output/ILogger.hpp"
#include <atomic>
#include <chrono>

/**
 * @file ShutdownManagerTest.cpp
 * @brief Unit tests for ShutdownManager
 */

class MockShutdownComponent : public IShutdown
{
public:
    std::atomic<bool> shutdownCalled{false};
    std::atomic<int> shutdownOrder{0};
    std::string componentName;

    static int globalOrder;

    explicit MockShutdownComponent(const std::string &name) : componentName(name) {}

    void shutdown(std::chrono::milliseconds /*timeoutMs*/) override
    {
        shutdownCalled.store(true);
        shutdownOrder.store(++globalOrder);
    }

    std::string name() const override
    {
        return componentName;
    }
};

int MockShutdownComponent::globalOrder = 0;

class ShutdownManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        MockShutdownComponent::globalOrder = 0;
    }
};

TEST_F(ShutdownManagerTest, ShutdownAll_ReverseOrder)
{
    auto mgr = std::make_shared<ShutdownManager>();
    auto first = std::make_shared<MockShutdownComponent>("First");
    auto second = std::make_shared<MockShutdownComponent>("Second");
    auto third = std::make_shared<MockShutdownComponent>("Third");

    mgr->registerComponent(first);
    mgr->registerComponent(second);
    mgr->registerComponent(third);

    mgr->shutdownAll();

    EXPECT_TRUE(first->shutdownCalled.load());
    EXPECT_TRUE(second->shutdownCalled.load());
    EXPECT_TRUE(third->shutdownCalled.load());

    // LIFO: Third shut down first (order=1), Second (order=2), First (order=3)
    EXPECT_LT(third->shutdownOrder.load(), second->shutdownOrder.load());
    EXPECT_LT(second->shutdownOrder.load(), first->shutdownOrder.load());
}

TEST_F(ShutdownManagerTest, ShutdownAll_SingleComponent)
{
    ShutdownManager mgr;
    auto component = std::make_shared<MockShutdownComponent>("Only");

    mgr.registerComponent(component);
    mgr.shutdownAll();

    EXPECT_TRUE(component->shutdownCalled.load());
}

TEST_F(ShutdownManagerTest, ShutdownAll_EmptyManager)
{
    ShutdownManager mgr;
    EXPECT_EQ(mgr.size(), 0u);
    mgr.shutdownAll();
}

TEST_F(ShutdownManagerTest, Size_ReturnsComponentCount)
{
    ShutdownManager mgr;
    EXPECT_EQ(mgr.size(), 0u);

    mgr.registerComponent(std::make_shared<MockShutdownComponent>("A"));
    EXPECT_EQ(mgr.size(), 1u);

    mgr.registerComponent(std::make_shared<MockShutdownComponent>("B"));
    EXPECT_EQ(mgr.size(), 2u);
}

TEST_F(ShutdownManagerTest, IShutdown_InterfaceHasVirtualShutdown)
{
    auto component = std::make_shared<MockShutdownComponent>("Test");
    IShutdown *shutdownPtr = component.get();
    EXPECT_NE(shutdownPtr, nullptr);
    EXPECT_EQ(shutdownPtr->name(), "Test");
}
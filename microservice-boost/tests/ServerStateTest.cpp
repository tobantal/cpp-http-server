#include <gtest/gtest.h>
#include "adapters/primary/BoostBeastApplication.hpp"
#include "adapters/secondary/Environment.hpp"

class TestApp : public BoostBeastApplication
{
public:
    TestApp()
    {
        env_ = std::make_shared<Environment>();
    }

    void loadEnvironment(int, char *[]) override {}
    void configureInjection() override {}

    using BoostBeastApplication::registerHandler;
};

namespace
{
    class DummyHandler : public IHttpHandler
    {
    public:
        void handle(IRequest &, IResponse &res) override
        {
            res.setResult(200, "text/plain", "ok");
        }
    };
}

TEST(ServerStateTest, RegisterHandlerAllowedBeforeStart)
{
    TestApp app;
    EXPECT_NO_THROW(app.registerHandler("GET", "/test", std::make_shared<DummyHandler>()));
}

TEST(ServerStateTest, RegisterHandlerMultipleAllowedBeforeStart)
{
    TestApp app;
    EXPECT_NO_THROW(app.registerHandler("GET", "/a", std::make_shared<DummyHandler>()));
    EXPECT_NO_THROW(app.registerHandler("POST", "/b", std::make_shared<DummyHandler>()));
    EXPECT_NO_THROW(app.registerHandler("DELETE", "/c/*", std::make_shared<DummyHandler>()));
}

TEST(ServerStateTest, StopWithoutStartIsNoop)
{
    TestApp app;
    EXPECT_NO_THROW(app.stop());
}

TEST(ServerStateTest, DoubleStopWithoutStartIsNoop)
{
    TestApp app;
    EXPECT_NO_THROW(app.stop());
    EXPECT_NO_THROW(app.stop());
}

TEST(ServerStateTest, EnumValues)
{
    EXPECT_NE(ServerState::NotStarted, ServerState::Running);
    EXPECT_NE(ServerState::Running, ServerState::Stopped);
    EXPECT_NE(ServerState::NotStarted, ServerState::Stopped);
}

TEST(ServerStateTest, EnumCoversAllStates)
{
    ServerState states[] = {ServerState::NotStarted, ServerState::Running, ServerState::Stopped};
    EXPECT_EQ(states[0], ServerState::NotStarted);
    EXPECT_EQ(states[1], ServerState::Running);
    EXPECT_EQ(states[2], ServerState::Stopped);
}
#include "AppControllerTest.hpp"

using ::testing::_;

void AppControllerTest::SetUp() {
    mockUiTask = std::make_unique<core::MockUiTask>();
    appController = std::make_unique<core::AppController>(*mockUiTask);
}

void AppControllerTest::TearDown() {
    appController.reset();
    mockUiTask.reset();
}

TEST_F(AppControllerTest, init_Success) {
    // Arrange
    common::UiEvent expectedEvent;
    expectedEvent.type = common::UiEvent::Type::RENDER_STATIONS;
    expectedEvent.selectedIndex = 0;

    // Expect
    EXPECT_CALL(*mockUiTask, post(_)).WillOnce([](const common::UiEvent &e) {
        EXPECT_EQ(e.type, common::UiEvent::Type::RENDER_STATIONS);
        EXPECT_EQ(e.selectedIndex, 0);
    });

    // Act
    appController->init();
}

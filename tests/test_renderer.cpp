#include <gtest/gtest.h>
#include "../src/renderer.cpp" // Include the renderer implementation for testing

class RendererTest : public ::testing::Test {
protected:
    Renderer* renderer;

    void SetUp() override {
        renderer = new Renderer();
        // Additional setup code if needed
    }

    void TearDown() override {
        delete renderer;
    }
};

TEST_F(RendererTest, InitializationTest) {
    ASSERT_NO_THROW(renderer->initialize());
}

TEST_F(RendererTest, RenderFrameTest) {
    renderer->initialize();
    ASSERT_NO_THROW(renderer->renderFrame());
}
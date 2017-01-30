/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include <vector>

#include <kulloclient/util/assert.h>
#include <kulloclient/util/filterchain.h>
#include <kulloclient/util/misc.h>

#include "tests/kullotest.h"

using namespace Kullo;
using namespace testing;

namespace {

class MockSink : public Util::Sink
{
public:
    MOCK_METHOD2(write, void(const unsigned char *, std::size_t));
    MOCK_METHOD0(close, void());
};

}

class MockFilter : public Util::Filter
{
public:
    MOCK_METHOD3(write, void(Util::Sink &, const unsigned char *, std::size_t));
    MOCK_METHOD1(close, void(Util::Sink &));
};

class ThrowingFilter : public Util::Filter
{
public:
    void write(
            Util::Sink &sink,
            const unsigned char *buffer,
            std::size_t size) override
    {
        K_UNUSED(sink);
        K_UNUSED(buffer);
        K_UNUSED(size);
        throw std::exception();
    }

    void close(Util::Sink &sink) override
    {
        K_UNUSED(sink);
        throw std::exception();
    }
};

class FilterChain : public KulloTest
{

};

K_TEST_F(FilterChain, ctorFailsOnNullptr)
{
    EXPECT_THROW(Util::FilterChain{ nullptr }, Util::AssertionFailed);
}

K_TEST_F(FilterChain, canConstructWithSink)
{
    Util::FilterChain{ make_unique<Util::NullSink>() };
}

K_TEST_F(FilterChain, dtorClosesSink)
{
    auto sink = make_unique<MockSink>();
    EXPECT_CALL(*sink, close());

    Util::FilterChain{ std::move(sink) };
}

K_TEST_F(FilterChain, closeClosesSink)
{
    auto sink = make_unique<StrictMock<MockSink>>();
    auto sinkPtr = sink.get();
    EXPECT_CALL(*sink, close());

    Util::FilterChain chain(std::move(sink));
    chain.close();
    // force verification before dtor is called, which would also close the sink
    Mock::VerifyAndClearExpectations(sinkPtr);
}

K_TEST_F(FilterChain, closeMustntBeCalledTwice)
{
    Util::FilterChain chain(make_unique<Util::NullSink>());
    chain.close();

    EXPECT_THROW(chain.close(), Util::AssertionFailed);
}

namespace {
void createAndDestructChainWithThrowingFilter()
{
    Util::FilterChain chain(make_unique<Util::NullSink>());
    chain.pushFilter(make_unique<ThrowingFilter>());
}
}

K_TEST_F(FilterChain, dtorMustntThrowWhenCloseThrows)
{
    EXPECT_NO_THROW(createAndDestructChainWithThrowingFilter());
}

K_TEST_F(FilterChain, writeMustntBeCalledAfterClose)
{
    Util::FilterChain chain(make_unique<Util::NullSink>());
    chain.close();

    unsigned char input = 'x';
    EXPECT_THROW(chain.write(&input, 1), Util::AssertionFailed);
}

K_TEST_F(FilterChain, writeCanBeCalledWithNullptrIffSizeIsZero)
{
    Util::FilterChain chain(make_unique<Util::NullSink>());
    EXPECT_NO_THROW(chain.write(nullptr, 0));
    EXPECT_THROW(chain.write(nullptr, 1), Util::AssertionFailed);
}

K_TEST_F(FilterChain, writeWritesToSink)
{
    std::vector<unsigned char> input = {23, 42, 5};

    auto sink = make_unique<NiceMock<MockSink>>();
    EXPECT_CALL(*sink, write(input.data(), input.size()));
    Util::FilterChain chain(std::move(sink));

    chain.write(input);
}

K_TEST_F(FilterChain, pushFilterMustntBeCalledWithNullptr)
{
    Util::FilterChain chain(make_unique<Util::NullSink>());

    EXPECT_THROW(chain.pushFilter(nullptr), Util::AssertionFailed);
}

K_TEST_F(FilterChain, pushFilterMustntBeCalledAfterClose)
{
    Util::FilterChain chain(make_unique<Util::NullSink>());

    chain.close();
    EXPECT_THROW(chain.pushFilter(make_unique<Util::NullFilter>()),
                 Util::AssertionFailed);
}

K_TEST_F(FilterChain, pushFilterMustntBeCalledAfterWrite)
{
    Util::FilterChain chain(make_unique<Util::NullSink>());

    unsigned char input = 'x';
    chain.write(&input, 1);
    EXPECT_THROW(chain.pushFilter(make_unique<Util::NullFilter>()),
                 Util::AssertionFailed);
}

K_TEST_F(FilterChain, filtersAreWrittenToOnWriteInCorrectOrder)
{
    unsigned char input = 'x';
    auto sink = make_unique<NiceMock<MockSink>>();
    auto filter1 = make_unique<NiceMock<MockFilter>>();
    auto filter2 = make_unique<NiceMock<MockFilter>>();

    auto write = [](Util::Sink &sink, const unsigned char *buffer, std::size_t size)
    {
        sink.write(buffer, size);
    };

    {
        InSequence seq;
        EXPECT_CALL(*filter2, write(_, &input, 1)).WillOnce(Invoke(write));
        EXPECT_CALL(*filter1, write(_, &input, 1)).WillOnce(Invoke(write));
        EXPECT_CALL(*sink, write(&input, 1));
    }

    Util::FilterChain chain(std::move(sink));
    chain.pushFilter(std::move(filter1));
    chain.pushFilter(std::move(filter2));

    chain.write(&input, 1);
}

K_TEST_F(FilterChain, filtersAreClosedOnCloseInCorrectOrder)
{
    auto sink = make_unique<NiceMock<MockSink>>();
    auto filter1 = make_unique<NiceMock<MockFilter>>();
    auto filter2 = make_unique<NiceMock<MockFilter>>();

    {
        InSequence seq;
        EXPECT_CALL(*filter2, close(_));
        EXPECT_CALL(*filter1, close(_));
        EXPECT_CALL(*sink, close());
    }

    Util::FilterChain chain(std::move(sink));
    chain.pushFilter(std::move(filter1));
    chain.pushFilter(std::move(filter2));

    chain.close();
}

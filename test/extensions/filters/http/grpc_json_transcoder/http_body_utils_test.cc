#include "common/buffer/buffer_impl.h"
#include "common/buffer/zero_copy_input_stream_impl.h"

#include "extensions/filters/http/grpc_json_transcoder/http_body_utils.h"

#include "test/proto/bookstore.pb.h"

#include "google/api/httpbody.pb.h"
#include "gtest/gtest.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace GrpcJsonTranscoder {
namespace {

class HttpBodyUtilsTest : public testing::Test {
public:
  HttpBodyUtilsTest() = default;

  void setBodyFieldPath(const std::vector<int>& body_field_path) {
    for (int field_number : body_field_path) {
      Protobuf::Field field;
      field.set_number(field_number);
      raw_body_field_path_.emplace_back(std::move(field));
    }
    for (auto& field : raw_body_field_path_) {
      body_field_path_.push_back(&field);
    }
  }

  template <typename Message>
  void basicTest(const std::string& content, const std::string& content_type,
                 const std::vector<int>& body_field_path,
                 std::function<google::api::HttpBody(Message message)> get_http_body) {
    setBodyFieldPath(body_field_path);

    // Parse using concrete message type.
    {
      Buffer::InstancePtr message_buffer = std::make_unique<Buffer::OwnedImpl>();
      HttpBodyUtils::appendHttpBodyEnvelope(*message_buffer, body_field_path_, content_type,
                                            content.length());
      message_buffer->add(content);

      Buffer::ZeroCopyInputStreamImpl stream(std::move(message_buffer));

      Message message;
      message.ParseFromZeroCopyStream(&stream);

      google::api::HttpBody http_body = get_http_body(std::move(message));
      EXPECT_EQ(http_body.content_type(), content_type);
      EXPECT_EQ(http_body.data(), content);
    }

    // Parse message dynamically by field path.
    {
      Buffer::InstancePtr message_buffer = std::make_unique<Buffer::OwnedImpl>();
      HttpBodyUtils::appendHttpBodyEnvelope(*message_buffer, body_field_path_, content_type,
                                            content.length());
      message_buffer->add(content);

      google::api::HttpBody http_body;
      Buffer::ZeroCopyInputStreamImpl stream(std::move(message_buffer));
      EXPECT_TRUE(HttpBodyUtils::parseMessageByFieldPath(&stream, body_field_path_, &http_body));
      EXPECT_EQ(http_body.content_type(), content_type);
      EXPECT_EQ(http_body.data(), content);
    }
  }

  std::vector<Protobuf::Field> raw_body_field_path_;
  std::vector<const Protobuf::Field*> body_field_path_;
};

TEST_F(HttpBodyUtilsTest, EmptyFieldsList) {
  basicTest<google::api::HttpBody>("abcd", "text/plain", {},
                                   [](google::api::HttpBody http_body) { return http_body; });
}

TEST_F(HttpBodyUtilsTest, LargeMessage) {
  // Check some content with more than single byte in varint encoding of the size.
  std::string content;
  content.assign(20000, 'a');
  basicTest<google::api::HttpBody>(content, "text/binary", {},
                                   [](google::api::HttpBody http_body) { return http_body; });
}

TEST_F(HttpBodyUtilsTest, LargeContentType) {
  // Check some content type with more than single byte in varint encoding of the size.
  std::string content_type;
  content_type.assign(20000, 'a');
  basicTest<google::api::HttpBody>("abcd", content_type, {},
                                   [](google::api::HttpBody http_body) { return http_body; });
}

TEST_F(HttpBodyUtilsTest, NestedFieldsList) {
  basicTest<bookstore::DeepNestedBody>(
      "abcd", "text/nested", {1, 1000000, 100000000, 500000000},
      [](bookstore::DeepNestedBody message) { return message.nested().nested().nested().body(); });
}

TEST_F(HttpBodyUtilsTest, SkipUnknownFields) {
  bookstore::DeepNestedBody message;
  auto* body = message.mutable_nested()->mutable_nested()->mutable_nested()->mutable_body();
  body->set_content_type("text/nested");
  body->set_data("abcd");
  message.mutable_extra()->set_field("test");
  message.mutable_nested()->mutable_extra()->set_field(123);

  Buffer::InstancePtr message_buffer = std::make_unique<Buffer::OwnedImpl>();
  message_buffer->add(message.SerializeAsString());
  setBodyFieldPath({1, 1000000, 100000000, 500000000});

  google::api::HttpBody http_body;
  Buffer::ZeroCopyInputStreamImpl stream(std::move(message_buffer));
  EXPECT_TRUE(HttpBodyUtils::parseMessageByFieldPath(&stream, body_field_path_, &http_body));
  EXPECT_EQ(http_body.content_type(), "text/nested");
  EXPECT_EQ(http_body.data(), "abcd");
}

} // namespace
} // namespace GrpcJsonTranscoder
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
